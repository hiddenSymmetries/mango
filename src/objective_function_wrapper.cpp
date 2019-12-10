#include<iostream>
#include<iomanip>
#include<cstring>
#include<ctime>
#include<sstream>
#include "mango.hpp"

void mango::problem::objective_function_wrapper(const double* x, double* f, bool* failed) {
  if (verbose > 0) std::cout << "Hello from objective_funciton_wrapper\n";

  /* For least-squares problems, function_evaluations is incremented in mango_residual_function_wrapper() */
  if (!least_squares) function_evaluations++;
  clock_t now = clock();

  int failed_int;
  objective_function(&N_parameters, x, f, &failed_int, this);
  *failed = (failed_int != 0);

  if (verbose > 0) std::cout << " objective_function_wrapper: *failed=" << *failed << " at_least_one_success=" << at_least_one_success << ", *f < best_objective_function=" << (*f < best_objective_function) << "\n";
  if (! *failed && (!at_least_one_success || *f < best_objective_function)) {
    at_least_one_success = true;
    best_objective_function = *f;
    best_function_evaluation = function_evaluations;
    memcpy(best_state_vector, x, N_parameters * sizeof(double));
    best_time = now;
  } else {
  }

  if (!least_squares) {
    /* For least-squares problems, output is written using mango_residual_function_wrapper() */
    write_file_line(x, *f, now);
  }
}



void mango::problem::write_file_line(const double* x, double f, clock_t print_time) {
  double elapsed_time = ((float)(print_time - start_time)) / CLOCKS_PER_SEC;

  output_file << std::setw(6) << std::right << function_evaluations << "," << std::setw(12) << std::setprecision(4) << std::scientific << elapsed_time;
  for (int j=0; j<N_parameters; j++) {
    output_file << "," << std::setw(24) << std::setprecision(16) << std::scientific << x[j];
  }
  output_file << "," << std::setw(24) << f << "\n" << std::flush;
}


void mango::problem::compose_hopspack_file_line(const double* x, const double f, std::string & file_string) {
  clock_t print_time = clock();
  double elapsed_time = ((float)(print_time - start_time)) / CLOCKS_PER_SEC;

  std::ostringstream string_stream;
  //string_stream << std::setw(6) << std::right << function_evaluations << "," << std::setw(12) << std::setprecision(4) << std::scientific << elapsed_time;
  string_stream << std::right << std::setw(12) << std::setprecision(4) << std::scientific << elapsed_time;
  for (int j=0; j<N_parameters; j++) {
    string_stream << "," << std::setw(24) << std::setprecision(16) << std::scientific << x[j];
  }
  string_stream << "," << std::setw(24) << f;
  // Finally, convert the stream to a std::string:
  file_string = string_stream.str();
}
