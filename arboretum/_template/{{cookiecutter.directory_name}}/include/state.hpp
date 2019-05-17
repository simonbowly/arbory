// Define state representation of the problem with transition and query methods.

#ifndef SRC_PROB_STATE_HPP_
#define SRC_PROB_STATE_HPP_

#include <utility>

#include "types.hpp"

namespace {{cookiecutter.namespace}} {

struct Right {};
struct Left {};
struct RightResult {};
struct LeftResult {};

class ProblemState {
public:
    std::pair<Right, Left> branch_decision() const;
    RightResult branch(const Right& decision);
    LeftResult branch(const Left& decision);
    void backtrack(const Right& decision, const RightResult& result);
    void backtrack(const Left& decision, const LeftResult& result);
    bool is_leaf() const;
    bool is_feasible() const;
    ObjectiveType get_upper_bound() const;
    SolutionType get_solution() const;
};

}

#endif  // SRC_PROB_STATE_HPP_
