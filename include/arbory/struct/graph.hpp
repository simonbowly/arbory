#ifndef SRC_ARBORY_STRUCT_GRAPH_HPP_
#define SRC_ARBORY_STRUCT_GRAPH_HPP_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>


// Undirected graph with N vertices numbered 0..N-1.
class UndirectedGraph {
    std::vector<std::vector<unsigned>> _adjacent;
public:
    // Construct from edge list (assumes no repetition of pairs).
    UndirectedGraph(unsigned n, std::vector<std::pair<unsigned, unsigned>> edges)
        : _adjacent(n)
    {
        for (auto [i, j] : edges) {
            _adjacent[i].push_back(j);
            _adjacent[j].push_back(i);
        }
        for (unsigned i = 0; i < n; i++) {
            auto& ref = _adjacent[i];
            std::sort(std::begin(ref), std::end(ref));
        }
    }
    // Accessors
    unsigned vertices() const { return _adjacent.size(); }
    // Return whether an edge exists between i and j.
    bool adjacent(const unsigned i, const unsigned j) const {
        const auto& ref = _adjacent[i];
        return std::binary_search(std::begin(ref), std::end(ref), j);
    }
    // Read a DIMACS file (format below) and return the graph object.
    //
    //   p edges N M
    //   e i j
    //   ...
    //
    static UndirectedGraph read_dimacs(std::string file_name);
};

#endif  // SRC_ARBORY_STRUCT_GRAPH_HPP_
