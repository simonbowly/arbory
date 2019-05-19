
#ifndef SRC_ALGORITHMS_BACKTRACKING_HPP_
#define SRC_ALGORITHMS_BACKTRACKING_HPP_

#include <chrono>
#include <iostream>
#include <optional>
#include <utility>
#include <vector>
#include <gsl/gsl_assert>

#include "sense.hpp"


template <typename Rule, typename Res1, typename Res2>
class StackNode {
    std::pair<Rule, Res1> rule;
    std::optional<Res2> result2;
 public:
    StackNode(std::pair<Rule, Res1> r) : rule(r), result2(std::nullopt) {}
    const Rule & get_rule() const { return rule.first; }
    const Res1 & get_branch1_result() const { return rule.second; }
    bool branch2_evaluated() const { return (bool) result2; }
    void set_branch_result(Res2 r2) { result2 = r2; }
    const Res2 & get_branch2_result() const { return *result2; }
};


template <typename State, typename Sol, typename Obj, Sense sense,
          typename Rule, typename Res1, typename Res2>
class Solver {
    using StackElement = StackNode<Rule, Res1, Res2>;
    using opt = SenseOps<sense>;
    State* state;
    std::vector<StackElement> stack;
    std::vector<Sol> solutions;
    Obj primal_bound;

 public:
    explicit Solver(State* s) : state(s), stack(), solutions(),
                        primal_bound(initial_primal_bound<Obj, sense>()) {}

    const std::vector<Sol>& get_solutions() const { return solutions; }

    void unwind_and_branch_alternate() {
        while (stack.size() > 0) {
            if (stack.back().branch2_evaluated()) {
                // Both branches have been pursued, discard the node.
                state->backtrack(stack.back().get_rule(), stack.back().get_branch2_result());
                stack.pop_back();
            } else {
                // Branch 1 has been pursued, branch 2 next.
                state->backtrack(stack.back().get_rule(), stack.back().get_branch1_result());
                if (opt::can_be_pruned(*state, primal_bound)) {
                    // Pre-emptively prune the other branch.
                    stack.pop_back();
                } else {
                    // Unwound back to an incompletely-explored node. Branch the other way.
                    stack.back().set_branch_result(state->branch_alternate(stack.back().get_rule()));
                    break;
                }
            }
        }
        Ensures((stack.size() == 0) || stack.back().branch2_evaluated());
    }

    void print_stack() const {
        for (const auto & node : stack) {
            if (node.branch2_evaluated()) {
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
        while (it != stack.end() && it->branch2_evaluated()) {
            ++it;
            ++ldepth;
        }
        while (it != stack.end() && !it->branch2_evaluated()) {
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
