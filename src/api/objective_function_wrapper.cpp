#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <sstream>
#include "mango.hpp"
#include "Problem_data.hpp"

void mango::Problem_data::objective_function_wrapper(const double* x, double* f, bool* failed) {
  if (verbose > 0) std::cout << "Hello from objective_function_wrapper" << std::endl;

  int failed_int;
  objective_function(&N_parameters, x, f, &failed_int, problem, user_data);
  *failed = (failed_int != 0);

  if (verbose > 0) std::cout << " objective_function_wrapper: *failed=" << *failed << " at_least_one_success=" << at_least_one_success 
			     << ", *f < best_objective_function=" << (*f < best_objective_function) << std::endl;

  record_function_evaluation(x, *f, *failed);
}


bool mango::Problem_data::record_function_evaluation(const double* x, double f, bool failed) {
  if (verbose > 0) std::cout << "Hello from Problem_data::record_function_evaluation" << std::endl;

  function_evaluations++;

  clock_t now = clock();

  bool new_optimum = false;
  if (!failed && (!at_least_one_success || f < best_objective_function)) {
    new_optimum = true;
    at_least_one_success = true;
    best_objective_function = f;
    best_function_evaluation = function_evaluations;
    memcpy(best_state_vector, x, N_parameters * sizeof(double));
    best_time = now;
  }

  recorder->record_function_evaluation(function_evaluations, now, x, f);

  return new_optimum;
}


