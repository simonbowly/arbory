
#include <chrono>
#include <vector>

#include <arbory/backtracking.hpp>
#include <arbory/recursion.hpp>
#include <arbory/sense.hpp>
#include <gsl/gsl_assert>

#include "../include/algorithm.hpp"
#include "../include/state.hpp"

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
    state.sort_and_imply();
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


vector<MaximumCliqueSol> solve_backtrack(const UndirectedGraph& graph, unsigned log_frequency) {
    vector<unsigned> initial_order;
    auto state = root_state(graph, &initial_order);
    state.sort_and_imply();
    using MaxCliqueSolver = Solver<
        MaximumCliqueState, MaximumCliqueSol, unsigned int, Sense::Maximize,
        DiveInclude, DiveExclude, DiveIncludeResult, DiveExcludeResult>;
    MaxCliqueSolver solver(&state);
    solver.solve(log_frequency);
    return solver.get_solutions();
}
