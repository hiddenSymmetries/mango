#include<iostream>
#include "mango.hpp"

/* void least_squares_to_single_objective(int*, const double*, double*, int*); */

void mango::problem::optimize_least_squares() {
  int j;

  if (!proc0_world) {
    group_leaders_least_squares_loop();
    return;
  }
  /* Only proc0_world continues past this point. */

  std::cout << "Hello world from optimize_least_squares()\n";
  function_evaluations = 0;

  /* Verify that the sigmas array is all nonzero. */
  for (j=0; j<N_terms; j++) {
    if (sigmas[j] == 0.0) {
      std::cout << "Error! The (0-based) entry " << j << " in the sigmas array is 0. sigmas must all be nonzero.\n";
      exit(1);
    }
  }

  /* Open output file */
  output_file.open(output_filename);
  if (!output_file.is_open()) {
    std::cout << "Error! Unable to open output file " << output_filename << "\n";
    exit(1);
  }
  /* Write header line of output file */
  output_file << "Least squares?\nyes\nN_parameters:\n" << N_parameters << "\nfunction_evaluation";
  for (j=0; j<N_parameters; j++) {
    output_file << ",x(" << j+1 << ")";
  }
  output_file << ",objective_function";
  for (j=0; j<N_terms; j++) {
    output_file << ",F(" << j+1 << ")";
  }
  output_file << "\n" << std::flush;

  /* Sanity test */
  /* if (!least_squares_algorithm) {
    std::cout << "Error in optimize_least_squares(). Should not get here!\n";
    exit(1);
    }*/

  /*  objective_function = (mango::objective_function_type) &mango::problem::least_squares_to_single_objective; */
  objective_function = &mango::problem::least_squares_to_single_objective;

  switch (package) {
  case PACKAGE_PETSC:
    if (least_squares_algorithm) {
      optimize_least_squares_petsc();
    } else {
      optimize_petsc();
    }
    break;
  case PACKAGE_NLOPT:
    optimize_nlopt();
    break;
  case PACKAGE_HOPSPACK:
    optimize_hopspack();
    break;
  case PACKAGE_GSL:
    if (least_squares_algorithm) {
      optimize_least_squares_gsl();
    } else {
      optimize_gsl();
    }
    break;
  default:
    std::cout << "Error! Unrecognized package.\n";
    exit(1);
  }

  output_file.close();

  /* Tell the other group leaders to exit. */
  int data = -1;
  MPI_Bcast(&data,1,MPI_INT,0,mpi_comm_group_leaders);

  std::cout << "Here comes state_vector from optimize_least_squares.cpp: " << state_vector[0];
  for (int j=1; j<N_parameters; j++) {
    std::cout << ", " << state_vector[j];
  }
  std::cout << "\n";

  /*
  std::cout << "\nAbout to call objective function from C.\n";
  double f;
  int failed;
  std::cout << "optimize.cpp: objective_function=" << (long int)objective_function << "\n";
  objective_function(&N_parameters, state_vector, &f, &failed, this);
  std::cout << "Value of objective function: " << f << "\n";
  */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

void mango::problem::least_squares_to_single_objective(int* N, const double* x, double* f, int* failed_int, mango::problem* this_problem) {
  /* Note that this function is static, so we must use this_problem rather than this. */

  int N_terms = this_problem->get_N_terms();

  std::cout << "Hello from least_squares_to_single_objective\n";
  double* residuals = new double[N_terms];

  bool failed_bool;
  this_problem->residual_function_wrapper(x, residuals, &failed_bool);
  if (failed_bool) {
    *failed_int = 1; 
  } else {
    *failed_int = 0;
  }

  double term;
  *f = 0;
  for (int j=0; j<N_terms; j++) {
    term = (residuals[j] - this_problem->targets[j]) / this_problem->sigmas[j];
    *f += term*term;
  }

  delete[] residuals;
}
