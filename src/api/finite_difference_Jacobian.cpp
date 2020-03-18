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
#include <cmath>
#include <ctime>
#include "mpi.h"
#include "mango.hpp"
#include "Solver.hpp"

void mango::Solver::finite_difference_Jacobian(vector_function_type vector_function, int N_terms, const double* state_vector, double* base_case_residual_function, double* Jacobian) {

  // base_case_residual_function should have been allocated already, with size N_terms.
  // Jacobian should have been allocated already, with size N_parameters * N_terms.

  // To simplify code in this file, make some copies of variables.
  MPI_Comm mpi_comm_group_leaders = mpi_partition->get_comm_group_leaders();
  bool proc0_world = mpi_partition->get_proc0_world();
  int mpi_rank_world = mpi_partition->get_rank_world();

  int data;
  int j_evaluation, j_parameter;

  if (verbose > 0) std::cout << "Hello from finite_difference_Jacobian from proc " << mpi_rank_world << std::endl;

  if (proc0_world) {
    // Tell the group leaders to start this subroutine 
    data = 1;
    MPI_Bcast(&data,1,MPI_INT,0,mpi_comm_group_leaders);
  }

  // Only proc0_world has a meaningful state vector at this point.
  // Copy it to the other processors. In the process we make a copy of the state vector,
  // so the original state vector can be const.
  double* state_vector_copy = new double[N_parameters];
  if (proc0_world) memcpy(state_vector_copy, state_vector, N_parameters*sizeof(double));
  MPI_Bcast(state_vector_copy, N_parameters, MPI_DOUBLE, 0, mpi_comm_group_leaders);

  int N_evaluations;
  if (centered_differences) {
    N_evaluations = N_parameters * 2+ 1;
  } else {
    N_evaluations = N_parameters + 1;
  }

  double* perturbed_state_vector = new double[N_parameters];
  double* residual_functions = new double[N_terms * N_evaluations];
  double* state_vectors = new double[N_parameters * N_evaluations];

  memset(base_case_residual_function, 0, N_terms*sizeof(double));
  memset(residual_functions, 0, N_terms * N_evaluations*sizeof(double));

  // Build the set of state vectors that will be considered.
  for (j_evaluation = 1; j_evaluation <= N_evaluations; j_evaluation++) {
    memcpy(perturbed_state_vector, state_vector_copy, N_parameters*sizeof(double));
    if (j_evaluation == 1) {
      // This is the base case, so do not perturb the state vector.
    } else if (j_evaluation <= N_parameters + 1) {
      // We are doing a forward step
      perturbed_state_vector[j_evaluation - 2] = perturbed_state_vector[j_evaluation - 2] + finite_difference_step_size;
    } else {
      // We must be doing a backwards step
      perturbed_state_vector[j_evaluation - 2 - N_parameters] = perturbed_state_vector[j_evaluation - 2 - N_parameters] - finite_difference_step_size;
    }
    memcpy(&state_vectors[(j_evaluation-1)*N_parameters], perturbed_state_vector, N_parameters*sizeof(double));
  }

  if (proc0_world && (verbose > 0)) {
    std::cout << "Here comes state_vectors:" << std::endl;
    for(j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      for(j_evaluation=0; j_evaluation<N_evaluations; j_evaluation++) {
	std::cout << std::setw(25) << std::setprecision(15) << state_vectors[j_evaluation*N_parameters + j_parameter];
      }
      std::cout << std::endl;
    }
  }

  // Each proc now evaluates the residual function for its share of the perturbed state vectors.
  bool* failures = new bool[N_evaluations];
  evaluate_set_in_parallel(vector_function, N_terms, N_evaluations, state_vectors, residual_functions, failures);
  delete failures; // Eventually do something smarter with the failure data.

  
  // Finally, evaluate the finite difference derivatives.
  memcpy(base_case_residual_function, residual_functions, N_terms*sizeof(double));
  if (centered_differences) {
    for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      for (int j_term=0; j_term<N_terms; j_term++) {
	Jacobian[j_parameter*N_terms+j_term] = (residual_functions[(j_parameter+1)*N_terms+j_term] - residual_functions[(j_parameter+1+N_parameters)*N_terms+j_term])
	  / (2 * finite_difference_step_size);
      }
    }
  } else {
    // 1-sided finite differences
    for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      for (int j_term=0; j_term<N_terms; j_term++) {
	Jacobian[j_parameter*N_terms+j_term] = (residual_functions[(j_parameter+1)*N_terms+j_term] - base_case_residual_function[j_term]) / finite_difference_step_size;
      }
    }
  }

  // Clean up.
  delete[] perturbed_state_vector;
  delete[] residual_functions;
  delete[] state_vectors;
  delete[] state_vector_copy;

  if (proc0_world && (verbose > 0)) {
    std::cout << "Here comes finite-difference Jacobian:" << std::endl;
    for (int j_term=0; j_term<N_terms; j_term++) {
      for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
	std::cout << std::setw(25) << std::setprecision(15) << Jacobian[j_parameter*N_terms+j_term];
      }
      std::cout << std::endl;
    }
  }

}
