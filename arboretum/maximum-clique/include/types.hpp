
#ifndef SRC_MAXIMUMCLIQUE_TYPES_HPP_
#define SRC_MAXIMUMCLIQUE_TYPES_HPP_

#include <algorithm>
#include <vector>

class MaximumCliqueSol {
    typedef std::vector<unsigned>::const_iterator iter;
    const std::vector<unsigned> result;
public:
    MaximumCliqueSol(iter begin, iter end) : result(begin, end) {}
    unsigned get_objective_value() const { return result.size(); }
    const std::vector<unsigned>& get() const { return result; }
    void print() const {
        std::cout << "[ ";
        std::for_each(std::begin(result), std::end(result),
                      [](int n){ std::cout << n << " "; });
        std::cout << "]";
    }
};

#endif  // SRC_MAXIMUMCLIQUE_TYPES_HPP_
