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
#include <stdexcept>
#include <cassert>
#include "mango.hpp"
#include "Solver.hpp"
#include "Recorder_standard.hpp"

// Constructor
mango::Solver::Solver(Problem* problem_in, int N_parameters_in) {
  if (N_parameters_in < 1) throw std::runtime_error("Error in mango::Solver::Solver(). N_parameters must be at least 1.");
  N_parameters = N_parameters_in;
  best_state_vector = new double[N_parameters_in];

  // Defaults are set in the following lines:
  verbose = 0;
  objective_function = NULL;
  function_evaluations = 0;
  argc = 1;
  argv = NULL;
  state_vector = NULL;
  algorithm = (algorithm_type)0;
  centered_differences = false;
  finite_difference_step_size = 1.0e-7;
  output_filename = "mango_out";
  max_function_evaluations = 10000;
  best_function_evaluation = -1;
  bound_constraints_set = false;
  package = NULL;
  user_data = NULL;
  problem = problem_in;
  recorder = new Recorder_standard(this);
  N_line_search = 0;
}

// Constructor with no arguments, used only for unit tests
mango::Solver::Solver() {
  //  N_parameters = 1;
  //best_state_vector = new double[1];
  recorder = new Recorder();

  // We need a Problem to exist that is connected to this Solver, so create one.
  problem = new Problem(1,NULL,NULL,1,NULL);
  delete problem->solver;
  problem->solver = this;
}


// Destructor
mango::Solver::~Solver() {
  delete[] best_state_vector;
}

void mango::Solver::objective_to_vector_function(int* N_parameters_arg, const double* state_vector_arg, int* N_terms, double* results, int* failed, mango::Problem* problem_arg, void* user_data_arg) {
  // Note that this method is static, so there is no "this".
  //assert(N_parameters_arg == N_parameters);
  //assert(problem_arg == problem);
  assert(*N_terms == 1);
  //std::cout << "problem_arg->get_N_parameters(): " << problem_arg->get_N_parameters() << std::endl;
  //std::cout << "problem_arg->get_solver()->finite_difference_step_size: " << problem_arg->get_solver()->finite_difference_step_size << std::endl;
  problem_arg->get_solver()->objective_function(N_parameters_arg, state_vector_arg, results, failed, problem_arg, user_data_arg);
  //objective_function(N_parameters_arg, state_vector_arg, results, failed, problem_arg, user_data_arg);
}

void mango::Solver::finite_difference_gradient(const double* state_vector, double* base_case_objective_function, double* gradient) {
  finite_difference_Jacobian(objective_to_vector_function, 1, state_vector, base_case_objective_function, gradient);
}

void mango::Solver::record_function_evaluation_pointer(const double* state_vector_arg, double* objective_function_arg, bool failed) {
  // This method is called from evaluate_set_in_parallel.
  // Call the method with the same name but a different signature
  if (verbose>0) std::cout << "Hello from void mango::Solver::record_function_evaluation_pointer(const double*, double*, bool)" << std::endl;
  record_function_evaluation(state_vector_arg, *objective_function_arg, failed);
}
