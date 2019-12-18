#!/bin/bash

set -ex
# In the above line, "set -x" causes this script to exit as soon as any line fails. "set -e" causes each line of this script to be printed (with a + in front) before it is executed, so if a step fails, you can see from the travis log what command failed.

echo Hello from install_catch2.sh

pwd

mkdir catch2
cd catch2
wget https://github.com/catchorg/Catch2/releases/download/v2.11.0/catch.hpp
cd ..

echo Done installing Catch2
