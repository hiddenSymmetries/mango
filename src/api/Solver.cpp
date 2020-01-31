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
}

// Destructor
mango::Solver::~Solver() {
  delete[] best_state_vector;
}
