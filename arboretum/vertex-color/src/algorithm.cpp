
#include "arbory/stack_backtracking.hpp"

#include "../include/algorithm.hpp"
#include "../include/state.hpp"

using namespace std;


void solveStackBacktrackingVector(
    unsigned vertices,
    const std::vector<std::pair<unsigned, unsigned>>& edges,
    unsigned nodeLogFrequency)
{
    Node root = Node::createRoot(vertices, edges);
    stackBacktracking<Node, BranchChoice, MergePlan, nullptr_t>(
        &root, nodeLogFrequency);
}
