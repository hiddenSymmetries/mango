#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <sstream>
#include "mango.hpp"
#include "Problem_data.hpp"

void mango::Problem_data::objective_function_wrapper(const double* x, double* f, bool* failed) {
  if (verbose > 0) std::cout << "Hello from objective_function_wrapper" << std::endl;

  // For least-squares problems, function_evaluations is incremented in mango_residual_function_wrapper()
  // 20200127 Not sure what to do here after the big refactor, since least_squares no longer exists.
  //if (!least_squares) function_evaluations++;
  function_evaluations++;
  
clock_t now = clock();

  int failed_int;
  objective_function(&N_parameters, x, f, &failed_int, mpi_partition, user_data);
  *failed = (failed_int != 0);

  if (verbose > 0) std::cout << " objective_function_wrapper: *failed=" << *failed << " at_least_one_success=" << at_least_one_success 
			     << ", *f < best_objective_function=" << (*f < best_objective_function) << std::endl;
  if (! *failed && (!at_least_one_success || *f < best_objective_function)) {
    at_least_one_success = true;
    best_objective_function = *f;
    best_function_evaluation = function_evaluations;
    memcpy(best_state_vector, x, N_parameters * sizeof(double));
    best_time = now;
  } else {
  }

  /* Not sure what to do about this next bit after the big refactor. 20200127.
  if ((!least_squares) && (algorithms[algorithm].package != PACKAGE_HOPSPACK)) {
    // For parallel gradient-based problems, output is ususally written using finite_difference_gradient() or finite_difference_Jacobian(),
    // but may be written here during the line search.
    // For non-gradient-based problems with concurrent function evaluations (e.g. hopspack), output is written elsewhere.
    // For least-squares problems with no concurrent function evaluations, output is written using residual_function_wrapper().
    // In the remaining case, we write the output here:
    write_file_line(now, x, *f);
  }
  */
}


