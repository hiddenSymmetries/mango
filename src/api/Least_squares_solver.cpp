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

#include <stdexcept>
#include "mango.hpp"
#include "Least_squares_solver.hpp"
#include "Recorder_least_squares.hpp"

// Constructor
mango::Least_squares_solver::Least_squares_solver(Least_squares_problem* problem_in, int N_parameters_in, int N_terms_in) 
: Solver(problem_in, N_parameters_in) // Call constructor of the base class
{
  if (N_terms_in < 1) throw std::runtime_error("Error in mango::Least_squares_solver::Least_squares_solver(). N_terms must be at least 1.");
  N_terms = N_terms_in;

  // Defaults specific to least_squares problems are set in the following lines
  targets = NULL;
  sigmas = NULL;
  residual_function = NULL;
  best_residual_function = NULL;
  residuals = new double[N_terms_in];
  print_residuals_in_output_file = true;
  objective_function = &least_squares_to_single_objective;

  recorder = new Recorder_least_squares(this);
}

// Constructor with no arguments, used only for unit tests.
mango::Least_squares_solver::Least_squares_solver()
  : Solver() // Call constructor of base class
{
}

// Destructor
mango::Least_squares_solver::~Least_squares_solver() {
  delete[] residuals;
}

void mango::Least_squares_solver::finite_difference_Jacobian(const double* state_vector_arg, double* base_case_residual, double* Jacobian) {
  // Call Solver::finite_difference_Jacobian
  mango::Solver::finite_difference_Jacobian(residual_function, N_terms, state_vector_arg, base_case_residual, Jacobian);
}
