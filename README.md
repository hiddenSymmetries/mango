# MANGO
[![Build Status](https://travis-ci.org/landreman/mango.svg?branch=master)](https://travis-ci.org/landreman/mango)

Multiprocessor Algorithms for Nonlinear Gradient-free Optimization

MANGO is a library for optimization using MPI parallelism. It provides
a common interface for calling other optimization libraries, including non-parallel algorithms,
making them parallel by providing parallelized finite-difference gradient calculations. Some of MANGO's features are the following:

* MANGO makes it possible to try out different optimization algorithms (and even different libraries) without recompiling your code. The algorithm can be chosen with a runtime option.
* MANGO provides a common interface to access multiple optimization libraries. At present, MANGO has support for [GSL](https://www.gnu.org/software/gsl/doc/html/), [PETSc/TAO](https://www.mcs.anl.gov/petsc/), [NLOpt](https://nlopt.readthedocs.io/en/latest/), and [HOPSPACK](https://dakota.sandia.gov/packages/hopspack). 
* MANGO has minimal dependencies, requiring nothing more than MPI. If any of the libraries above are not available on your system, MANGO can be built without them.
* At present, MANGO is focused on optimization problems for which derivatives are not available. MANGO allows any derivative-based algorithm in the above libraries to be used with finite-difference gradients that are evaluated concurrently.
* MPI parallelism is allowed within the objective function, both for optimization algorithms that are serial and parallel. For parallel algorithms with parallelization within the objective function, MANGO helps you partition the available processors into worker groups.
* MANGO is presently callable from C++, C, and Fortran. Python support is planned in the future.
* MANGO includes algorithms both for nonlinear least-squares problems and conventional optimization. If your problem has least-squares structure and you want to try a non-least-squares algorithm, you don't need to do anything other than select the desired algorithm - MANGO will automatically convert the residuals to a single objective function.
* MANGO presently includes support for (optional) bound constraints. Support for nonlinear equality and inequality constraints is planned.
* MANGO includes a collection of test problems, which allow you to compare existing algorithms and try out new ones.
* For reliability, MANGO includes unit tests, integrated tests, regression tests, and continuous integration.


MANGO is being developed as part of the [Collaboration on Hidden Symmetries and Fusion Energy](https://hiddensymmetries.princeton.edu/). This work is supported by a grant from the Simons Foundation (560651, M.L.).