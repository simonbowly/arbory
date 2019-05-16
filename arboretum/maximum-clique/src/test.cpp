
#include <iostream>
#include <vector>

#include <arbory/backtracking.hpp>
#include <arbory/recursion.hpp>
#include <arbory/sense.hpp>

#include "../include/algorithm.hpp"


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
        auto solution = solve_recursive(graph);
        cout << "Solution: " << endl;
        cout << "  (Obj = " << solution->get_objective_value() << ")  ";
        solution->print();
        cout << endl;
    }

    {
        cout << "======== BACKTRACKING ========" << endl;
        auto solutions = solve_backtrack(graph, 10);
        cout << "Solution Pool: " << endl;
        for (const auto& solution : solutions) {
            cout << "  (Obj = " << solution.get_objective_value() << ")  ";
            solution.print();
            cout << endl;
        }
    }

    {
        cout << "========= SUBGRAPH ==========" << endl;
        vector<unsigned> vertices {4, 7, 5, 6, 0, 9};
        auto mid = solve_subgraph(graph, &vertices);
        cout << "Clique: ";
        for_each(begin(vertices), mid, [](int n){ cout << n << " "; });
        cout << endl;
        cout << "Other: ";
        for_each(mid, end(vertices), [](int n){ cout << n << " "; });
        cout << endl;
    }

}
