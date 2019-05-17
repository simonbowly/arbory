
#include <iostream>
#include <string>

#include "../include/algorithm.hpp"

using namespace std;


void test_solve(string file_name) {
    cout << "===== Solving " << file_name << " =====" << endl;
    const auto graph = UndirectedGraph::read_dimacs(file_name);
    cout << "Vertices: " << graph.vertices() << endl;
    cout << "Edges: " << graph.edges() << endl;
    solve_backtrack_vc(graph, 10);
}


int main() {
    test_solve("../../instances/graphs/2-FullIns_3.col");
    test_solve("../../instances/graphs/miles250.col");
    return 0;
}
