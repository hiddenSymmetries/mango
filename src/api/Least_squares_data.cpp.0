#include <stdexcept>
#include "mango.hpp"
#include "Least_squares_data.hpp"

// Constructor
mango::Least_squares_data::Least_squares_data(int N_terms_in) {
  if (N_terms_in < 1) throw std::runtime_error("Error in mango::Least_squares_data::Least_squares_data(int). N_terms must be at least 1.")
  N_terms = N_terms_in;

  // Defaults are set in the following lines
  targets = NULL;
  sigmas = NULL;
  residual_function = NULL;
  best_residual_function = NULL;
  residuals = new double[N_terms_in];
  print_residuals_in_output_file = true;
}

// Destructor
mango::Least_squares_data::~Least_squares_data() {
  delete[] residuals;
}
