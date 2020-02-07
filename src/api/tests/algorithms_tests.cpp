// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

#include "catch.hpp"
#include "mango.hpp"


TEST_CASE("does_algorithm_exist(): Verify that return value is true for all the algorithms.","[algorithms]") {
  for (int j = 0; j < mango::NUM_ALGORITHMS; j++) {
    CHECK(mango::does_algorithm_exist(mango::algorithms[j].name));
  }
}

TEST_CASE("does_algorithm_exist(): Verify that return value is false for a bunch of strings that are not algorithm names.","[algorithms]") {
  CHECK(!mango::does_algorithm_exist(" "));
  for (int j = 0; j < mango::NUM_ALGORITHMS; j++) {
    CHECK(!mango::does_algorithm_exist(mango::algorithms[j].name + " "));
    CHECK(!mango::does_algorithm_exist(" " + mango::algorithms[j].name));
  }
}

TEST_CASE("get_algorithm(): Verify that return value is consistent with the algorithms array.","[algorithms]") {
  for (int j = 0; j < mango::NUM_ALGORITHMS; j++) {
    mango::algorithm_type alg;
    mango::get_algorithm(mango::algorithms[j].name, &alg);
    CHECK(alg == j);
  }
}
