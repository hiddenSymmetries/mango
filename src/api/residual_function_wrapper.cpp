// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include "mango.hpp"
#include "Least_squares_solver.hpp"

void mango::Least_squares_solver::residual_function_wrapper(const double* x, double* f, bool* failed) {
  // This subroutine is used to call the residual function when N_worker_groups = 1 (i.e. parallelization only within the objective function evaluation),
  // or for algorithms like hopspack that allow concurrent evaluations of the objective function but not using finite difference Jacobians.
  // This subroutine is not used for finite difference Jacobians.

  // I chose to make this subroutine take a parameter f[] to store the residuals rather than always storing them
  // in the "residuals" array of the mango::problem class because PETSc uses its own storage for the residuals.

  // For non-least-squares algorithms, 

  int failed_int;
  residual_function(&(N_parameters), x, &N_terms, f, &failed_int, problem, user_data);
  *failed = (failed_int != 0);

  if (verbose > 0) {
    std::cout << "Hello from residual_function_wrapper. Here comes x:" << std::endl;
    int j;
    for (j=0; j < N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x[j];
    }
    std::cout << std::endl;
  }

  record_function_evaluation(x, f, *failed);

}

void mango::Least_squares_solver::record_function_evaluation(const double* x, double *f, bool failed) {
  // This method is NOT an override: note double*f instead of double f, and return void instead of bool.
  if (verbose>0) std::cout << "Hello from Least_squares_solver::record_function_evaluation, the non-override." << std::endl;
  double objective_value = residuals_to_single_objective(f);
  current_residuals = f;
  record_function_evaluation(x, objective_value, failed);
}

bool mango::Least_squares_solver::record_function_evaluation(const double* x, double f, bool failed) {
  // This method overrides mango::Solver::record_function_evaluation()

  if (verbose>0) std::cout << "Hello from Least_squares_solver::record_function_evaluation, the override." << std::endl;
  // Call the overridden function from the base class:
  bool new_optimum = mango::Solver::record_function_evaluation(x,f,failed);
  if (new_optimum) {
    memcpy(best_residual_function, current_residuals, N_terms * sizeof(double));
  }
  
  return new_optimum;
  /*

  // For non-least-squares algorithms, the function evaluations are recorded in objective_function_wrapper.
  // Otherwise, do the recording here.
  bool new_optimum;
  if (algorithms[algorithm].least_squares) {
    double objective_value = residuals_to_single_objective(f);
    current_residuals = f;
    new_optimum = record_function_evaluation(x, objective_value, failed);
    if (new_optimum) {
      memcpy(best_residual_function, f, N_terms * sizeof(double));
    }
  }

  */
}


void mango::Least_squares_solver::objective_function_wrapper(const double* x, double* f, bool* failed) {
  // This method overrides mango::Solver::objective_function_wrapper().
  // The difference from that method is that here we do not call record_function_evaluation,
  // since it was done already in residual_function_wrapper().
  if (verbose > 0) std::cout << "Hello from Least_squares_solver::objective_function_wrapper" << std::endl;

  int failed_int;
  objective_function(&N_parameters, x, f, &failed_int, problem, user_data);
  *failed = (failed_int != 0);
}
