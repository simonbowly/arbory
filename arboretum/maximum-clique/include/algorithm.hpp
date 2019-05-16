#ifndef SRC_MAXIMUMCLIQUE_ALGORITHM_HPP_
#define SRC_MAXIMUMCLIQUE_ALGORITHM_HPP_

#include <optional>
#include <vector>

#include <arbory/struct/graph.hpp>

#include "types.hpp"

std::optional<MaximumCliqueSol> solve_recursive(const UndirectedGraph& graph);
std::vector<MaximumCliqueSol> solve_backtrack(const UndirectedGraph& graph, unsigned log_frequency);

#endif  // SRC_MAXIMUMCLIQUE_ALGORITHM_HPP_
