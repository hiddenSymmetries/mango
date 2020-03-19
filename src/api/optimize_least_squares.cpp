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
#include <cstring>
#include <stdexcept>
#include <limits>
#include "mango.hpp"
#include "Least_squares_solver.hpp"

double mango::Least_squares_solver::optimize(MPI_Partition* mpi_partition_in) {

  mpi_partition = mpi_partition_in;
  init_optimization();

  int j;
  bool proc0_world = mpi_partition->get_proc0_world();

  if (verbose > 0) std::cout << "Hello world from Least_squares_solver::optimize()" << std::endl;

  // I need to think about this next bit. Do all group leaders need a copy of sigmas and targets?
  // For hopspack, they do, because the residuals are combined into the total objective function on each group leader.
  // But for finite difference Jacobians, only proc0 needs sigmas and targets.
  // If they do, then they must all agree on N_terms; otherwise probably the size of targets and sigmas
  // will be incorrect on non-master procs.
  // Make sure that all procs agree on sigmas and targets.
  MPI_Bcast(targets, N_terms, MPI_DOUBLE, 0, mpi_partition->get_comm_group_leaders());
  MPI_Bcast(sigmas,  N_terms, MPI_DOUBLE, 0, mpi_partition->get_comm_group_leaders());

  if (algorithms[algorithm].uses_derivatives && !proc0_world && algorithms[algorithm].package != PACKAGE_MANGO) {
    // In line above, we include algorithms[algorithm].package != PACKAGE_MANGO
    // because MANGO's own algorithms may need a parallel line search in addition to parallel gradients.
    // Therefore MANGO's own algorithms are responsible for launching their own group_leaders_loop.
    // For a more general solution, I might want to consider adding an algorithm property like "parallel_only_in_gradient".
    group_leaders_loop();
    return std::numeric_limits<double>::quiet_NaN();
  }

  // proc0_world always continues past this point.
  // For finite-difference-derivative algorithms, the other procs do not go past this point.
  // For parallel algorithms that do not use finite-difference derivatives, such as HOPSPACK, the other group leader procs DO continue past this point.

  function_evaluations = 0;

  // Verify that the sigmas array is all nonzero.
  for (j=0; j<N_terms; j++) {
    if (sigmas[j] == 0.0) {
      std::cerr << "Error! The (0-based) entry " << j << " in the sigmas array is 0. sigmas must all be nonzero." << std::endl;
      throw std::runtime_error("Error in mango::problem::optimize_least_squares. sigmas is not all nonzero.");
    }
  }

  if (proc0_world) {

    /*
    // Open output file
    output_file.open(output_filename.c_str());
    if (!output_file.is_open()) {
      std::cout << "output file: " << output_filename << std::endl;
      throw std::runtime_error("Error in mango::problem::optimize_least_squares(). Unable to open output file.");
    }
    // Write header line of output file
    output_file << "Least squares?" << std::endl << "yes" << std::endl << "N_parameters:" << std::endl << N_parameters << std::endl << "function_evaluation,seconds";
    for (j=0; j<N_parameters; j++) {
      output_file << ",x(" << j+1 << ")";
    }
    output_file << ",objective_function";
    if (print_residuals_in_output_file) {
      for (j=0; j<N_terms; j++) {
	output_file << ",F(" << j+1 << ")";
      }
    }
    output_file << std::endl << std::flush;
    */

    /* Sanity test */
    /* if (!least_squares_algorithm) {
       std::cout << "Error in optimize_least_squares(). Should not get here!\n";
       exit(1);
       }*/
    
    /*  objective_function = (mango::objective_function_type) &mango::problem::least_squares_to_single_objective; */
  } // if (proc0_world)

  // 20200129 This next line was moved to the constructor
  //objective_function = &mango::Least_squares_solver::least_squares_to_single_objective;

  // This next bit is useful for hopspack.
  memset(best_residual_function, 0, N_terms*sizeof(double));
  current_residuals = best_residual_function;

  // Perform the main optimization.
  if (algorithms[algorithm].least_squares) {
    package->optimize_least_squares(this);
  } else {
    // Non-least-squares algorithms
    package->optimize(this);
  }

  if (!proc0_world) return std::numeric_limits<double>::quiet_NaN();
  // Only proc0_world continues past this point.

  // Tell the other group leaders to exit.
  int data = -1;
  MPI_Bcast(&data,1,MPI_INT,0,mpi_partition->get_comm_group_leaders());

  memcpy(state_vector, best_state_vector, N_parameters * sizeof(double)); // Make sure we leave state_vector equal to the best state vector seen.

  recorder->finalize();
  /*
  // Copy the line corresponding to the optimum to the bottom of the output file.
  int function_evaluations_temp= function_evaluations;
  function_evaluations = best_function_evaluation;
  write_least_squares_file_line(best_time, state_vector, best_objective_function, best_residual_function);
  function_evaluations = function_evaluations_temp;

  output_file.close();
  */

  if (verbose > 0) {
    std::cout << "Here comes the optimal state_vector from optimize_least_squares.cpp: " << state_vector[0];
    for (int j=1; j<N_parameters; j++) {
      std::cout << ", " << state_vector[j];
    }
    std::cout << std::endl;
  }

  return best_objective_function;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void mango::Least_squares_solver::least_squares_to_single_objective(int* N, const double* x, double* f, int* failed_int, mango::Problem* this_problem, void* user_data) {
  // Note that this function is static, so "this" does not exist.
  // We therefore need a pointer to the Least_squares_solver.
  // Since this function is being called, this_problem->solver must be a Least_squares_solver, so we can cast the pointer:
  Least_squares_solver* least_squares_solver = (Least_squares_solver*)(this_problem->get_solver());

  // Note that this subroutine sets the 'residuals' array of the mango::Least_squares_solver class.

  if (least_squares_solver->verbose > 0) std::cout << "Hello from least_squares_to_single_objective" << std::endl;
  //double* residuals = new double[N_terms];

  bool failed_bool;
  least_squares_solver->residual_function_wrapper(x, least_squares_solver->residuals, &failed_bool);
  if (failed_bool) {
    *failed_int = 1; 
  } else {
    *failed_int = 0;
  }

  int N_terms = least_squares_solver->N_terms;
  double term;
  *f = 0;
  for (int j=0; j<N_terms; j++) {
    term = (least_squares_solver->residuals[j] - least_squares_solver->targets[j]) / least_squares_solver->sigmas[j];
    *f += term*term;
  }

  least_squares_solver->current_residuals = least_squares_solver->residuals;
  //delete[] residuals;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

double mango::Least_squares_solver::residuals_to_single_objective(double* residuals_in) {
  // Combine the residuals into the total objective function.
  double total_objective_function, temp;
  int j;

  total_objective_function = 0;
  for (j=0; j<N_terms; j++) {
    temp = (residuals_in[j] - targets[j]) / sigmas[j];
    total_objective_function += temp*temp;
  }
  return(total_objective_function);
}


