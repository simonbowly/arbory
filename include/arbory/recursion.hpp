#ifndef SRC_RECURSION_HPP
#define SRC_RECURSION_HPP

#include <optional>
#include <gsl/gsl_assert>

#include "sense.hpp"


// Template function carrying out a simple recursion on the problem state.
// Returns an optimal solution with a strictly larger objective than the value
// of primal_bound passed in, or nullopt if such a solution cannot be found.
//
// Required methods:
//
//      State:
//          is_leaf()
//          is_feasible()
//          get_solution()
//          get_upper_bound()/get_lower_bound()
//          branch_decision()
//          // ... for each Decision class
//          DecisionResult branch(Decision)
//          backtrack(DecisionResult)
//
//      Sol:
//          get_objective_value()
//
template <typename State, typename Sol, typename Obj, Sense sense>
std::optional<Sol> solve_recursive(State* state, Obj primal_bound) {
    using opt = SenseOps<sense>;
    if (opt::can_be_pruned(*state, primal_bound))
        return std::nullopt;
    if (!state->is_feasible())
        return std::nullopt;
    if (state->is_leaf())
        return state->get_solution();
    // Subproblem is incomplete, still improving and still feasible. Need to branch.
    auto [first, second] = state->branch_decision();
    // Explore the right branch, backtrack and update primal bound.
    auto first_result = state->branch(first);
    auto best = solve_recursive<State, Sol, Obj, sense>(state, primal_bound);
    state->backtrack(first_result);
    if (best) {
        bool cond = opt::is_improvement(
            best->get_objective_value(), primal_bound);
        Expects(cond);      // due to pruning
        primal_bound = best->get_objective_value();
        if (opt::can_be_pruned(*state, primal_bound))
            return best;
    }
    // Explore the alternative branch, backtrack and return best result.
    auto second_result = state->branch(second);
    auto other = solve_recursive<State, Sol, Obj, sense>(state, primal_bound);
    state->backtrack(second_result);
    Expects((!best) || (primal_bound == best->get_objective_value()));
    if ((!best) || (other && opt::is_improvement(
            other->get_objective_value(), primal_bound)))
        return other;
    return best;
}


template <typename State, typename Sol, typename Obj, Sense sense>
std::optional<Sol> solve_recursive(State* state) {
    return solve_recursive<State, Sol, Obj, sense>(
        state, initial_primal_bound<Obj, sense>());
}

#endif  // SRC_RECURSION_HPP
