// Copyright [2019] <Simon Bowly>

#ifndef SRC_VERTEXCOLOR_STATE_HPP_
#define SRC_VERTEXCOLOR_STATE_HPP_

#include <algorithm>
#include <iostream>
#include <memory>
#include <limits>
#include <utility>
#include <vector>

#include <gsl/gsl_assert>
#include <arbory/struct/graph.hpp>

#include "../../maximum-clique/include/algorithm.hpp"


struct Merge {
    unsigned u;
    unsigned v;
};

struct MergeResult {
    std::vector<unsigned> makeNeighboursOfU;
    std::vector<unsigned> addToClique;
};

struct Difference {
    unsigned u;
    unsigned v;
};

struct DifferenceResult {};

class VertexColorSol {
    unsigned obj;
 public:
    explicit VertexColorSol(unsigned v) : obj(v) {}
    unsigned get_objective_value() const {
        return obj;
    }
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
    const UndirectedGraph& graph;
    std::vector<unsigned> state;
    std::vector<std::vector<unsigned>> neighbours;
    unsigned cliqueSize;
    unsigned mergeCount;

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
        if (is_leaf() != expectComplete) {
            throw std::domain_error("Complete flag value is incorrect.");
        }
    }
    #endif

    Node(const Node& a) = default;
    Node& operator=(const Node&) = default;

 public:
    explicit Node(const UndirectedGraph& g) :
        graph(g), state(g.vertices(), non_clique),
        neighbours(g.vertices()), cliqueSize(0), mergeCount(0) {}
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

    void initialise() {
        Expects(std::all_of(
            std::begin(state), std::end(state),
            [](unsigned val) { return val == non_clique; }));
        // Initialised with n non-clique states, and n empty neighbour lists.
        // Sets the current graph clique (reset are non-clique by default).
        auto clique = solve_recursive(graph);
        for (const auto& u : clique->get()) {
            state[u] = u;
        }
        cliqueSize = clique->get().size();
        // Construct variable graph states.
        for (const auto& u : clique->get()) {
            for (const auto& v : graph[u]) {
                if (state[v] == non_clique) {
                    neighbours[v].push_back(u);
                }
            }
        }
        #ifndef NDEBUG
        checkInvariant();
        #endif
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
        Ensures(state[v] == non_clique);
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

    MergeResult planMerge(const Merge& choice) const {
        MergeResult plan;
        // Non-clique neighbours of v not already neighbours of u
        // either need neighbours updated or are clique candidates.
        for (const auto& w : graph[choice.v]) {
            if (state[w] == non_clique) {
                const auto& nw = neighbours[w];
                if (std::find(nw.begin(), nw.end(), choice.u) == nw.end()) {
                    if (nw.size() == cliqueSize - 1) {
                        plan.addToClique.push_back(w);
                    } else {
                        plan.makeNeighboursOfU.push_back(w);
                    }
                }
            }
        }
        // Divide cliqueCandidates into a sub-clique and others.
        // Add clique to plan.addToClique and other to plan.makeNeighboursOfU.
        // Not sure this has turned out faster than a greedy algorithm.
        if (plan.addToClique.size() > 1) {
            auto mid = solve_subgraph(graph, &plan.addToClique);
            for (auto it = mid; it != std::end(plan.addToClique); ++it) {
                plan.makeNeighboursOfU.push_back(*it);
            }
            plan.addToClique.erase(mid, std::end(plan.addToClique));
        }
        return plan;
    }

    void executeMerge(const Merge& choice, const MergeResult& plan) {
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
            for (const auto& x : graph[w]) {
                if (state[x] == non_clique) {
                    neighbours[x].push_back(w);
                }
            }
        }
        RUN_INVARIANT_CHECK
    }

    std::pair<Merge, Difference> branch_decision() const {
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
        Merge m;
        Difference d;
        m.v = getMaxDSATVertex();
        m.u = getMergeCandidate(m.v);
        d.v = m.v;
        d.u = m.u;
        Expects(branchChoiceIsValid(m));
        return std::make_pair(m, d);
    }

    bool branchChoiceIsValid(const Merge& choice) const {
        const auto& nv = neighbours[choice.v];
        return (
            choice.u < state.size()
            && choice.v < state.size()
            && state[choice.u] == choice.u
            && state[choice.v] == non_clique
            && std::find(nv.begin(), nv.end(), choice.u) == nv.end());
    }

    // Merge v into clique vertex u.
    MergeResult branch(const Merge& choice) {
        const MergeResult& plan = planMerge(choice);
        executeMerge(choice, plan);
        return plan;
    }

    // Revert a call to diveMerge with the same arguments.
    void backtrack(const Merge& choice, const MergeResult& plan) {
        // Neighbours must be removed before any states are changed so this
        // loop runs exactly as it did in the call to diveMerge().
        for (const auto& w : plan.addToClique) {
            for (const auto& x : graph[w]) {
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

    // Add an edge between u and v.
    DifferenceResult branch(const Difference& choice) {
        if (neighbours[choice.v].size() == cliqueSize - 1) {
            state[choice.v] = choice.v;
            cliqueSize += 1;
            for (auto w : graph[choice.v]) {
                if (state[w] == non_clique) {
                    neighbours[w].push_back(choice.v);
                }
            }
        } else {
            neighbours[choice.v].push_back(choice.u);
        }
        RUN_INVARIANT_CHECK
        DifferenceResult res;
        return res;
    }

    // Revert a call to diveDifference with the same arguments.
    void backtrack(const Difference& choice, const DifferenceResult&) {
        if (state[choice.v] == choice.v) {
            state[choice.v] = non_clique;
            cliqueSize -= 1;
            for (auto w : graph[choice.v]) {
                if (state[w] == non_clique) {
                    neighbours[w].pop_back();
                }
            }
        } else {
            neighbours[choice.v].pop_back();
        }
        RUN_INVARIANT_CHECK
    }

    bool is_leaf() const {
        return (cliqueSize + mergeCount) == state.size();
    }

    constexpr bool is_feasible() const { return true; }

    unsigned get_lower_bound() const  {
        return cliqueSize;
    }

    VertexColorSol get_solution() const {
        return VertexColorSol(cliqueSize);
    }

};


#endif  // SRC_VERTEXCOLOR_STATE_HPP_
