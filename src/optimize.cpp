#include<iostream>
#include<math.h>
#include<limits>
#include<cstring>
#include<stdlib.h>
#include "mango.hpp"

double mango::problem::optimize() {

  if (! proc0_worker_groups) {
    std::cout << "Error! The mango_optimize() subroutine should only be called by group leaders, not by all workers.\n";
    exit(1);
  }

  function_evaluations = 0;
  at_least_one_success = false;
  best_objective_function = std::numeric_limits<double>::quiet_NaN();
  best_function_evaluation = -1;

  /* Make sure that parameters used by the finite-difference gradient routine are the same for all group leaders: */
  MPI_Bcast(&N_parameters, 1, MPI_INT, 0, mpi_comm_group_leaders);
  MPI_Bcast(&N_terms, 1, MPI_INT, 0, mpi_comm_group_leaders);
  MPI_Bcast(&centered_differences, 1, MPI_C_BOOL, 0, mpi_comm_group_leaders);
  MPI_Bcast(&finite_difference_step_size, 1, MPI_DOUBLE, 0, mpi_comm_group_leaders);
  MPI_Bcast(&least_squares, 1, MPI_C_BOOL, 0, mpi_comm_group_leaders);
  MPI_Bcast(&N_worker_groups, 1, MPI_INT, 0, mpi_comm_group_leaders);
  MPI_Bcast(&algorithm, 1, MPI_INT, 0, mpi_comm_group_leaders);
  load_algorithm_properties(); /* Now that all group leader procs agree on algorithm, these procs will get the correct algorithm properties. */

  if (algorithm_uses_derivatives) {
    if (centered_differences) {
      max_function_and_gradient_evaluations = ceil(max_function_evaluations / 7.0);
    } else {
      max_function_and_gradient_evaluations = ceil(max_function_evaluations / 4.0);
    }
  } else {
    max_function_and_gradient_evaluations = max_function_evaluations;
  }

  std::cout << "Proc " << mpi_rank_world << " is entering optimize(), and thinks proc0_world=" << proc0_world << "\n";
  std::cout << "max_function_evaluations = " << max_function_evaluations << 
    ", max_function_and_gradient_evaluations = " << max_function_and_gradient_evaluations << "\n";

  if (least_squares) {
    optimize_least_squares();
    return(best_objective_function);
  }

  if (!proc0_world) {
    group_leaders_loop();
    return(std::numeric_limits<double>::quiet_NaN());
  }
  /* Only proc0_world continues past this point. */

  std::cout << "Hello world from optimize()\n";

  /* Open output file */
  output_file.open(output_filename.c_str());
  if (!output_file.is_open()) {
    std::cout << "Error! Unable to open output file " << output_filename << "\n";
    exit(1);
  }
  /* Write header line of output file */
  output_file << "Least squares?\nno\nN_parameters:\n" << N_parameters << "\nfunction_evaluation";
  for (int j=0; j<N_parameters; j++) {
    output_file << ",x(" << j+1 << ")";
  }
  output_file << ",objective_function\n";

  if (least_squares_algorithm) {
    std::cout << "Error! An algorithm for least-squares problems was chosen, but the problem specified is not least-squares.\n";
    exit(1);
  }

  switch (package) {
  case PACKAGE_PETSC:
    optimize_petsc();
    break;
  case PACKAGE_NLOPT:
    optimize_nlopt();
    break;
  case PACKAGE_HOPSPACK:
    optimize_hopspack();
    break;
  case PACKAGE_GSL:
    optimize_gsl();
    break;
  default:
    std::cout << "Error! Unrecognized package.\n";
    exit(1);
  }

  /* Tell the other group leaders to exit. */
  int data = -1;
  MPI_Bcast(&data,1,MPI_INT,0,mpi_comm_group_leaders);

  memcpy(state_vector, best_state_vector, N_parameters * sizeof(double)); /* Make sure we leave state_vector equal to the best state vector seen. */

  /* Copy the line corresponding to the optimum to the bottom of the output file. */
  int function_evaluations_temp = function_evaluations;
  function_evaluations = best_function_evaluation;
  write_file_line(state_vector, best_objective_function);
  function_evaluations = function_evaluations_temp;

  output_file.close();

  std::cout << "Here comes the optimal state_vector from optimize.cpp: " << state_vector[0];
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

  return(best_objective_function);
}
