#include<iostream>
#include<iomanip>
#include<cstring>
#include<stdexcept>
#include "mango.hpp"

void mango::problem::finite_difference_Jacobian_to_gradient(const double* state_vector, double* base_case_objective_function, double* gradient) {

  // gradient should have been allocated already, with size N_parameters.

  if (verbose > 0) std::cout << "Hello from finite_difference_Jacobian_to_gradient from proc " << mpi_partition.get_rank_world() << std::endl;

  if (!mpi_partition.get_proc0_world()) throw std::runtime_error("Only proc0_world should get here!");

  double* base_case_residual_vector = new double[N_terms];
  double* Jacobian = new double[N_terms * N_parameters];

  finite_difference_Jacobian(state_vector, base_case_residual_vector, Jacobian);

  int j_parameter, j_term;
  double term;
  *base_case_objective_function = 0;
  memset(gradient, 0, N_parameters*sizeof(double));
  for (j_term=0; j_term<N_terms; j_term++) {
    term = (base_case_residual_vector[j_term] - targets[j_term]) / sigmas[j_term];
    *base_case_objective_function += term*term;
    for (j_parameter=0; j_parameter<N_parameters; j_parameter++) {
      gradient[j_parameter] += 2 * (term / sigmas[j_term]) * Jacobian[j_parameter*N_terms + j_term];
    }
  }

  delete[] base_case_residual_vector;
  delete[] Jacobian;

}
