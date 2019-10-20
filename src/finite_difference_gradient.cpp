#include<iostream>
#include<iomanip>
#include<cstring>
#include "mpi.h"
#include "mango.hpp"
#include<cmath>

void mango::problem::finite_difference_gradient(const double* state_vector, double* base_case_objective_function, double* gradient) {

  /* gradient should have been allocated already, with size N_parameters. */

  int data;
  int j_evaluation, j_parameter;

  std::cout << "Hello from finite_difference_gradient from proc " << mpi_rank_world << "\n" << std::flush;

  if (proc0_world) {
    /* Tell the group leaders to start this subroutine  */
    data = 1;
    MPI_Bcast(&data,1,MPI_INT,0,mpi_comm_group_leaders);
  }

  /* Only proc0_world has a meaningful state vector at this point. 
     Copy it to the other processors. */
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
  double* objective_functions = new double[N_evaluations];
  /*  double* state_vectors = new double[N_parameters,N_evaluations]; */
  double* state_vectors = new double[N_parameters * N_evaluations];

  *base_case_objective_function = 0;
  memset(objective_functions, 0, N_evaluations*sizeof(double));

  /*
  std::cout << "Here comes state_vector in 1D when it should be 0: ";
  for (int j=0; j<N_parameters*N_evaluations; j++) {
    std::cout << std::setw(25) << state_vectors[j];
  }
  std::cout << "\n";
  */

  /* Build the set of state vectors that will be considered. */
  for (j_evaluation = 1; j_evaluation <= N_evaluations; j_evaluation++) {
    memcpy(perturbed_state_vector, state_vector_copy, N_parameters*sizeof(double));
    if (j_evaluation == 1) {
      /* This is the base case, so do not perturb the state vector. */
    } else if (j_evaluation <= N_parameters + 1) {
      /* We are doing a forward step */
      perturbed_state_vector[j_evaluation - 2] = perturbed_state_vector[j_evaluation - 2] + finite_difference_step_size;
    } else {
      /* We must be doing a backwards step */
      perturbed_state_vector[j_evaluation - 2 - N_parameters] = perturbed_state_vector[j_evaluation - 2 - N_parameters] - finite_difference_step_size;
    }
    /* std::cout << "perturbed_state_vector[0] for j_evaluation " << j_evaluation << " is " <<std::setprecision(15) << perturbed_state_vector[0] << "\n"; */
    memcpy(&state_vectors[(j_evaluation-1)*N_parameters], perturbed_state_vector, N_parameters*sizeof(double));
  }

  /*
  std::cout << "Here comes state_vector in 1D: ";
  for (int j=0; j<N_parameters*N_evaluations; j++) {
    std::cout << std::setw(25) << state_vectors[j];
  }
  std::cout << "\n";
  */

  if (proc0_world) {
    std::cout << "Here comes state_vectors:\n";
    for(j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      for(j_evaluation=0; j_evaluation<N_evaluations; j_evaluation++) {
	std::cout << std::setw(25) << std::setprecision(15) << state_vectors[j_evaluation*N_parameters + j_parameter];
      }
      std::cout << "\n";
    }
  }

  /* Each proc now evaluates the objective function for its share of the perturbed state vectors. */
  double f;
  int failed_int;
  for(j_evaluation=0; j_evaluation < N_evaluations; j_evaluation++) {
    if ((j_evaluation % N_worker_groups) == mpi_rank_group_leaders) {
      objective_function(&N_parameters, &state_vectors[j_evaluation*N_parameters], &f, &failed_int, this);
      objective_functions[j_evaluation] = f;
    }
  }

  /* Send results back to the world master.
     Make sure not to reduce over MPI_COMM_WORLD, since then the objective function values will be multiplied by # of workers per worker group.
  */
  if (proc0_world) {
    MPI_Reduce(MPI_IN_PLACE, objective_functions, N_evaluations, MPI_DOUBLE, MPI_SUM, 0, mpi_comm_group_leaders);
  } else {
    MPI_Reduce(objective_functions, objective_functions, N_evaluations, MPI_DOUBLE, MPI_SUM, 0, mpi_comm_group_leaders);
  }

  /* Record the results in order in the output file. */
  if (proc0_world) {
    for(j_evaluation=0; j_evaluation<N_evaluations; j_evaluation++) {
      function_evaluations += 1;
      write_file_line(&state_vectors[j_evaluation*N_parameters], objective_functions[j_evaluation]);
    }
  }
  
  /* Finally, evaluate the finite difference derivatives. */
  *base_case_objective_function = objective_functions[0];
  if (centered_differences) {
    for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      gradient[j_parameter] = (objective_functions[j_parameter+1] - objective_functions[j_parameter+1+N_parameters]) / (2 * finite_difference_step_size);
    }
  } else {
    /* 1-sided finite differences */
    for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      gradient[j_parameter] = (objective_functions[j_parameter+1] - *base_case_objective_function) / finite_difference_step_size;
    }
  }

  if (proc0_world) {
    std::cout << "Here comes the finite-difference gradient: ";
    for(j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      std::cout << std::setw(25) << std::setprecision(15) << gradient[j_parameter];
      }
    std::cout << "\n";
  }

  /* Clean up. */
  delete[] perturbed_state_vector;
  delete[] objective_functions;
  delete[] state_vectors;
  delete[] state_vector_copy;
}
