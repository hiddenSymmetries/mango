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

// Destructor
mango::Least_squares_solver::~Least_squares_solver() {
  delete[] residuals;
}
