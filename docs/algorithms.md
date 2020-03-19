# Available Algorithms {#algorithms}

# Overview

MANGO provides interfaces to a wide variety of optimization algorithms,
including both local and global optimization, general and least-squares minimization,
and derivative-free and derivative-based algorithms. (For the latter, MANGO uses
finite difference derivatives.) Most of the algorithms are provided via interfaces
to outside software packages (GSL, PETSc, NLOPT, and HOPSPACK). MANGO can also
provide its own optimization algorithms. Presently there is one such algorithm,
`mango_levenberg_marquardt`.

The available algorithms are enumerated in mango::algorithm_type.

Each algorithm can be identified either by an integer index or by a string,
identical to the name of the integer index but lowercase. For instance, PETSc's POUNDERS algorithm
can be identified either with the constant mango::PETSC_POUNDERS or by the string `"petsc_pounders"`.
The string always begins with the name of the software package in which the algorithm is implemented:
`gsl`, `nlopt`, `petsc`, `hopspack`, or `mango` for the case of algorithms implemented directly in MANGO.
For packages that support multiple algorithms (all packages except HOPSPACK), the algorithm name then contains
an underscore, followed by the name of the specific algorithm.


# Selecting an algorithm

There are three ways to select an algorithm: from within your driver code by specifying the enumerated constant or string, or by loading in the string from a file.
From C++, the first two approaches are provided by mango::Problem::set_algorithm(), while the file-based approach is provided by mango::Problem::read_input_file().
For instance after declaring a problem instance

~~~~~{.cpp}
mango::Problem my_problem(N_dims, state_vector, &objective_function, argc, argv);
~~~~~

we could call

~~~~~{.cpp}
my_problem.set_algorithm(mango::GSL_BFGS);
~~~~~

or

~~~~~{.cpp}
my_problem.set_algorithm("gsl_bfgs");
~~~~~

~~~~~{.cpp}
my_problem.read_input_file("my_input_file");
~~~~~

In the last case, the input file should consist of 2 lines. The first line is an integer giving the number of worker groups, while the second line contains the string
representation of the algorithm name (e.g. `nlopt_ln_neldermead`).

# GSL (Gnu Scientific Library)

# NLOpt

# PETSc

# HOPSPACK

# Algorithms provided directly by MANGO