#ifndef SRC_MAXIMUMCLIQUE_GRAPH_HPP_
#define SRC_MAXIMUMCLIQUE_GRAPH_HPP_

#include <algorithm>
#include <utility>
#include <vector>

// Graph representation which carries out constraint checking (in this case,
// whether two vertices are adjacent) for the State class.
class Graph {
    std::vector<std::vector<unsigned>> _adjacent;
public:
    Graph(unsigned n, std::vector<std::pair<unsigned, unsigned>> edges) : _adjacent(n) {
        for (auto [i, j] : edges) {
            _adjacent[i].push_back(j);
            _adjacent[j].push_back(i);
        }
    }

    bool adjacent(const unsigned u, const unsigned v) const {
        const auto& ref = _adjacent[u];
        return std::find(std::begin(ref), std::end(ref), v) != std::end(ref);
    }
};

#endif  // SRC_MAXIMUMCLIQUE_GRAPH_HPP_
