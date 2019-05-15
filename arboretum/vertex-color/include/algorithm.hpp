
#ifndef SRC_VERTEXCOLOR_ALGORITHM_HPP_
#define SRC_VERTEXCOLOR_ALGORITHM_HPP_

#include <utility>
#include <vector>

void solveStackBacktrackingVector(
    unsigned vertices,
    const std::vector<std::pair<unsigned, unsigned>>& edges,
    unsigned nodeLogFrequency);

#endif  // SRC_VERTEXCOLOR_ALGORITHM_HPP_
