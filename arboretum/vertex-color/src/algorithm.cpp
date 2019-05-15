
#include "arbory/backtracking.hpp"

#include "../include/algorithm.hpp"
#include "../include/state.hpp"

using namespace std;


void solveStackBacktrackingVector(
    unsigned vertices,
    const std::vector<std::pair<unsigned, unsigned>>& edges,
    unsigned nodeLogFrequency)
{
    using VertexColorSolver = Solver<
        Node, VertexColorSol, unsigned int, Sense::Minimize,
        Merge, Difference, MergeResult, DifferenceResult>;
    Node root = Node::createRoot(vertices, edges);
    VertexColorSolver solver(&root);
    solver.solve(nodeLogFrequency);

    // stackBacktracking<Node, BranchChoice, MergePlan, nullptr_t>(
    //     &root, nodeLogFrequency);
}
