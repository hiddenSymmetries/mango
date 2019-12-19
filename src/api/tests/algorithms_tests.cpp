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
