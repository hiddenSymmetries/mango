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

make test_make

./tests/install_nlopt.sh

make

make test