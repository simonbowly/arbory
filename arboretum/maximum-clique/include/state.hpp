#ifndef SRC_MAXIMUMCLIQUE_STATE_HPP_
#define SRC_MAXIMUMCLIQUE_STATE_HPP_

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include <gsl/gsl_assert>
#include <arbory/struct/graph.hpp>

#include "types.hpp"


// Branch division includes/excludes the given vertex.
class Rule {
    unsigned vertex;
public:
    explicit Rule(unsigned v) : vertex(v) {}
    unsigned get_vertex() const { return vertex; }
};


// Stores how far the neighbours_end iterator was moved so the include branch
// can be backtracked.
class IncludeResult {
    unsigned clique_move;
    unsigned neighbours_move;
public:
    explicit IncludeResult(unsigned c, unsigned n) :
        clique_move(c), neighbours_move(n) {}
    unsigned get_clique_move() const { return clique_move; }
    unsigned get_neighbours_move() const { return neighbours_move; }
};


// Reversal is always the same for an exclude branch, no data needs to be
// stored here.
class ExcludeResult {
    unsigned clique_move;
public:
    explicit ExcludeResult(unsigned c) : clique_move(c) {}
    unsigned get_clique_move() const { return clique_move; }
};


//
// Maximum clique state representation. e.g.
//
//  state = [  1  4  5  8  2  6  7  3  0  9  ]
//             B        C           N        E
//
// The :state array contains the set of vertices over which we are searching
// for a maximum clique. Two pointers, clique_end (C) and neighbours_end (N)
// are kept.
// All vertices to the left of C are currently in the clique. In the above
// example it has been verified that {1, 4, 5} in the above example form a
// clique in the target graph.
// Vertices in the range [C, N) are candidates to be added to the clique. In the
// above example, {8, 2, 6, 7} are all adjacent to all vertices in {1, 4, 5}.
// but it has not been checked whether they are adjacent to one another).
// Vertices at and to the right of N cannot be added to the current clique at
// this branch since they have been determined in previous steps not to be
// neighbours of some vertex in {1, 4, 5}.
//
// To transition to an 'include' state, a new vertex from the range C->N is
// selected by advancing the pointer C. The new range C->N is partitioned into
// neighbours and non-neighbours of the branch vertex.
//
//  state = [  1  4  5  8  7  6  2  3  0  9  ]
//             B           C     N           E
//
// In thie example, vertex 8 is added to the clique (this is valid because it
// was already vertified as a neighbour of all vertices in {1, 4, 5}). The
// partion and update divides the set of candidates into {7, 6} which are
// neighbours of 8 and {2} which are not. Thus {7, 6} are continuing candidates
// for inclusion but all vertices to the right of N cannot be added to the
// clique in this branch.
//
// The opposing 'exclude' branch swaps the vertex 8 into the set [N, E) to ensure
// it is not considered in any further search. Only {2, 6, 7} are considered
// below this state.
//
//  state = [  1  4  5  7  2  6  8  3  0  9  ]
//             B        C        N           E
//
class MaximumCliqueState {
    using Iter = std::vector<unsigned>::iterator;

    const UndirectedGraph & graph;
    Iter state_begin;
    Iter clique_end;
    Iter neighbours_end;
    Iter state_end;

public:
    MaximumCliqueState(const UndirectedGraph& g, Iter b, Iter e) :
        graph(g), state_begin(b), clique_end(b),
        neighbours_end(e), state_end(e) {}

    // Brings the next vertex to be branched on to the first candidate position.
    // The branch vertex is included by implication if possible.
    void sort_and_imply() {
        while (true) {
            if (is_leaf())
                break;
            std::partial_sort(clique_end, clique_end + 1, neighbours_end, [this](unsigned u, unsigned v) {
                return graph.degree(u) < graph.degree(v);
            });
            bool noimply = std::any_of(clique_end + 1, neighbours_end, [this](unsigned v) {
                return !graph.adjacent(*clique_end, v);
            });
            if (noimply)
                break;
            ++clique_end;
        }
    }

    // Alter the state to check the include(v) branch. The returned result object
    // stores how far the partitioning operation moved the neighbours_end iterator
    // (i.e. how many vertices were rejected as a result of including the branch
    // vertex in the clique) to allow backtracking.
    std::pair<Rule, IncludeResult> branch() {
        Rule decision(*clique_end);
        auto prev_clique_end = clique_end;
        auto prev_neighbours_end = neighbours_end;
        // Add the branch vertex to the clique and update candidate set.
        ++clique_end;
        neighbours_end = std::partition(clique_end, neighbours_end, [this, decision](unsigned v) {
            return graph.adjacent(decision.get_vertex(), v);
        });
        // Expects(neighbours_end != prev_neighbours_end);
        // Look for implied inclusions, record pointer movements for backtracking.
        sort_and_imply();
        return std::make_pair(decision, IncludeResult(
            clique_end - prev_clique_end,
            prev_neighbours_end - neighbours_end));
    }

    // Revert a call to branch(Rule), transitioning to the parent state.
    void backtrack(const Rule&, const IncludeResult& result) {
        clique_end -= result.get_clique_move();
        neighbours_end += result.get_neighbours_move();
    }

    // Alter the state to check the exclude(v) branch.
    ExcludeResult branch_alternate(const Rule& decision) {
        Expects(*clique_end == decision.get_vertex());
        auto prev_clique_end = clique_end;
        // Move the branch vertex into the excluded set.
        --neighbours_end;
        std::swap(*clique_end, *neighbours_end);
        // Look for implied inclusions, record pointer movement.
        sort_and_imply();
        return ExcludeResult(clique_end - prev_clique_end);
    }

    // Reverts a call to branch(Rule), transitioning to the parent state.
    void backtrack(const Rule& decision, const ExcludeResult& result) {
        Expects(*neighbours_end == decision.get_vertex());
        clique_end -= result.get_clique_move();
        ++neighbours_end;
    }

    // Return whether a leaf has been reached (there are no more candidates
    // to check below this state).
    bool is_leaf() const {
        return clique_end == neighbours_end;
    }

    constexpr bool is_feasible() const { return true; }

    // Returns the best-case objective function which could potentially be
    // found below this node.
    unsigned get_upper_bound() const {
        return neighbours_end - state_begin;
    }

    // If at a leaf state, return the solution.
    MaximumCliqueSol get_solution() const {
        Expects(is_leaf());
        return MaximumCliqueSol(state_begin, clique_end);
    }

    void print_state() const {
        std::for_each(state_begin, state_end,
                      [](int n){ std::cout << n << " "; });
        std::cout << std::endl;
        for (unsigned i = 0; i < clique_end - state_begin; i++) {
            std::cout << "  ";
        }
        std::cout << "C";
        if (neighbours_end == clique_end) {
            std::cout << "N";
        } else {
            std::cout << " ";
            for (unsigned i = 0; i < neighbours_end - clique_end - 1; i++) {
                std::cout << "  ";
            }
            std::cout << "N";
        }
        std::cout << std::endl;
    }

};

#endif  // SRC_MAXIMUMCLIQUE_STATE_HPP_
