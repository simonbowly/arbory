
#include "arbory/backtracking.hpp"

#include "../include/algorithm.hpp"
#include "../include/state.hpp"

using namespace std;


void solve_backtrack(const UndirectedGraph& graph, unsigned log_frequency) {
    using VertexColorSolver = Solver<
        Node, VertexColorSol, unsigned int, Sense::Minimize,
        Merge, Difference, MergeResult, DifferenceResult>;
    Node root(graph);
    root.initialise_greedy();
    cout << "Clique: " << root.get_lower_bound() << endl;
    VertexColorSolver solver(&root);
    solver.solve(log_frequency);
}
