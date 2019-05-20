
Below shows an implementation of static branching, used if there is a fundamental difference between the structure of the two branches.
The flow control is handled by the distinction between types.

An example is the merge/difference branches in graph colouring.
The two operations on the state data structure and very different and require different methods to backtrack effectively.
Furthermore, the choice of first branch is always static; we always want to assign colours before adding edges.

~~~~cpp
class ProblemState {
public:
    pair<Decision, Result> branch();
    void backtrack(const Decision& decision, const Result& result);
    ResultAlt branch_alternate(const Decision& decision);
    void backtrack(const Decision& decision, const ResultAlt& result);
    bool is_leaf() const;
    bool is_feasible() const;
    pair<SolutionType, ObjectiveType> get_solution() const;
    ObjectiveType get_upper_bound() const;      // If maximising.
    ObjectiveType get_lower_bound() const;      // If minimising.
};
~~~~

Use dynamic branching if there isn't a fundamental difference between the operations and the choice between the two is made at run-time.
For example, assigning a variable to a value would likely use dynamic branching.
In this case we would want to determine which value to assign dynamically, rather than having, for example, `AssignTrue` and `AssignFalse` classes.
Backtracking behaviour in this case is common, handled by one function which must backtrack based on the information contained in the `Result` object, without knowledge of whether `branch` or `branch_alternate` was the most recent call.

~~~~cpp
    pair<Decision, Result> branch();
    Result branch_alternate(const Decision& decision);
    void backtrack(const Decision& decision, const Result& result);
~~~~

The `branch` method returns a pair to allow simultaneous calculation of the rule and transition to the child state.
It would be highly unusual to separate these steps: calculating the decision doesn't get us far unless we actually make the state transition to check the corresponding bound or feasibility.
`backtrack` and `branch_alternate` can then be used to escape this state and try another branch.
We will need to provide an additional `branch` method, taking at least the `Decision` type to re-enter a branch we had previously abandoned.
This is not necessary in backtracking so is not required so far.

Since we check feasibility before we check leafy-ness, `is_leaf` is really `is_complete_feasible_solution`.
It should indicate there are no more branches to explore below this node and a solution should be returned.
If there are no more branches below the node because it is infeasible, then `is_feasible` should return `false`; it will be checked first and the solver will not attempt to extract a solution.

`get_solution` is only called when a feasible state representing a complete solution is reached.
It should return the objective function value and an object representing the solution itself.
This is returned as a pair to avoid the need to write a wrapper class which recalculates or stores the objective function value.
A solution is considered immutable, with fixed objective function value, so the solver should handle tracking of objective function values.
