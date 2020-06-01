# The Algorithm Database {#algorithmDatabase}

The master database of all the available optimization algorithms in MANGO is maintained in the file [algorithms.dat](algorithms_8dat_source.html).
In the same directory is a python script [update_algorithms](update__algorithms_source.html). When this script is run,
several sections of MANGO's C++ source and python testing scripts are updated based on the data in `algorithms.dat`. In this way,
all parts of MANGO's source and testing system are guaranteed to be consistent with respect to the available optimization algorithms and their properties.
For more information about the available algorithms and how to choose the algorithm, see @ref algorithms.

In the `algorithms.dat` file, each entry contains the name of the algorithm, the package it comes from (e.g. PETSc, GSL, etc), and various
properties of the algorithm, such as whether it applies to least-squares problems vs general problems, whether it uses derivatives, etc.
Blank lines are ignored, as are any lines beginning with `#`.
The algorithm names that users select (e.g. `petsc_pounders` or `gsl_nm`) are formed by concatenating the package name with an underscore and then the short
algorithm name listed in the `algorithms.dat` database.
Packages are discussed in @ref algorithms.
For algorithms implemented directly in MANGO itself, such as the parallelized Levenberg-Marquardt algorithm, the package listed in `algorithms.dat` is `mango`. For packages such as HOPSPACK that include
only a single algorithm, the short algorithm name in `algorithms.dat` can be blank, in which case the long algorithm name is just the package name.

The `parallel` entry in `algorithms.dat` indicates whether an algorithm can support concurrent evaluations of the objective function.
Algorithms for which `uses_derivatives` is `T` always are parallel in this sense, as finite difference evaluations of derivatives use
concurrent function evaluations. Algorithms that do not use derivatives can still be parallel; an example is HOPSPACK.

The `deterministic` entry in `algorithms.dat` indicates whether a given algorithm, for a given objective function and initial condition, always
results in the same sequence of objective function evaluations. An algorithm is not deterministic if it employs random numbers (as in `NLOPT_LN_PRAXIS`) or
if results depend on the order in which different MPI processes complete their work (as in `HOPSPACK`). 
Whether or not an algorithm
is deterministic matters only for regression testing: nondeterministic algorithms are not expected to give results that exactly match reference values.
Note that MANGO's finite difference derivative calculations are deterministic.

Several sections of MANGO's source code are written by [update_algorithms](update__algorithms_source.html). 
In `mango.hpp`, the enum `algorithm_type` and the `algorithms` array are populated.
Here, and everywhere that `update_algorithms` operates, the sections of code to be written are marked up, e.g.
with `// <database>` ... `// </database>`. The code within these markup regions should not be edited directly,
since it will be over-written the next time `update_algorithms` is run.

Instructions for adding algorithms to the database are given below. Note that any time a new algorithm is added,
you can add the algorithm to the regression testing system with the following two steps. First, add the new algorithm name
to `examples/input/test_parameters.<examplename>` for any examples that are appropriate. Second, add
a line for the new algorithm to the file `examples/output/short_summary.<examplename>.reference`.


# Adding an algorithm implemented directly in MANGO

To add an optimization algorithm that is implemented directly in MANGO itself,
first add a line for this algorithm in `algorithms.dat`. Then run `update_algorithms`.
Doing so will automatically generate corresponding lines in either
`src/api/optimize_mango.cpp` or `src/api/optimize_least_squares_mango.cpp`,
depending on whether the algorithm is for least-squares problems or general problems.
In particular, MANGO will now include a C++ header file, with the same name as the algorithm,
all lowercase except that the first letter must be capitalized. For instance `Levenberg_marquardt.hpp` or `Imfil.hpp`.
This header file should be located in `src/algorithms`. See `Levenberg_marquardt.hpp` and `Imfil.hpp`
for examples.
These header files should declare a subclass of mango::Algorithm or mango::LeastSquaresAlgorithm,
depending on whether the algorithm is for least-squares problems or general problems.
The name of the subclass must be the same as the algorithm name, all lowercase except that the first letter should be capitalized.
You can then implement the algorithm in `src/algorithms/<Algorithmname>.cpp`. Be sure to write unit tests
for the algorithm in `src/algorithms/tests/`!


# Adding an algorithm from an outside package that is already connected to MANGO

For packages such as PETSc and NLOpt that are already known by MANGO, if a new algorithm becomes supported by the package,
only a few steps are needed to add the new algorithm to MANGO. First, add a line for the algorithm in `algorithms.dat`. Then
run `update_algorithms`. Doing so will add the name of the new algorithm to 
the list of available algorithms in `mango.hpp` and mango::Problem::set_algorithm.
Now look at `src/api/optimize_<packagename>.cpp` or `src/api/optimize_least_squares_<packagename>.cpp`,
depending on whether the algorithm is for least-squares problems or general problems.
Any details specific to the package and the algorithm should go in this file.


# Adding a new package

To add a new algorithm from a new package that was not previously known by MANGO, you must carry out the following steps.
First, add a line for the algorithm in `algorithms.dat`. Then run `update_algorithms`. 
Doing so will add several lines to src/api/set_package.cpp corresponding to the new package.
In particular, MANGO will expect a new header file `Package_<packagename>.hpp` to exist,
and you must create this file in `src/api/`. This file must declare a subclass of mango::Package
with the name `mango::Package_<packagename>`. This subclass must include
concrete implementations of the abstract subroutines mango::Package::optimize and mango::Package::optimize_least_squares.
These subroutines can be implemented in files `src/api/optimize_<packagename>.cpp` and/or `src/api/optimize_least_squares_<packagename>.cpp`,
which you are responsible for creating. Any package-specific implementation details can go in these files.
You will also need to extend MANGO's build system to link to the new package.