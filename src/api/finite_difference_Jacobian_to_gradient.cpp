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
#include <stdexcept>
#include "mango.hpp"
#include "Least_squares_solver.hpp"

void mango::Least_squares_solver::finite_difference_gradient(const double* state_vector, double* base_case_objective_function, double* gradient) {
  // This subroutine overrides mango::Solver::finite_difference_gradient.
  // gradient should have been allocated already, with size N_parameters.

  if (verbose > 0) std::cout << "Hello from finite_difference_Jacobian_to_gradient from proc " << mpi_partition->get_rank_world() << std::endl;
  if (!mpi_partition->get_proc0_world()) throw std::runtime_error("Only proc0_world should get here!");

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
