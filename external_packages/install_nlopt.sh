#!/bin/bash

# Copyright 2019, University of Maryland and the MANGO development team.
#
# This file is part of MANGO.
#
# MANGO is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# MANGO is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with MANGO.  If not, see
# <https://www.gnu.org/licenses/>.

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
