
#include <iostream>
#include <vector>

#include <arbory/backtracking.hpp>
#include <arbory/recursion.hpp>
#include <arbory/sense.hpp>

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
    UndirectedGraph graph(10, edges);

    {
        cout << "========= RECURSION ==========" << endl;
        vector<unsigned> initial_order {4, 7, 2, 3, 5, 6, 0, 9, 1, 8};
        MaximumCliqueState state(graph, begin(initial_order), end(initial_order));
        auto solution = solve_recursive<
            MaximumCliqueState, MaximumCliqueSol,
            unsigned int, Sense::Maximize>(&state);
        cout << "Solution: " << endl;
        cout << "  (Obj = " << solution->get_objective_value() << ")  ";
        solution->print();
        cout << endl;
    }

    {
        cout << endl << "===== STACK BACKTRACKING =====" << endl;
        vector<unsigned> initial_order {4, 7, 2, 3, 5, 6, 0, 9, 1, 8};
        MaximumCliqueState state(graph, begin(initial_order), end(initial_order));
        using MaxCliqueSolver = Solver<
            MaximumCliqueState, MaximumCliqueSol, unsigned int, Sense::Maximize,
            DiveInclude, DiveExclude, DiveIncludeResult, DiveExcludeResult>;
        MaxCliqueSolver solver(&state);
        solver.solve(100);
        cout << "Solution Pool: " << endl;
        for (const auto& solution : solver.get_solutions()) {
            cout << "  (Obj = " << solution.get_objective_value() << ")  ";
            solution.print();
            cout << endl;
        }
    }

}
