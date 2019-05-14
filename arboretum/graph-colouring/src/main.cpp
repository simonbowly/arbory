
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include <cxxopts.hpp>

#include "../../../include/stackBacktracking.hpp"

#include "AlgorithmGraphColorNodeVector.hpp"

using namespace std;


int main(int argc, char **argv) {

    cxxopts::Options options("Graph Colourer", "Complete GC solver");
    options.add_options()
        ("v,vertices", "Number of Vertices", cxxopts::value<unsigned>())
        ("p,probability", "Edge Probability", cxxopts::value<double>())
        ("f,frequency", "Node Log Frequency", cxxopts::value<unsigned>())
        ;
    auto result = options.parse(argc, argv);

    default_random_engine rng;
    uniform_real_distribution<double> dist(0, 1);

    // Graph problem construction.
    unsigned n = result["vertices"].as<unsigned>();
    double d = result["probability"].as<double>();
    vector<pair<unsigned, unsigned>> edges;
    {
        for (unsigned i = 0; i < n; i++) {
            for (unsigned j = i + 1; j < n; j++) {
                if (dist(rng) < d) {
                    edges.emplace_back(i, j);
                }
            }
        }
    }

    solveStackBacktrackingVector(n, edges, result["frequency"].as<unsigned>());

    return 0;

}
