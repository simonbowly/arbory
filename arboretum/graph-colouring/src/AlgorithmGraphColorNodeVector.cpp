
#include "../../../include/stackBacktracking.hpp"
#include "GraphColorNodeVector.hpp"

#include "AlgorithmGraphColorNodeVector.hpp"

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
