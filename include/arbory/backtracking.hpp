
#ifndef SRC_ALGORITHMS_BACKTRACKING_HPP_
#define SRC_ALGORITHMS_BACKTRACKING_HPP_

#include <chrono>
#include <iostream>
#include <optional>
#include <vector>
#include <gsl/gsl_assert>

#include "sense.hpp"


template <typename Br1, typename Br2, typename Res1, typename Res2>
class StackNode {
    Br1 branch1;
    Br2 branch2;
    std::optional<Res1> result1;
    std::optional<Res2> result2;

 public:
    StackNode(Br1 b1, Br2 b2) : branch1(b1), branch2(b2),
                        result1(std::nullopt), result2(std::nullopt) {}
    bool branch1_evaluated() const { return (bool) result1; }
    bool branch2_evaluated() const { return (bool) result2; }
    const Br1 & get_branch1() const { return branch1; }
    const Br2 & get_branch2() const { return branch2; }
    void set_branch_result(Res1 r1) { result1 = r1; }
    void set_branch_result(Res2 r2) { result2 = r2; }
    const Res1 & get_branch1_result() const { return *result1; }
    const Res2 & get_branch2_result() const { return *result2; }
};


template <typename State, typename Sol, typename Obj, Sense sense,
          typename Br1, typename Br2, typename Res1, typename Res2>
class Solver {
    using StackElement = StackNode<Br1, Br2, Res1, Res2>;
    using opt = SenseOps<sense>;
    State* state;
    std::vector<StackElement> stack;
    std::vector<Sol> solutions;
    Obj primal_bound;

 public:
    explicit Solver(State* s) : state(s), stack(), solutions(),
                        primal_bound(initial_primal_bound<Obj, sense>()) {}

    const std::vector<Sol>& get_solutions() const { return solutions; }

    void unwind() {
        while (stack.size() > 0) {
            const auto& head = stack.back();
            Expects(head.branch1_evaluated());
            if (head.branch2_evaluated()) {
                // Both branches have been pursued, discard the node.
                state->backtrack(head.get_branch2_result());
                stack.pop_back();
            } else {
                // Branch 1 has been pursued, branch 2 next.
                state->backtrack(head.get_branch1_result());
                if (opt::can_be_pruned(*state, primal_bound)) {
                    // Pre-emptively prune the other branch.
                    stack.pop_back();
                } else {
                    break;
                }
            }
        }
    }

    void solve(unsigned int logFrequency) {
        unsigned int nodes = 0;
        auto start = std::chrono::high_resolution_clock::now();
        while (true) {
            bool incumbent = false;
            nodes++;
            if (!state->is_feasible()) {
                // No solutions due to infeasibility, unwind.
                unwind();
            } else if (opt::can_be_pruned(*state, primal_bound)) {
                // Not worth exploring due to dual bound, unwind.
                unwind();
            } else if (state->is_leaf()) {
                // Feasible complete solution. Add it to the pool and update
                // the primal bound.
                solutions.emplace_back(state->get_solution());
                Expects(opt::is_improvement(
                    solutions.back().get_objective_value(),
                    primal_bound));
                primal_bound = solutions.back().get_objective_value();
                incumbent = true;
                unwind();
            } else {
                // Subproblem is incomplete, still improving and still feasible.
                // Evaluate branch rule and add node to the stack.
                auto [first, second] = state->branch_decision();
                stack.emplace_back(first, second);
            }
            // Follow the first unexplored branch on the head node of the stack.
            if (stack.size() == 0) {
                break;
            }
            auto& head = stack.back();
            if (!head.branch1_evaluated()) {
                head.set_branch_result(state->branch(head.get_branch1()));
            } else {
                Expects(!head.branch2_evaluated());
                head.set_branch_result(state->branch(head.get_branch2()));
            }
            if ((nodes % logFrequency) == 0 || incumbent) {
                // Log current time, nodes processed, best solution.
                double runtime = std::chrono::duration<double, std::milli>
                    (std::chrono::high_resolution_clock::now() - start)
                    .count() / 1000;
                if (incumbent) { std::cout << "*"; } else { std::cout << " "; }
                std::cout << "  TIME: " << runtime << "s"
                        << "  NODES: " << nodes
                        << "  PRIMAL: " << primal_bound
                        << std::endl;
            }
        }
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
