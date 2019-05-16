
#include <iostream>
#include <string>

#include <cxxopts.hpp>

#include "../include/algorithm.hpp"

using namespace std;


int main(int argc, char **argv) {
    cxxopts::Options options("Arbory MaxClique", "Exact Maximum Clique Solver");
    options.add_options()
        ("f,file", "Input File", cxxopts::value<string>())
        ("l,log", "Node Log Frequency", cxxopts::value<unsigned>())
        ("m,mode", "Tree Search Mode", cxxopts::value<string>())
        ;
    options.parse_positional({"file"});
    auto result = options.parse(argc, argv);
    const auto graph = UndirectedGraph::read_dimacs(result["file"].as<string>());
    cout << "Vertices: " << graph.vertices() << endl;
    cout << "Edges: " << graph.edges() << endl;
    if (result["mode"].as<string>() == "recursion") {
        auto solution = solve_recursive(graph);
        cout << "Solution:  ";
        solution->print();
        cout << "  (Obj = " << solution->get_objective_value() << ")";
        cout << endl;
    } else if (result["mode"].as<string>() == "backtrack") {
        auto solutions = solve_backtrack(graph, result["log"].as<unsigned>());
        cout << "Solution Pool: " << endl;
        for (const auto& solution : solutions) {
            cout << "  (Obj = " << solution.get_objective_value() << ")  ";
            solution.print();
            cout << endl;
        }
    } else {
        throw domain_error("Bad mode choice.");
    }
}
