
#include <iostream>
#include <string>

#include <cxxopts.hpp>

#include "../include/algorithm.hpp"

using namespace std;


int main(int argc, char **argv) {
    cxxopts::Options options("Arbory Vertex Coloring", "Exact Vertex Coloring Solver");
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
    solve_backtrack(graph, result["log"].as<unsigned>());
    return 0;
}
