#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <ctime>
#include "mpi.h"
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Least_squares_data.hpp"

void mango::Least_squares_data::finite_difference_Jacobian(const double* state_vector, double* base_case_residual_function, double* Jacobian) {

  // base_case_residual_function should have been allocated already, with size N_terms.
  // Jacobian should have been allocated already, with size N_parameters * N_terms.

  // To simplify code in this file, make some copies of variables.
  MPI_Comm mpi_comm_group_leaders = problem_data->mpi_partition->get_comm_group_leaders();
  bool proc0_world = problem_data->mpi_partition->get_proc0_world();
  int mpi_rank_world = problem_data->mpi_partition->get_rank_world();
  int mpi_rank_group_leaders = problem_data->mpi_partition->get_rank_group_leaders();
  int N_worker_groups = problem_data->mpi_partition->get_N_worker_groups();
  int N_parameters = problem_data->N_parameters;
  int verbose = problem_data->verbose;

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
  if (problem_data->centered_differences) {
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
      perturbed_state_vector[j_evaluation - 2] = perturbed_state_vector[j_evaluation - 2] + problem_data->finite_difference_step_size;
    } else {
      // We must be doing a backwards step
      perturbed_state_vector[j_evaluation - 2 - N_parameters] = perturbed_state_vector[j_evaluation - 2 - N_parameters] - problem_data->finite_difference_step_size;
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
  int failed_int;
  for(j_evaluation=0; j_evaluation < N_evaluations; j_evaluation++) {
    if ((j_evaluation % N_worker_groups) == mpi_rank_group_leaders) {
      // Note that the use of &residual_functions[j_evaluation*N_terms] in the next line means that j_terms must be the least-signficiant dimension in residual_functions.
      residual_function(&N_parameters, &state_vectors[j_evaluation*N_parameters], &N_terms, &residual_functions[j_evaluation*N_terms], &failed_int, problem_data->problem, original_user_data);
    }
  }

  // Send results back to the world master.
  // Make sure not to reduce over MPI_COMM_WORLD, since then the residual function values will be multiplied by # of workers per worker group.
  if (proc0_world) {
    MPI_Reduce(MPI_IN_PLACE, residual_functions, N_evaluations * N_terms, MPI_DOUBLE, MPI_SUM, 0, mpi_comm_group_leaders);
  } else {
    MPI_Reduce(residual_functions, residual_functions, N_evaluations * N_terms, MPI_DOUBLE, MPI_SUM, 0, mpi_comm_group_leaders);
  }

  // Record the results in order in the output file. At the same time, check for any best-yet values of the
  // objective function.
  double total_objective_function;
  bool failed;
  clock_t now;
  if (proc0_world) {
    for(j_evaluation=0; j_evaluation<N_evaluations; j_evaluation++) {
      problem_data->function_evaluations += 1;
      now = clock();
      total_objective_function = residuals_to_single_objective(&residual_functions[j_evaluation*N_terms]);
      write_least_squares_file_line(now, &state_vectors[j_evaluation*N_parameters], total_objective_function, &residual_functions[j_evaluation*N_terms]);

      failed = false;
      if (!failed && (!problem_data->at_least_one_success || total_objective_function < problem_data->best_objective_function)) {
        problem_data->at_least_one_success = true;
        problem_data->best_objective_function = total_objective_function;
        problem_data->best_function_evaluation = problem_data->function_evaluations;
        memcpy(problem_data->best_state_vector, &state_vectors[j_evaluation*N_parameters], N_parameters * sizeof(double));
        memcpy(best_residual_function, &residual_functions[j_evaluation*N_terms], N_terms * sizeof(double));
	problem_data->best_time = now;
      }
    }
  }
  
  // Finally, evaluate the finite difference derivatives.
  memcpy(base_case_residual_function, residual_functions, N_terms*sizeof(double));
  if (problem_data->centered_differences) {
    for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      for (int j_term=0; j_term<N_terms; j_term++) {
	Jacobian[j_parameter*N_terms+j_term] = (residual_functions[(j_parameter+1)*N_terms+j_term] - residual_functions[(j_parameter+1+N_parameters)*N_terms+j_term])
	  / (2 * problem_data->finite_difference_step_size);
      }
    }
  } else {
    // 1-sided finite differences
    for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      for (int j_term=0; j_term<N_terms; j_term++) {
	Jacobian[j_parameter*N_terms+j_term] = (residual_functions[(j_parameter+1)*N_terms+j_term] - base_case_residual_function[j_term]) / problem_data->finite_difference_step_size;
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
