
#ifndef SRC_ALGORITHMS_BACKTRACKING_HPP_
#define SRC_ALGORITHMS_BACKTRACKING_HPP_

#include <chrono>
#include <iostream>
#include <optional>
#include <utility>
#include <type_traits>
#include <variant>
#include <vector>
#include <gsl/gsl_assert>

#include "sense.hpp"


// Use where the distinction between main and alternate branch is
// static and different backtrack methods are provided.
template<typename Rule, typename Result, typename ResultAlternate>
class StaticBranching {
    static_assert(!std::is_same_v<Result, ResultAlternate>);
public:
    class StackNode {
        Rule rule;
        std::variant<Result, ResultAlternate> result;
    public:
        StackNode(std::pair<Rule, Result> r) :
            rule(std::move(r.first)), result(std::move(r.second)) {}
        bool alternate_evaluated() const {
            return std::holds_alternative<ResultAlternate>(result);
        }
        // Doesn't do pre-emptive pruning.
        template<typename State>
        bool unwind_step(State* state) {
            return std::visit([this, state](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ResultAlternate>) {
                    // Both branches have been pursued, discard the node.
                    state->backtrack(rule, arg);
                    return true;
                } else {
                    // Branch 1 has been pursued, branch 2 next.
                    state->backtrack(rule, arg);
                    result = state->branch_alternate(rule);
                    return false;
                }
            }, result);
        }
    };
};


// Use where the distinction between main and alternate branch is
// decided dynamically and the backtracking method has no type
// overloading.
template<typename Rule, typename Result>
class DynamicBranching {
public:
    class StackNode {
        Rule rule;
        Result result;
        bool _alternate_evaluated;
    public:
        StackNode(std::pair<Rule, Result> r) :
            rule(std::move(r.first)), result(std::move(r.second)),
            _alternate_evaluated(false) {}
        bool alternate_evaluated() const { return _alternate_evaluated; }
        template<typename State>
        bool unwind_step(State* state) {
            state->backtrack(rule, result);
            if (_alternate_evaluated) {
                return true;
            } else {
                result = state->branch_alternate(rule);
                _alternate_evaluated = true;
                return false;
            }
        }
    };
};


// Should sense be a property of the state class?
template <typename State, Sense sense>
class Solver {
    using opt = SenseOps<sense>;
    using Sol = typename std::invoke_result<decltype(&State::get_solution), State>::type;
    using Obj = typename std::invoke_result<decltype(&Sol::get_objective_value), Sol>::type;
    // Determine whether static or dynamic branching should be used.
    using br_rule = typename std::invoke_result<decltype(&State::branch), State>::type::first_type;
    using br_res = typename std::invoke_result<decltype(&State::branch), State>::type::second_type;
    using br_res_alt = typename std::invoke_result<decltype(&State::branch_alternate), State, br_rule>::type;
    using StackElement = typename std::conditional<
        std::is_same_v<br_res, br_res_alt>,
        DynamicBranching<br_rule, br_res>,
        StaticBranching<br_rule, br_res, br_res_alt>>::type::StackNode;

    State* state;
    std::vector<StackElement> stack;
    std::vector<Sol> solutions;
    Obj primal_bound;

public:
    explicit Solver(State* s) : state(s), stack(), solutions(),
                        primal_bound(initial_primal_bound<Obj, sense>()) {}

    const std::vector<Sol>& get_solutions() const { return solutions; }

    // Do a single backtracking step and return whether the head node
    // should be popped & unwinding should continue.
    // This method should be implemented by the StackElement class
    // (returning whether to break) so that static/dynamic branching can
    // be substituted.
    static bool unwind_step(StackElement& head, State* state, const Obj& primal_bound) {
        if (head.alternate_evaluated()) {
            // Both branches have been pursued, discard the node.
            state->backtrack(head.get_rule(), head.get_branch2_result());
            return true;
        } else {
            // Branch 1 has been pursued, branch 2 next.
            state->backtrack(head.get_rule(), head.get_branch1_result());
            if (opt::can_be_pruned(*state, primal_bound)) {
                // Pre-emptively prune the other branch.
                return true;
            } else {
                // Unwound back to an incompletely-explored node.
                // Branch the other way and stop unwinding to pursue the new branch.
                head.set_branch_result(state->branch_alternate(head.get_rule()));
                return false;
            }
        }
    }

    void unwind_and_branch_alternate() {
        while ((stack.size() > 0) && stack.back().unwind_step(state)) {
        // while ((stack.size() > 0) && unwind_step(stack.back(), state, primal_bound)) {
            stack.pop_back();
        }
        Ensures((stack.size() == 0) || stack.back().alternate_evaluated());
    }

    void print_stack() const {
        for (const auto & node : stack) {
            if (node.alternate_evaluated()) {
                std::cout << "L";
            } else {
                std::cout << "R";
            }
        }
        std::cout << std::endl;
    }

    std::pair<unsigned, unsigned> depths() const {
        unsigned ldepth = 0, rdepth = 0;
        auto it = stack.begin();
        while (it != stack.end() && it->alternate_evaluated()) {
            ++it;
            ++ldepth;
        }
        while (it != stack.end() && !it->alternate_evaluated()) {
            ++it;
            ++rdepth;
        }
        return std::make_pair(ldepth, rdepth);
    }

    void log_progress(
        std::chrono::time_point<std::chrono::high_resolution_clock> start,
        unsigned nodes, Obj primal_bound, bool incumbent)
    {
        double runtime = std::chrono::duration<double, std::milli>
            (std::chrono::high_resolution_clock::now() - start)
            .count() / 1000;
        if (incumbent) { std::cout << "*"; } else { std::cout << " "; }
        auto [ldepth, rdepth] = depths();
        std::cout << "  TIME: " << runtime << "s"
                << "  NODES: " << nodes
                << "  PRIMAL: " << primal_bound
                << "  LDEPTH: " << ldepth
                << "  RDEPTH: " << rdepth
                << std::endl;
    }

    void solve(unsigned int log_frequency) {
        unsigned int nodes = 0;
        auto start = std::chrono::high_resolution_clock::now();
        do {
            if (!state->is_feasible() || opt::can_be_pruned(*state, primal_bound)) {
                // No solutions due to infeasibility, or not worth exploring
                // due to dual bounds. Unwind.
                unwind_and_branch_alternate();
            } else if (state->is_leaf()) {
                // Feasible complete solution. Add it to the pool and update
                // the primal bound.
                solutions.emplace_back(state->get_solution());
                Expects(opt::is_improvement(
                    solutions.back().get_objective_value(),
                    primal_bound));
                primal_bound = solutions.back().get_objective_value();
                log_progress(start, nodes, primal_bound, true);
                unwind_and_branch_alternate();
            } else {
                // Subproblem is incomplete, still improving and still feasible.
                // Evaluate branch rule and add node to the stack.
                stack.emplace_back(state->branch());
            }
            nodes++;
            if ((nodes % log_frequency) == 0) {
                log_progress(start, nodes, primal_bound, false);
            }
        } while (stack.size() > 0);
        // Final logging statistics after completion.
        double runtime = std::chrono::duration<double, std::milli>
            (std::chrono::high_resolution_clock::now() - start)
            .count() / 1000;
        std::cout << "====== COMPLETE ======" << std::endl;
        std::cout << "Status:      Optimal" << std::endl;
        std::cout << "Nodes:       " << nodes << std::endl;
        std::cout << "Solutions:   " << solutions.size() << std::endl;
        std::cout << "Time:        " << runtime << " seconds" << std::endl;
        std::cout << "Objective:   " << primal_bound << std::endl;
        std::cout << "Rate:        " << nodes / runtime << " nodes/second" << std::endl;
        std::cout << "======================" << std::endl;
    }
};

#endif  // SRC_ALGORITHMS_BACKTRACKING_HPP_
