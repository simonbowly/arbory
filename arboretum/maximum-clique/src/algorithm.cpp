
#include <chrono>
#include <vector>

#include <arbory/backtracking.hpp>
#include <arbory/recursion.hpp>
#include <arbory/sense.hpp>
#include <gsl/gsl_assert>

#include "../include/algorithm.hpp"

using namespace std;


MaximumCliqueState root_state(const UndirectedGraph& graph, vector<unsigned>* initial_order) {
    Expects(initial_order->empty());
    initial_order->reserve(graph.vertices());
    for (unsigned i = 0; i < graph.vertices(); i++) {
        initial_order->push_back(i);
    }
    return MaximumCliqueState(graph, begin(*initial_order), end(*initial_order));
}


optional<MaximumCliqueSol> solve_recursive(const UndirectedGraph& graph) {
    vector<unsigned> initial_order;
    auto state = root_state(graph, &initial_order);
    auto start = chrono::high_resolution_clock::now();
    auto solution = solve_recursive<
        MaximumCliqueState, MaximumCliqueSol,
        unsigned int, Sense::Maximize>(&state);
    double runtime = std::chrono::duration<double, std::milli>
        (chrono::high_resolution_clock::now() - start)
        .count() / 1000;
    cout << "Time:      " << runtime << " seconds" << endl;
    return solution;
}


vector<MaximumCliqueSol> solve_backtrack(const UndirectedGraph& graph) {
    vector<unsigned> initial_order;
    auto state = root_state(graph, &initial_order);
    using MaxCliqueSolver = Solver<
        MaximumCliqueState, MaximumCliqueSol, unsigned int, Sense::Maximize,
        DiveInclude, DiveExclude, DiveIncludeResult, DiveExcludeResult>;
    MaxCliqueSolver solver(&state);
    solver.solve(1000);
    return solver.get_solutions();
}
