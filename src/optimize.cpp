#include<iostream>
#include "mango.h"

void mango::problem::optimize() {
  least_squares = false;

  if (! proc0_worker_groups) {
    std::cout << "Error! The mango_optimize() subroutine should only be called by group leaders, not by all workers.\n";
    exit(1);
  }

  std::cout << "Here comes state_vector from optimize.cpp: " << state_vector[0];
  for (int j=1; j<N_parameters; j++) {
    std::cout << ", " << state_vector[j];
  }

  std::cout << "\nAbout to call objective function from C.\n";
  double f;
  std::cout << "optimize.cpp: objective_function=" << (long int)objective_function << "\n";
  objective_function(&N_parameters, state_vector, &f);
  std::cout << "Value of objective function: " << f << "\n";
}
