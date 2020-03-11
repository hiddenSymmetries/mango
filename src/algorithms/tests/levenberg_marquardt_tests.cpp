#include "catch.hpp"
#include "levenberg_marquardt.hpp"

TEST_CASE("levenberg_marquardt","[algorithm][levenberg_marquardt]") {
  CHECK(mango::levenberg_marquardt::fun1() == 17);
}
