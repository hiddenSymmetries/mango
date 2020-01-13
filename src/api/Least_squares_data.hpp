#ifndef MANGO_LEAST_SQUARES_DATA_H
#define MANGO_LEAST_SQUARES_DATA_H

#include "mango.hpp"

namespace mango {

  class Least_squares_data {
    // Everything is public because this information must be used by the concrete Package.
  public:
    int N_terms;
    double* targets;
    double* sigmas;
    residual_function_type residual_function;
    double* best_residual_function;
    double* residuals;
    bool print_residuals_in_output_file;

    Least_squares_data(int);
    ~Least_squares_data();
  }
}

#endif
