#include<iostream>
#include<iomanip>
#include<cstring>
#include "mango.hpp"

void mango::problem::objective_function_wrapper(const double* x, double* f, bool* failed) {
  /* For least-squares problems, function_evaluations is incremented in mango_residual_function_wrapper() */
  if (!least_squares) function_evaluations++;

  int failed_int;
  objective_function(&N_parameters, x, f, &failed_int, this);
  *failed = (failed_int != 0);

  if (verbose > 0) std::cout << " objective_function_wrapper: *failed=" << *failed << " at_least_one_success=" << at_least_one_success << ", *f < best_objective_function=" << (*f < best_objective_function) << "\n";
  if (! *failed && (!at_least_one_success || *f < best_objective_function)) {
    at_least_one_success = true;
    best_objective_function = *f;
    best_function_evaluation = function_evaluations;
    memcpy(best_state_vector, x, N_parameters * sizeof(double));
  } else {
  }

  if (!least_squares) {
    /* For least-squares problems, output is written using mango_residual_function_wrapper() */
    write_file_line(x, *f);
  }
}



void mango::problem::write_file_line(const double* x, double f) {
  output_file << std::setw(6) << std::right << function_evaluations;
  for (int j=0; j<N_parameters; j++) {
    output_file << "," << std::setw(26) << std::setprecision(16) << std::scientific << x[j];
  }
  output_file << "," << std::setw(26) << f << "\n" << std::flush;
}
