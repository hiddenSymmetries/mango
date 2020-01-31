#include <iostream>
#include <math.h>
#include <limits>
//#include <cstring>
#include <stdexcept>
#include <ctime>
#include "mango.hpp"
#include "Solver.hpp"

void mango::Solver::init_optimization() {

  // This subroutine carries out a few steps that are shared by both least_squares and conventional optimization.

  if (! mpi_partition->get_proc0_worker_groups()) 
    throw std::runtime_error("Error! The mango::optimize() subroutine should only be called by group leaders, not by all workers.");

  function_evaluations = 0;
  at_least_one_success = false;
  best_objective_function = std::numeric_limits<double>::quiet_NaN();
  best_function_evaluation = -1;
  start_time = clock();

  // To simplify code a bit...
  MPI_Comm mpi_comm_group_leaders = mpi_partition->get_comm_group_leaders();

  // Make sure that parameters used by the finite-difference gradient routine are the same for all group leaders:
  MPI_Bcast(&N_parameters, 1, MPI_INT, 0, mpi_comm_group_leaders);
  MPI_Bcast(&centered_differences, 1, MPI_C_BOOL, 0, mpi_comm_group_leaders);
  MPI_Bcast(&finite_difference_step_size, 1, MPI_DOUBLE, 0, mpi_comm_group_leaders);
  MPI_Bcast(&algorithm, 1, MPI_INT, 0, mpi_comm_group_leaders);
  // 20200127 These next 2 lines should end up in Least_squares_data::optimize()?
  //  MPI_Bcast(&N_terms, 1, MPI_INT, 0, mpi_comm_group_leaders);
  //  MPI_Bcast(&least_squares, 1, MPI_C_BOOL, 0, mpi_comm_group_leaders);

  if (algorithms[algorithm].requires_bound_constraints && (!bound_constraints_set)) 
    throw std::runtime_error("Error! A MANGO algorithm was chosen that requires bound constraints, but bound constraints were not set.");

  if (bound_constraints_set && (!algorithms[algorithm].allows_bound_constraints) && mpi_partition->get_proc0_world()) {
#define star_line "*******************************************************************************************************"
    std::cerr << star_line << std::endl;
    std::cerr << "WARNING! Bound constraints were set, but an algorithm was chosen that does not allow bound constraints." << std::endl;
    std::cerr << "Therefore, the bound constraints will be ignored for this calculation." << std::endl;
    std::cerr << star_line << std::endl;
  }

  if (algorithms[algorithm].uses_derivatives) {
    if (centered_differences) {
      max_function_and_gradient_evaluations = ceil(max_function_evaluations / 7.0);
    } else {
      max_function_and_gradient_evaluations = ceil(max_function_evaluations / 4.0);
    }
  } else {
    max_function_and_gradient_evaluations = max_function_evaluations;
  }

  set_package();

  if (verbose > 0) {
    std::cout << "Proc " << mpi_partition->get_rank_world() << " is entering optimize(), and thinks proc0_world=" << mpi_partition->get_proc0_world() << std::endl;
    std::cout << "max_function_evaluations = " << max_function_evaluations << 
      ", max_function_and_gradient_evaluations = " << max_function_and_gradient_evaluations << std::endl;
  }

  if (mpi_partition->get_proc0_world()) recorder->init();
}
