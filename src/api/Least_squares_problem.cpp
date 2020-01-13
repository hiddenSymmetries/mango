#include <iostream>
#include <string>
#include <stdexcept>
#include "mango.hpp"
#include "Problem_data.hpp"

// Constructor for least-squares problems
mango::Least_squares_problem::Least_squares_problem(int N_parameters_in, double* state_vector_in, int N_terms_in, double* targets_in, double* sigmas_in, 
			double* best_residual_function_in, residual_function_type residual_function_in, int argc_in, char* argv_in[]) {
  data = new Problem_data(N_parameters_in);
  data->argc = argc_in;
  data->argv = argv_in;
  data->N_parameters = N_parameters_in;
  data->objective_function = NULL; // Should this be least_squares_to_single_objective?
  data->state_vector = state_vector_in;

  least_squares_data = new Least_squares_data(N_terms_in);
  least_squares_data->N_terms = N_terms_in;
  least_squares_data->targets = targets_in;
  least_squares_data->sigmas = sigmas_in;
  least_squares_data->residual_function = residual_function_in;
  least_squares_data->best_residual_function = best_residual_function_in;
}

// Destructor
mango::Least_squares_problem::~Least_squares_problem() {
  if (data->verbose > 0) std::cout << "Mango least squares problem is being destroyed." << std::endl;
  delete[] data;
  delete[] least_squares_data;
}

void mango::Least_squares_problem::set_print_residuals_in_output_file(bool new_bool) {
  least_squares_data->print_residuals_in_output_file = new_bool;
}

int mango::Least_squares_problem::get_N_terms() {
  return least_squares_data->N_terms;
}

double mango::Least_squares_problem::optimize() {
  // Delegate this work to Least_squares_data so we don't need to put "least_squares_data->" in front of everything.
  return least_squares_data->optimize();
}
