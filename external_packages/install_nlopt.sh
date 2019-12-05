#!/bin/bash

set -ex
# In the above line, "set -x" causes this script to exit as soon as any line fails. "set -e" causes each line of this script to be printed (with a + in front) before it is executed, so if a step fails, you can see from the travis log what command failed.

echo Hello from install_nlopt.sh

pwd

mkdir nlopt
cd nlopt
wget https://github.com/stevengj/nlopt/archive/v2.6.1.tar.gz
tar -xzvf v2.6.1.tar.gz
cd nlopt-2.6.1
mkdir build
mkdir install
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make
make install
cd ../../..
ls nlopt/nlopt-2.6.1/install/*

echo Done installing NLOPT
