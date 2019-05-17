
#include <arbory/backtracking.hpp>
#include <arbory/recursion.hpp>
#include <arbory/sense.hpp>

#include "../include/algorithm.hpp"
#include "../include/state.hpp"
#include "../include/types.hpp"

using namespace std;
using namespace {{cookiecutter.namespace}};


ProblemState root_state(const ProblemType& problem) {
    // Build an initial state for the problem.
}


optional<SolutionType> solve_recursive(const ProblemType& problem) {
    auto state = root_state(problem);
    return solve_recursive<ProblemState, SolutionType, ObjectiveType, Sense::Maximize>(&state);
}


vector<SolutionType> solve_backtrack(const ProblemType& problem, unsigned log_frequency) {
    auto state = root_state(problem);
    Solver<ProblemState, SolutionType, ObjectiveType, Sense::Maximize,
           Right, Left, RightResult, LeftResult>
        solver(&state);
    solver.solve(log_frequency);
    return solver.get_solutions();
}
