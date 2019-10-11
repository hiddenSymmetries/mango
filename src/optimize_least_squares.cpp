#include<iostream>
#include "mango.hpp"

void least_squares_to_single_objective(int*, const double*, double*, int*);

void mango::problem::optimize_least_squares() {
  
  if (!proc0_world) {
    group_leaders_least_squares_loop();
    return;
  }
  /* Only proc0_world continues past this point. */

  std::cout << "Hello world from optimize_least_squares()\n";
  function_evaluations = 0;

  /* Open output file */
  output_file.open(output_filename);
  if (!output_file.is_open()) {
    std::cout << "Error! Unable to open output file " << output_filename << "\n";
    exit(1);
  }
  /* Write header line of output file */
  output_file << "Least squares?\nyes\nN_parameters:\n" << N_parameters << "\nfunction_evaluation";
  for (int j=0; j<N_parameters; j++) {
    output_file << ",x(" << j+1 << ")";
  }
  output_file << ",objective_function";
  for (int j=0; j<N_terms; j++) {
    output_file << ",F(" << j+1 << ")";
  }
  output_file << "\n" << std::flush;

  /* Sanity test */
  if (!least_squares_algorithm) {
    std::cout << "Error in optimize_least_squares(). Should not get here!\n";
    exit(1);
  }

  objective_function = &least_squares_to_single_objective;

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

  std::cout << "Here comes state_vector from optimize.cpp: " << state_vector[0];
  for (int j=1; j<N_parameters; j++) {
    std::cout << ", " << state_vector[j];
  }

  std::cout << "\nAbout to call objective function from C.\n";
  double f;
  int failed;
  std::cout << "optimize.cpp: objective_function=" << (long int)objective_function << "\n";
  objective_function(&N_parameters, state_vector, &f, &failed);
  std::cout << "Value of objective function: " << f << "\n";
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

void least_squares_to_single_objective(int* N, const double* x, double* f, int* failed) {
  /*  double* residuals = new double[ */
}
