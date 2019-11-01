#!/bin/bash

echo Hello from travis.sh

pwd
ls
env
make test_make

echo
echo About to install NLOPT
echo

# Install nlopt
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
