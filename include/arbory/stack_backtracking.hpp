
#ifndef SRC_ALGORITHMS_STACKBACKTRACKING_HPP_
#define SRC_ALGORITHMS_STACKBACKTRACKING_HPP_

#include <chrono>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

// TODO embed this in a class so backtracking can be paused and state kept.
// Write stack unwinding/splitting methods to allow part of the stack to
// be copied/extracted for another thread to work on in such a way that
// the new stacks (possibly with altered root nodes) won't duplicate any
// work. This would require copying the RULE/RTSRUCT/LSTRUCT objects in the
// stack, and creating new nodes by copy.

template <typename NODE, typename RULE, typename RSTRUCT, typename LSTRUCT>
class StackBacktracking {
    NODE node;
    std::vector<std::pair<RULE, std::variant<RSTRUCT, LSTRUCT>>> stack;
    std::optional<unsigned> bestSolution = std::nullopt;

 public:
    explicit StackBacktracking(NODE* root) : node(std::move(*root)) {}
    void run(unsigned logFrequency) {
        unsigned leaves = 0, pruned = 0, nodes = 0;
        auto start = std::chrono::high_resolution_clock::now();

        while (true) {
            bool incumbent = false;
            nodes += 1;
            if (node.isComplete() || (bestSolution.has_value() && (node.getObjective() >= *bestSolution))) {
                // Leaf node (by pruning or completion).
                if (node.isComplete()) {
                    if (bestSolution.has_value()) {
                        if (node.getObjective() < *bestSolution) {
                            bestSolution = node.getObjective();
                            leaves += 1;
                            incumbent = true;
                        } else {
                            pruned += 1;
                        }
                    } else {
                        bestSolution = node.getObjective();
                        leaves += 1;
                        incumbent = true;
                    }
                } else {
                    pruned += 1;
                }
                // Backtrack up all left branches.
                while ((!stack.empty()) && std::holds_alternative<LSTRUCT>(stack.back().second)) {
                    node.unwind(stack.back().first, std::get<LSTRUCT>(stack.back().second));
                    stack.pop_back();
                }
                // Unwound right up the left branch to the root -> done.
                if (stack.empty()) {
                    break;
                }
                // Unwind the right branch we've already pursued and dive left.
                node.unwind(stack.back().first, std::get<RSTRUCT>(stack.back().second));
                if (bestSolution.has_value() && (node.getObjective() >= *bestSolution)) {
                    // This check says that diving left would immediately result in
                    // a pruned node. So we cut off preemptively and don't change the
                    // node state. For bookkeeping - record node was unchecked.
                    // Effect is pretty miniscule and this is not the best place to put
                    // the check.
                    stack.pop_back();
                    nodes -= 1;
                } else {
                    std::variant<RSTRUCT, LSTRUCT> lchange = node.diveLeft(stack.back().first);
                    std::swap(stack.back().second, lchange);
                }
            } else {
                // Dive right from any incomplete state.
                RULE rule = node.getBranchingRule();
                RSTRUCT rchange = node.diveRight(rule);
                stack.emplace_back(std::move(rule), std::move(rchange));
            }

            if ((nodes % logFrequency) == 0 || incumbent) {
                // Log current time, nodes processed, best solution.
                double runtime = std::chrono::duration<double, std::milli>
                    (std::chrono::high_resolution_clock::now() - start)
                    .count() / 1000;
                if (incumbent) { std::cout << "*"; } else { std::cout << " "; }
                std::cout << "  TIME: " << runtime << "s"
                        << "  NODES: " << nodes
                        << "  PRUNED : " << pruned
                        << "  BEST: " << *bestSolution
                        << std::endl;
            }
        }
        // Final logging statistics after completion.
        double runtime = std::chrono::duration<double, std::milli>
            (std::chrono::high_resolution_clock::now() - start)
            .count() / 1000;
        std::cout << "==== OPTIMAL ====" << std::endl;
        std::cout << "Nodes:     " << nodes
                << "  (Leaves: " << leaves
                << "  Pruned: " << pruned
                << "  Branch: " << nodes - leaves - pruned
                << ")" << std::endl;
        std::cout << "Time:      " << runtime << " seconds" << std::endl;
        std::cout << "Objective: " << *bestSolution << std::endl;
        std::cout << "Rate:      " << nodes / runtime << " nodes/second" << std::endl;
        std::cout << "=================" << std::endl;
    }
};


template <typename NODE, typename RULE, typename RSTRUCT, typename LSTRUCT>
void stackBacktracking(NODE* node, unsigned logFrequency) {
    StackBacktracking<NODE, RULE, RSTRUCT, LSTRUCT> runner(node);
    runner.run(logFrequency);
}


#endif  // SRC_ALGORITHMS_STACKBACKTRACKING_HPP_
