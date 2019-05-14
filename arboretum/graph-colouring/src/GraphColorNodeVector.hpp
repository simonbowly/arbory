// Copyright [2019] <Simon Bowly>

#ifndef SRC_GRAPHCOLORNODEVECTOR_HPP_
#define SRC_GRAPHCOLORNODEVECTOR_HPP_

#include <algorithm>
#include <iostream>
#include <memory>
#include <limits>
#include <utility>
#include <vector>

#include "Graph.hpp"

struct MergePlan {
    std::vector<unsigned> makeNeighboursOfU;
    std::vector<unsigned> addToClique;
};

struct BranchChoice {
    unsigned u;
    unsigned v;
};

#ifndef NDEBUG
#define RUN_INVARIANT_CHECK checkInvariant();
#else
#define RUN_INVARIANT_CHECK
#endif

const unsigned non_clique = std::numeric_limits<unsigned>::max();


template<typename T>
int index_of(std::vector<T> vec, T u) {
    return std::find(vec.begin(), vec.end(), u) - vec.begin();
}


class Node {
    // state[u] == u -> u is a clique vertex
    // state[u] == v -> u is merged with u
    // state[u] == nullopt -> u outside clique
    std::vector<unsigned> state;
    std::vector<std::vector<unsigned>> neighbours;
    unsigned cliqueSize;
    unsigned mergeCount;
    std::shared_ptr<const std::vector<NeighbourSet>> graph;

    // TODO(simonbowly) check uniqueness in graph & neighbour structures.
    // Use a constexpr to introduce these calls to allow
    // Node<true /* do checks */> ??
    #ifndef NDEBUG
    void checkInvariant() const {
        unsigned expectCliqueSize = 0;
        bool expectComplete = true;
        for (unsigned u = 0; u < state.size(); u++) {
            if (state[u] == non_clique) {
                expectComplete = false;
                for (const auto& v : neighbours[u]) {
                    if (state[v] != v) {
                        throw std::domain_error(
                            "Neighbour does not point to clique vertex.");
                    }
                }
            } else if (state[u] == u) {
                expectCliqueSize += 1;
            } else {
                unsigned v = state[u];
                if (state[v] != v) {
                    throw std::domain_error(
                        "Merge vertex does not point to clique vertex.");
                }
            }
            if (neighbours[u].size() >= cliqueSize) {
                throw std::domain_error("Clique is not maximal.");
            }
        }
        if (cliqueSize != expectCliqueSize) {
            throw std::domain_error("Clique size value is incorrect.");
        }
        if (isComplete() != expectComplete) {
            throw std::domain_error("Complete flag value is incorrect.");
        }
    }
    #endif

    Node(const Node& a) = default;
    Node& operator=(const Node&) = default;

 public:
    explicit Node(unsigned n) : state(n, non_clique), neighbours(n),
        cliqueSize(0), mergeCount(0), graph(nullptr) {}
    Node(Node&& a) = default;
    Node& operator=(Node&& a) = default;

    Node clone() const {
        return *this;
    }

    bool operator==(const Node& other) const {
        return (
            (state == other.state)
            && (neighbours == other.neighbours)
            && (cliqueSize == other.cliqueSize)
            && (mergeCount == other.mergeCount));
    }

    static Node createRoot(unsigned vertices,
            const std::vector<std::pair<unsigned, unsigned>>& edges) {
        auto g = createGraph(vertices, edges);
        // Greedy clique construction.
        std::vector<unsigned> clique;
        clique.push_back(0);
        for (unsigned u = 1; u < vertices; u++) {
            bool include = true;
            for (unsigned c : clique) {
                if (!g[u].find(c)) {
                    include = false;
                    break;
                }
            }
            if (include) {
                clique.push_back(u);
            }
        }
        // Initialised with n non-clique states, and n empty neighbour lists.
        Node node(vertices);
        // Sets the current graph clique (reset are non-clique by default).
        for (const auto& u : clique) {
            node.state[u] = u;
        }
        node.cliqueSize = clique.size();
        // Construct variable graph states.
        for (const auto& u : clique) {
            for (const auto& v : g[u]) {
                if (node.state[v] == non_clique) {
                    node.neighbours[v].push_back(u);
                }
            }
        }
        // TODO(simonbowly) can replace graph with trimmed graph at this point.
        node.graph = std::make_shared<std::vector<NeighbourSet>>(
            trimmedGraph(g, clique));
        #ifndef NDEBUG
        node.checkInvariant();
        #endif
        return node;
    }

    bool isComplete() const {
        return (cliqueSize + mergeCount) == state.size();
    }

    unsigned getMaxDSATVertex() const {
        // Return the non_clique vertex with the most neighbours.
        // Kind of awkward two-structure loop. Is there a standard
        // C++ method to find a joined condition like this?
        // Alternate method might be a single structure like
        // vector<pair<unsigned, vector<unsigned>> which we can loop
        // over once.
        auto s = find(state.begin(), state.end(), non_clique);
        auto n = neighbours.begin() + (s - state.begin());
        auto best = n;
        for (; s != state.end(); ++s, ++n) {
            if ((*s == non_clique) & (n->size() > best->size())) {
                best = n;
            }
        }
        unsigned v = best - neighbours.begin();
        assert(state[v] == non_clique);
        return v;
    }

    unsigned getMergeCandidate(unsigned v) const {
        unsigned u = 0;
        const auto& nv = neighbours[v];
        for (auto it = state.begin(); it != state.end(); ++it, ++u) {
            if (*it == u) {
                if (std::find(nv.begin(), nv.end(), u) == nv.end()) {
                    break;
                }
            }
        }
        return u;
    }

    [[gnu::always_inline]]
    BranchChoice getBranchChoice() const  {
        // Note that vertex indices, instead of iterators into the data
        // structure, are used here. This is probably a performance hit
        // for a pure backtracking algorithm, but is required if we are
        // going to use this branching information in a different structure
        // used by a different thread.
        // Could provide GetBranchChoiceLocal() and GetBranchChoiceLocal()?
        // Or a struct with a .generic() method which returns a non-iterator
        // variant?
        // Is it possible to prevent pointer invalidation in a reasonable
        // way?
        BranchChoice choice;
        choice.v = getMaxDSATVertex();
        choice.u = getMergeCandidate(choice.v);
        return choice;
    }

    bool branchChoiceIsValid(const BranchChoice& choice) const {
        const auto& nv = neighbours[choice.v];
        return (
            choice.u < state.size()
            && choice.v < state.size()
            && state[choice.u] == choice.u
            && state[choice.v] == non_clique
            && std::find(nv.begin(), nv.end(), choice.u) == nv.end());
    }

    // Add an edge between u and v.
    void diveDifference(const BranchChoice& choice) {
        if (neighbours[choice.v].size() == cliqueSize - 1) {
            state[choice.v] = choice.v;
            cliqueSize += 1;
            for (auto w : (*graph)[choice.v]) {
                if (state[w] == non_clique) {
                    neighbours[w].push_back(choice.v);
                }
            }
        } else {
            neighbours[choice.v].push_back(choice.u);
        }
        RUN_INVARIANT_CHECK
    }

    // Revert a call to diveDifference with the same arguments.
    void backtrackDifference(const BranchChoice& choice) {
        if (state[choice.v] == choice.v) {
            state[choice.v] = non_clique;
            cliqueSize -= 1;
            for (auto w : (*graph)[choice.v]) {
                if (state[w] == non_clique) {
                    neighbours[w].pop_back();
                }
            }
        } else {
            neighbours[choice.v].pop_back();
        }
        RUN_INVARIANT_CHECK
    }

    MergePlan planMerge(const BranchChoice& choice) const {
        MergePlan plan;
        // Non-clique neighbours of v not already neighbours of u
        // either need neighbours updated or are clique candidates.
        std::vector<unsigned> cliqueCandidates;
        for (const auto& w : (*graph)[choice.v]) {
            if (state[w] == non_clique) {
                const auto& nw = neighbours[w];
                if (std::find(nw.begin(), nw.end(), choice.u) == nw.end()) {
                    if (nw.size() == cliqueSize - 1) {
                        cliqueCandidates.push_back(w);
                    } else {
                        plan.makeNeighboursOfU.push_back(w);
                    }
                }
            }
        }
        for (const auto& c : cliqueCandidates) {
            bool include = true;
            const auto& tmp = (*graph)[c];
            for (auto w : plan.addToClique) {
                if (!tmp.find(w)) {
                    include = false;
                }
            }
            if (include) {
                plan.addToClique.push_back(c);
            } else {
                plan.makeNeighboursOfU.push_back(c);
            }
        }
        return plan;
    }

    void executeMerge(const BranchChoice& choice, const MergePlan& plan) {
        state[choice.v] = choice.u;
        mergeCount += 1;
        for (const auto& w : plan.makeNeighboursOfU) {
            neighbours[w].push_back(choice.u);
        }
        // Vertex states must be updated before updating clique neighbours
        // so that state[x] == non_clique is consistent.
        cliqueSize += plan.addToClique.size();
        for (const auto& w : plan.addToClique) {
            state[w] = w;
        }
        for (const auto& w : plan.addToClique) {
            for (const auto& x : (*graph)[w]) {
                if (state[x] == non_clique) {
                    neighbours[x].push_back(w);
                }
            }
        }
        RUN_INVARIANT_CHECK
    }

    // Merge v into clique vertex u.
    [[gnu::always_inline]]
    MergePlan diveMerge(const BranchChoice& choice)  {
        const MergePlan& plan = planMerge(choice);
        executeMerge(choice, plan);
        return plan;
    }

    // Revert a call to diveMerge with the same arguments.
    void backtrackMerge(const BranchChoice& choice, const MergePlan& plan) {
        // Neighbours must be removed before any states are changed so this
        // loop runs exactly as it did in the call to diveMerge().
        for (const auto& w : plan.addToClique) {
            for (const auto& x : (*graph)[w]) {
                if (state[x] == non_clique) {
                    neighbours[x].pop_back();
                }
            }
        }
        for (const auto& w : plan.addToClique) {
            state[w] = non_clique;
        }
        cliqueSize -= plan.addToClique.size();
        for (const auto& w : plan.makeNeighboursOfU) {
            neighbours[w].pop_back();
        }
        state[choice.v] = non_clique;
        mergeCount -= 1;
        RUN_INVARIANT_CHECK
    }

    [[gnu::always_inline]]
    MergePlan diveRight(const BranchChoice& choice)  {
        return diveMerge(choice);
    }

    [[gnu::always_inline]]
    nullptr_t diveLeft(const BranchChoice& choice)  {
        diveDifference(choice);
        return nullptr;
    }

    [[gnu::always_inline]]
    void unwind(const BranchChoice& choice, const MergePlan& plan)  {
        backtrackMerge(choice, plan);
    }

    [[gnu::always_inline]]
    void unwind(const BranchChoice& choice, nullptr_t)  {
        backtrackDifference(choice);
    }

    [[gnu::always_inline]]
    BranchChoice getBranchingRule() const  {
        return getBranchChoice();
    }

    [[gnu::always_inline]]
    unsigned getObjective() const  {
        return cliqueSize;
    }

};


#endif  // SRC_GRAPHCOLORNODEVECTOR_HPP_
