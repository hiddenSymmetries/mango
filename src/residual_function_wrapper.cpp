#include<iostream>
#include<iomanip>
#include<cstring>
#include<ctime>
#include "mango.hpp"

void mango::problem::residual_function_wrapper(const double* x, double* f, bool* failed) {
  // This subroutine is used to call the residual function when N_worker_groups = 1 (i.e. parallelization only within the objective function evaluation),
  // or for algorithms like hopspack that allow concurrent evaluations of the objective function but not using finite difference Jacobians.
  // This subroutine is not used for finite difference Jacobians.

  // I chose to make this subroutine take a parameter f[] to store the residuals rather than always storing them
  // in the "residuals" array of the mango::problem class because PETSc uses its own storage for the residuals.

  function_evaluations++;

  int failed_int;
  residual_function(&N_parameters, x, &N_terms, f, &failed_int, this);
  *failed = (failed_int != 0);
  clock_t now = clock();

  if (verbose > 0) {
    std::cout << "Hello from residual_function_wrapper. Here comes x:\n";
    int j;
    for (j=0; j < N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x[j];
    }
    std::cout << "\n";
  }

  double objective_value = residuals_to_single_objective(f);

  // I may want to change the logic in the next line at some point. The idea is that only proc 0 should write to the output file.
  // This subroutine is only called on proc 0 except for hopspack, where this subroutine is called by all group leaders.
  // Hence, for hopspack, we should not write to the file here.
  if (!algorithms[algorithm].parallel) {
    write_least_squares_file_line(now, x, objective_value, f);
  }

  if (! *failed && (!at_least_one_success || objective_value < best_objective_function)) {
    at_least_one_success = true;
    best_objective_function = objective_value;
    best_function_evaluation = function_evaluations;
    memcpy(best_state_vector, x, N_parameters * sizeof(double));
    memcpy(best_residual_function, f, N_terms * sizeof(double));
    best_time = now;
  }

}

