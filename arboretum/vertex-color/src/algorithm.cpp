
#include "arbory/backtracking.hpp"

#include "../include/algorithm.hpp"
#include "../include/state.hpp"

using namespace std;


void solve_backtrack_vc(const UndirectedGraph& graph, unsigned log_frequency) {
    using VertexColorSolver = Solver<
        Node, Sense::Minimize,
        StaticBranching<Rule, MergeResult, DifferenceResult>>;
    Node root(graph);
    root.initialise();
    cout << "Clique: " << root.get_lower_bound() << endl;
    VertexColorSolver solver(&root);
    solver.solve(log_frequency);
}
