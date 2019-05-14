// Copyright [2019] <Simon Bowly>

#ifndef SRC_GRAPH_HPP_
#define SRC_GRAPH_HPP_

#include <assert.h>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>


class NeighbourSet {
    std::vector<unsigned> data;
 public:
    NeighbourSet() {}
    explicit NeighbourSet(std::vector<unsigned>&& d) : data(std::move(d)) {}
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
    auto rbegin() const { return data.rbegin(); }
    auto rend() const { return data.rend(); }
    auto operator[](unsigned u) const { return data[u]; }
    auto size() const { return data.size(); }
    bool find(unsigned u) const {
        return std::binary_search(data.begin(), data.end(), u);
        // return std::find(data.begin(), data.end(), u) != data.end();
    }
};


std::vector<NeighbourSet> createGraph(unsigned vertices,
        const std::vector<std::pair<unsigned, unsigned>>& edges);


std::vector<NeighbourSet> trimmedGraph(
    const std::vector<NeighbourSet>& g,
    const std::vector<unsigned>& clique);


#endif  // SRC_GRAPH_HPP_
