#!/bin/bash

set -ex
# In the above line, "set -x" causes this script to exit as soon as any line fails. "set -e" causes each line of this script to be printed (with a + in front) before it is executed, so if a step fails, you can see from the travis log what command failed.

echo Hello from travis.sh


pwd
ls
env
which mpicc
which mpif90
which gfortran
which mpiexec

cd external_packages
./install_catch2.sh
./install_nlopt.sh
#./install_hopspack.sh
cd ..

make test_make

# Executables need to know where to find the nlopt shared library:
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/travis/build/landreman/mango/external_packages/nlopt/nlopt-2.6.1/install/lib

make -j

make test