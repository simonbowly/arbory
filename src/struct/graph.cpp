
#include <fstream>

#include <gsl/gsl_assert>

#include "../../include/arbory/struct/graph.hpp"

using namespace std;


UndirectedGraph UndirectedGraph::read_dimacs(string file_name) {
    unsigned vertices = 0, edges = 0;
    vector<pair<unsigned, unsigned>> edge_list;
    string line;
    ifstream col_file(file_name);
    if (col_file.is_open()) {
        while ( getline(col_file, line) ) {
            if (line.substr(0, 1).compare("p") == 0) {
                Expects(vertices == 0);
                string info = line.substr(7, line.size());
                auto found = info.find(" ");
                vertices = stoi(info.substr(0, found));
                edges = stoi(info.substr(found + 1, info.size()));
                edge_list.reserve(edges);
            } else if (line.substr(0, 1).compare("e") == 0) {
                Expects(vertices > 0);
                string info = line.substr(2, line.size());
                auto found = info.find(" ");
                unsigned a = stoi(info.substr(0, found));
                unsigned b = stoi(info.substr(found + 1, info.size()));
                edge_list.emplace_back(a - 1, b - 1);
            }
        }
    } else {
        throw "File not open.";
    }
    if (edge_list.size() != edges) {
        throw domain_error("Incorrect number of edges.");
    }
    return UndirectedGraph(vertices, edge_list);
}
