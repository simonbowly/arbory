
#include <iostream>
#include <utility>
#include <vector>
#include <gsl/gsl_assert>

#include "arbory/recursion.hpp"
#include "arbory/sense.hpp"

#include "../include/state.hpp"


using namespace std;


int main() {

    vector<pair<unsigned, unsigned>> edges;
        edges.emplace_back(0, 1);
        edges.emplace_back(0, 5);
        edges.emplace_back(0, 6);
        edges.emplace_back(0, 9);
        edges.emplace_back(0, 7);
        edges.emplace_back(1, 5);
        edges.emplace_back(1, 9);
        edges.emplace_back(1, 8);
        edges.emplace_back(7, 1);
        edges.emplace_back(9, 5);
        edges.emplace_back(2, 5);
        edges.emplace_back(2, 8);
        edges.emplace_back(5, 8);
    Graph graph(10, edges);

    vector<unsigned> initial_order {4, 7, 2, 3, 5, 6, 0, 9, 1, 8};
    MaximumCliqueState state(graph, begin(initial_order), end(initial_order));

    auto solution = solve_recursive<
        MaximumCliqueState, MaximumCliqueSol,
        unsigned int, Sense::Maximize>(&state);

    if (solution) {
        cout << "Solution: ";
        solution->print();
        cout << "   Objective: " << solution->get_objective_value() << endl;
    } else {
        cout << "Infeasible" << endl;
    }

}
