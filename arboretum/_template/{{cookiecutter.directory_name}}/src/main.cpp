
#include <exception>
#include <string>

#include <cxxopts.hpp>

#include "../include/algorithm.hpp"

using namespace std;
using namespace {{cookiecutter.namespace}};


int main(int argc, char **argv) {
    cxxopts::Options options("Name", "Description");
    options.add_options()
        ("f,file", "Input File", cxxopts::value<string>())
        ("m,mode", "Tree Search Mode", cxxopts::value<string>())
        ;
    options.parse_positional({"file"});
    auto result = options.parse(argc, argv);
    ProblemType problem;  // read(result["file"].as<string>())
    if (result["mode"].as<string>() == "recursion") {
        auto solution = solve_recursive(problem);
    } else if (result["mode"].as<string>() == "backtrack") {
        auto solutions = solve_backtrack(problem, result["log"].as<unsigned>());
        cout << "Solution Pool: " << endl;
        for (const auto& solution : solutions) {
            cout << "  Obj = " << solution.get_objective_value() << ")" << endl;
        }
    } else {
        throw domain_error("Bad mode choice.");
    }
}
