This subdirectory contains several examples that illustrate how to call mango from C++ and fortran.
These same examples are also used for regression/functional testing, i.e. making sure that certain results do not change when modifications are made to the code.

For regression/functional testing, the following checks are performed:

* If a deterministic algorithm is used, then the optimum x* and f(x*) should be independent (to all digits) of the number of MPI processors used. The same is true of the number of function evaluations.

* For each example that has both C++ and fortran versions, and for each deterministic algorithm, the optimum x* and f(x*) should be independent (to all digits) of the language used.
The same is true of the number of function evaluations.

* For each example and each algorithm (including nondeterministic algorithms), the optimum x* and f(x*) should match a reference value in the repository
within. The match should be exact for deterministic algorithms, while the match will only be within some tolerance for nondeterministic algorithms.
For deterministic algorithms, the number of function evaluations should also match exactly.