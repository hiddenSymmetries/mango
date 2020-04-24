# Getting Started {#gettingStarted}

## Getting the code

First, you can get the MANGO source code by cloning the git repository:

    > git clone https://github.com/hiddenSymmetries/mango.git

## Dependencies

MANGO requires MPI. There are also several optional dependencies, including the [Catch2](https://github.com/catchorg/Catch2)
unit testing framework and various optimization libraries. Several of these dependencies
can be installed in the `external_packages` directory of MANGO. There, you can run
the scripts `install_nlopt`, `install_hopspack`, and `install_catch2` to download the relevant
packages. Note that Catch2 is not needed to run optimizations with MANGO, only to run the tests.
There is no script in the `external_packages` directory to install GSL or PETSc; these packages
are installed on many HPC systems, or you can install them yourself by consulting the documentation
for these packages.


## Building the MANGO library

Presently MANGO is built using gnu make. Settings that are specific to different computers, such as the paths to
various include files and libraries, are set in the file `makefile.system-dependent`. 
If you are building MANGO on a machine for which there is not a relevant section in this file,
you can copy one of the existing sections to make a new section for your machine.
Note that in this file, each machine has a section like
~~~~~
MANGO_PETSC_AVAILABLE = T
MANGO_HOPSPACK_AVAILABLE = F
MANGO_NLOPT_AVAILABLE = T
MANGO_GSL_AVAILABLE = T
~~~~~
These variables control which optization packages MANGO is built with. You can set each variable to T or F
as you like.

Then, the MANGO library and examples are built by running

    > make -j

(The `-j` option tells `make` to use multiple threads to make compiling faster.)

If successful, the files `mango/lib/libmango.a` and `mango/include/mango_mod.mod` will be created. The latter may have different capitalization, depending on the fortran compiler.
Also, executables for the examples will be built in `mango/examples/bin/`.