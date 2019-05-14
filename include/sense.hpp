#ifndef SRC_SENSE_HPP
#define SRC_SENSE_HPP

// Uses 'tag dispatch' to select appropriate methods for min/max problems.

#include <limits>
#include <type_traits>


enum Sense {
    Maximize, Minimize
};


// Maximisation case.
template <typename State, typename Obj>
bool can_be_pruned_impl(std::true_type, const State& state, const Obj& lower_bound) {
    return state.get_upper_bound() <= lower_bound;
}


// Minimisation case.
template <typename State, typename Obj>
bool can_be_pruned_impl(std::false_type, const State& state, const Obj& upper_bound) {
    return state.get_lower_bound() >= upper_bound;
}


// Maximisation case.
template <typename Obj>
bool is_improvement_impl(std::true_type, const Obj& objective_value, const Obj& lower_bound) {
    return objective_value > lower_bound;
}


// Minimisation case.
template <typename Obj>
bool is_improvement_impl(std::false_type, const Obj& objective_value, const Obj& upper_bound) {
    return objective_value < upper_bound;
}


template <Sense sense>
class SenseOps {
public:
    template <typename Obj>
    static bool is_improvement(const Obj& objective_value, const Obj& primal_bound) {
        return is_improvement_impl(
            std::bool_constant<sense == Sense::Maximize>(),
            objective_value, primal_bound);
    }

    template <typename State, typename Obj>
    static bool can_be_pruned(const State& state, const Obj primal_bound) {
        return can_be_pruned_impl(
            std::bool_constant<sense == Sense::Maximize>(),
            state, primal_bound);
    }
};


// Returns a worst-case primal bound given the objective type and sense.
template<typename Obj, Sense sense>
constexpr Obj initial_primal_bound() {
    if (sense == Sense::Maximize) {
        return std::numeric_limits<Obj>::min();
    } else {
        return std::numeric_limits<Obj>::max();
    }
}

#endif // SRC_SENSE_HPP
