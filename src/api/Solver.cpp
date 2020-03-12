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
}


// Destructor
mango::Solver::~Solver() {
  delete[] best_state_vector;
}
