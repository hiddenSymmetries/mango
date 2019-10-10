#include<iostream>
#include<iomanip>
#include "mango.hpp"

void mango::problem::objective_function_wrapper(const double* x, double* f, bool* failed) {
  function_evaluations++;
  int failed_int;
  objective_function(&N_parameters, x, f, &failed_int);
  *failed = (failed_int != 0);

  if (!least_squares) {
    /* For least-squares problems, output is written using mango_residual_function_wrapper() */
    output_file << std::setw(6) << std::right << function_evaluations;
    for (int j=0; j<N_parameters; j++) {
      output_file << "," << std::setw(26) << std::setprecision(16) << std::scientific << x[j];
    }
    output_file << "," << std::setw(26) << *f << "\n" << std::flush;
  }
}
