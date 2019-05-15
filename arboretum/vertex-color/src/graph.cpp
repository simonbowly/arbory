// Copyright [2019] <Simon Bowly>

#include <unordered_set>

#include "../include/graph.hpp"

using std::vector, std::pair, std::unordered_set, std::domain_error;

vector<NeighbourSet> createGraph(unsigned vertices,
        const vector<pair<unsigned, unsigned>>& edges) {
    vector<NeighbourSet> graph;
    graph.reserve(vertices);
    vector<vector<unsigned>> tmp(vertices);
    for (const auto& [i, j] : edges) {
        tmp[i].push_back(j);
        tmp[j].push_back(i);
    }
    for (auto& nset : tmp) {
        sort(nset.begin(), nset.end());
        for (auto it = nset.begin(), next = nset.begin() + 1;
                next != nset.end(); ++it, ++next) {
            // Check element uniqueness once sorted.
            if (*it >= *next) {
                throw domain_error("Duplicate edges in undirected graph.");
            }
        }
        // NeighbourSet takes ownership (non-copy).
        graph.emplace_back(move(nset));
        assert(nset.size() == 0);
    }
    return graph;
}

vector<NeighbourSet> trimmedGraph(
        const vector<NeighbourSet>& g, const vector<unsigned>& c) {
    unordered_set<unsigned> clique(c.begin(), c.end());
    vector<NeighbourSet> graph;
    unsigned u = 0;
    for (const auto& row : g) {
        vector<unsigned> tmp;
        if (clique.find(u) == clique.end()) {
            for (const auto& v : row) {
                if (clique.find(v) == clique.end()) {
                    tmp.push_back(v);
                }
            }
        }
        graph.emplace_back(move(tmp));
        u++;
    }
    return graph;
}
