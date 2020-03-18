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
#include <string>
#include <stdexcept>
#include "mango.hpp"
#include "Solver.hpp"
#include "Least_squares_solver.hpp"

// Constructor for least-squares problems
mango::Least_squares_problem::Least_squares_problem(int N_parameters_in, double* state_vector_in, int N_terms_in, double* targets_in, double* sigmas_in, 
			double* best_residual_function_in, vector_function_type residual_function_in, int argc_in, char* argv_in[]) 
  : Problem(N_parameters_in, state_vector_in, NULL, argc_in, argv_in) // Call constructor of base class.
{
  // Replace Solver with a Least_squares_solver
  delete solver;
  least_squares_solver = new Least_squares_solver(this,N_parameters_in, N_terms_in);
  solver = least_squares_solver;
  // Note that least_squares_solver and solver point to the same object.

  solver->argc = argc_in;
  solver->argv = argv_in;
  solver->N_parameters = N_parameters_in;
  //solver->objective_function = NULL; // Should this be least_squares_to_single_objective?
  solver->state_vector = state_vector_in;

  /*
  least_squares_data = new Least_squares_data(solver, N_terms_in);// I'm not sure "this" is correct? Does it need to be a Problem* instead of a Least_squares_problem* ? 
  */
  least_squares_solver->N_terms = N_terms_in;
  least_squares_solver->targets = targets_in;
  least_squares_solver->sigmas = sigmas_in;
  least_squares_solver->residual_function = residual_function_in;
  least_squares_solver->best_residual_function = best_residual_function_in;
  
}

// Destructor
mango::Least_squares_problem::~Least_squares_problem() {
  if (solver->verbose > 0) std::cout << "Mango least squares problem is being destroyed." << std::endl;
  //delete data; // This is handled by the destructor of the parent class (Problem).
  //delete solver; // This is handled by the destructor of the parent class (Problem).
}

void mango::Least_squares_problem::set_print_residuals_in_output_file(bool new_bool) {
  least_squares_solver->print_residuals_in_output_file = new_bool;
}

int mango::Least_squares_problem::get_N_terms() {
  return least_squares_solver->N_terms;
}

/*
double mango::Least_squares_problem::optimize() {
  // Delegate this work to Least_squares_data so we don't need to put "least_squares_data->" in front of everything.
  return least_squares_data->optimize(&mpi_partition);
}
*/
