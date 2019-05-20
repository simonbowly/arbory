
#include "arbory/backtracking.hpp"

#include "../include/algorithm.hpp"
#include "../include/state.hpp"

using namespace std;


void solve_backtrack_vc(const UndirectedGraph& graph, unsigned log_frequency) {
    Node root(graph);
    root.initialise();
    cout << "Clique: " << root.get_lower_bound() << endl;
    Solver<Node, Sense::Minimize> solver(&root);
    solver.solve(log_frequency);
}
