#include "catch.hpp"
#include "levenberg_marquardt.hpp"


TEST_CASE("levenberg_marquardt","[algorithm][levenberg_marquardt]") {
  // If N_line_search==1, the normalized lambda grid should be [1.0] for any lambda_step
  int N_line_search = 1;
  auto log_step = GENERATE(range(0,5)); // so log_step = 0, 1, 2, 3, or 4.
  double lambda_step = exp(log_step - 2.0);
  double lambda_grid;
  mango::levenberg_marquardt::compute_lambda_grid(N_line_search, lambda_step, &lambda_grid);
  CHECK(lambda_grid == Approx(1.0).epsilon(1e-13));
}

