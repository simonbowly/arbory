// Pretty much just boilerplate: use the state to define problem specific
// algorithms using each of the arbory templates. Add any custom uses of
// the template here. This header exposes the implemented algorithms without
// exposing the state class or arbory headers.

#ifndef SRC_PROB_ALGORITHM_HPP_
#define SRC_PROB_ALGORITHM_HPP_

#include <optional>
#include <vector>

#include "types.hpp"

namespace {{cookiecutter.namespace}} {

std::optional<SolutionType> solve_recursive(const ProblemType& graph);
std::vector<SolutionType> solve_backtrack(const ProblemType& graph, unsigned log_frequency);

}

#endif  // SRC_PROB_ALGORITHM_HPP_
