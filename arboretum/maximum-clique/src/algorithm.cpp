
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
    auto solution = solve_recursive<MaximumCliqueState, Sense::Maximize>(&state);
    double runtime = std::chrono::duration<double, std::milli>
        (chrono::high_resolution_clock::now() - start)
        .count() / 1000;
    cout << "Time: " << runtime << " seconds" << endl;
    return solution;
}


vector<MaximumCliqueSol> solve_backtrack(const UndirectedGraph& graph, unsigned log_frequency) {
    vector<unsigned> initial_order;
    auto state = root_state(graph, &initial_order);
    state.sort_and_imply();
    Solver<MaximumCliqueState, Sense::Maximize> solver(&state);
    solver.solve(log_frequency);
    return solver.get_solutions();
}


// definitely can be improved (don't copy out the solution, just leave partitioned?)
vector<unsigned>::iterator solve_subgraph(const UndirectedGraph& graph, vector<unsigned>* vertices) {
    MaximumCliqueState state(graph, begin(*vertices), end(*vertices));
    state.sort_and_imply();
    auto solution = solve_recursive<MaximumCliqueState, Sense::Maximize>(&state);
    return partition(begin(*vertices), end(*vertices), [solution](unsigned u) {
        return find(begin(solution->get()), end(solution->get()), u) != end(solution->get());
    });
}
