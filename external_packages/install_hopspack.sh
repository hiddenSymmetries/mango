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

echo Hello from install_hopspack

cd hopspack

wget https://dakota.sandia.gov/sites/default/files/hopspack-2.0.2-src.tar.gz

tar -xzvf hopspack-2.0.2-src.tar.gz

#mkdir build_mpi
#cd build_mpi
#cmake ../hopspack-2.0.2-src -Dmpi=yes -DLAPACK_ADD_LIBS=/usr/lib/libblas.dylib

#make

# Rather than use the cmake build system that standalone HOPSPACK uses, here I instead use standard 'make'. 
# There are several reasons. One is that I need to change the build system anyway in order to link
# HOPSPACK and mango. Another motivation is to remove the dependency on cmake.

# Copy the original HOPSPACK source files to a new src directory:
cp hopspack-2.0.2-src/src/src*/*.cpp src
cp hopspack-2.0.2-src/src/src*/*.hpp src
cp hopspack-2.0.2-src/src/src-citizens/citizen*/*.* src
cp hopspack-2.0.2-src/src/src-citizens/citizen-gss/cddlib/* src

# Copy the mango modifications over:
cp src_modifications/* src

# Remove files for the serial and multithreaded versions of hopspack:
rm src/HOPSPACK_main_serial.cpp
rm src/HOPSPACK_main_threaded.cpp
rm src/HOPSPACK*Thread*.cpp

#make -j

echo Done installing HOPSPACK. Compiling will occur when you compile mango.