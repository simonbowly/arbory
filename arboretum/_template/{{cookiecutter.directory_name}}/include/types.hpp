// If custom types are needed, define them here. Means we only have
// to include the algorithm header for external use.

#ifndef SRC_PROB_TYPES_HPP_
#define SRC_PROB_TYPES_HPP_

namespace {{cookiecutter.namespace}} {

using ProblemType = ...;
using ObjectiveType = ...;

class SolutionType {
public:
    ObjectiveType get_objective_value() const;
};

}

#endif  // SRC_PROB_TYPES_HPP_
