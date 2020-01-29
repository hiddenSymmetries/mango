#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Least_squares_data.hpp"

void mango::Least_squares_data::residual_function_wrapper(const double* x, double* f, bool* failed) {
  // This subroutine is used to call the residual function when N_worker_groups = 1 (i.e. parallelization only within the objective function evaluation),
  // or for algorithms like hopspack that allow concurrent evaluations of the objective function but not using finite difference Jacobians.
  // This subroutine is not used for finite difference Jacobians.

  // I chose to make this subroutine take a parameter f[] to store the residuals rather than always storing them
  // in the "residuals" array of the mango::problem class because PETSc uses its own storage for the residuals.

  // For non-least-squares algorithms, 

  int failed_int;
  residual_function(&(problem_data->N_parameters), x, &N_terms, f, &failed_int, problem_data->problem, original_user_data);
  *failed = (failed_int != 0);

  if (problem_data->verbose > 0) {
    std::cout << "Hello from residual_function_wrapper. Here comes x:" << std::endl;
    int j;
    for (j=0; j < problem_data->N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x[j];
    }
    std::cout << std::endl;
  }

  record_function_evaluation(x, f, *failed);

  /*
  //double objective_value = residuals_to_single_objective(f);


  if (algorithms[problem_data->algorithm].least_squares) {
    // For non-least-squares algorithms, the steps below are handled by objective_function_wrapper.
    problem_data->function_evaluations++;
    current_residuals = f;
    clock_t now = clock();
    problem_data->recorder->record_function_evaluation(problem_data->function_evaluations, now, x, objective_value);
  }

  // I may want to change the logic in the next line at some point. The idea is that only proc 0 should write to the output file.
  // This subroutine is only called on proc 0 except for hopspack, where this subroutine is called by all group leaders.
  // Hence, for hopspack, we should not write to the file here.
  if (algorithms[problem_data->algorithm].package != PACKAGE_HOPSPACK) {
    write_least_squares_file_line(now, x, objective_value, f);
  }

  if (! *failed && (!problem_data->at_least_one_success || objective_value < problem_data->best_objective_function)) {
    problem_data->at_least_one_success = true;
    problem_data->best_objective_function = objective_value;
    problem_data->best_function_evaluation = problem_data->function_evaluations;
    memcpy(problem_data->best_state_vector, x, problem_data->N_parameters * sizeof(double));
    memcpy(best_residual_function, f, N_terms * sizeof(double));
    problem_data->best_time = now;
  }
  */
}

void mango::Least_squares_data::record_function_evaluation(const double* x, double* f, bool failed) {

  // For non-least-squares algorithms, the function evaluations are recorded in objective_function_wrapper.
  // Otherwise, do the recording here.
  bool new_optimum;
  if (algorithms[problem_data->algorithm].least_squares) {
    double objective_value = residuals_to_single_objective(f);
    current_residuals = f;
    new_optimum = problem_data->record_function_evaluation(x, objective_value, failed);
    if (new_optimum) {
      memcpy(best_residual_function, f, N_terms * sizeof(double));
    }
  }
}
