#include<iostream>
#include "mango.hpp"

void mango::problem::optimize() {

  if (! proc0_worker_groups) {
    std::cout << "Error! The mango_optimize() subroutine should only be called by group leaders, not by all workers.\n";
    exit(1);
  }

  /* Make sure that parameters used by the finite-difference gradient routine are the same for all group leaders: */
  MPI_Bcast(&N_parameters, 1, MPI_INT, 0, mpi_comm_group_leaders);
  MPI_Bcast(&centered_differences, 1, MPI_C_BOOL, 0, mpi_comm_group_leaders);
  MPI_Bcast(&finite_difference_step_size, 1, MPI_DOUBLE, 0, mpi_comm_group_leaders);
  MPI_Bcast(&least_squares, 1, MPI_C_BOOL, 0, mpi_comm_group_leaders);
  /* Make sure all procs agree on certain variables that will be used here. */
  MPI_Bcast(&N_worker_groups, 1, MPI_INT, 0, mpi_comm_world);
  MPI_Bcast(&algorithm, 1, MPI_INT, 0, mpi_comm_world);
  get_algorithm_properties(); /* Now that all procs agree on algorithm, all procs will get the correct algorithm properties. */

  if (least_squares) {
    optimize_least_squares();
    return;
  }

  if (!proc0_world) {
    group_leaders_loop();
    return;
  }
  /* Only proc0_world continues past this point. */

  std::cout << "Hello world from optimize()\n";
  function_evaluations = 0;

  /* Open output file */
  output_file.open(output_filename);
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
  objective_function(&N_parameters, state_vector, &f, &failed, this);
  std::cout << "Value of objective function: " << f << "\n";
}
