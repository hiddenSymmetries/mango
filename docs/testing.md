# Testing {#testing}

## Unit tests

Unit tests are implemented using the [Catch2](https://github.com/catchorg/Catch2)
framework. The tests are written in `mango/src/api/tests`. When MANGO is compiled,
the unit tests are put in the executable `mango/tests/unit_tests`. There are many
options for this executable, which can be found by running

    ~/mango/tests> ./unit_tests --help

The `unit_tests` executable runs with however many MPI processes are given to it.
So, `./unit_tests` runs with a single process, `mpiexec -n 2 ./unit_tests` runs with 2 processes, etc.
The python script `mango/tests/run_mpi_unit_tests` automatically runs the `unit_tests` exectuable
for 1, 2, 3, 4, and 5 MPI processes, to make sure the tests pass for all cases.

To add a new unit test, you can use the examples in `mango/src/api/tests/` as a model,
and look at the [Catch2 documentation](https://github.com/catchorg/Catch2).
MANGO considers the unit tests to be defined by the variable `TEST_SRC_FILES` of the main makefile.
You can run

    ~/mango> make test_make

to print out the makefile variables and examine `TEST_SRC_FILES` to make sure it includes your
new test.

## Integrated and regression tests

Integrated/regression tests are incorporated using the examples in `mango/examples/`.
Each example consists of source code in `mango/examples/src/`, an input file
 `mango/examples/input/mango_in.*`, a file `mango/examples/input/test_parameters.*`,
and a reference output file `mango/examples/output/short_summary.*.reference`.
The `mango_in.*` file contains 2 lines, listing the number of worker groups and the optimization
algorithm, and these entries are re-written by the testing framework when the test is
run to vary these two parameters. The `test_parameters.*` file has a list
of the number of MPI processes to try, the number of worker groups to try,
and the optimization algorithms to try. 



## Running the tests

From the root MANGO directory, you can run

    ~/mango> make test

to run both the unit tests and integrated/regression tests. If you want to run only the unit tests, you can instead run

    ~/mango> make unit_tests


## Continuous integration