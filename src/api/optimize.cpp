#include <iostream>
#include <math.h>
#include <limits>
#include <cstring>
#include <stdexcept>
#include <ctime>
#include "mango.hpp"
#include "Solver.hpp"

double mango::Solver::optimize(MPI_Partition* mpi_partition_in) {
  // This subroutine is run only for non-least-squares optimizaiton.
  // It carries out the part of the optimization that is not specific to any one package.

  mpi_partition = mpi_partition_in;
  bool proc0_world = mpi_partition->get_proc0_world();
  if (proc0_world && verbose > 0) std::cout << "Hello world from optimize()" << std::endl;

  init_optimization();

  if (algorithms[algorithm].uses_derivatives && !proc0_world) {
    // All group leaders that are not proc0_world do group_leaders_loop(), then return.
    group_leaders_loop();
    return(std::numeric_limits<double>::quiet_NaN());
  }

  // proc0_world always continues past this point.
  // For finite-difference-derivative algorithms, the other procs do not go past this point.
  // For parallel algorithms that do not use finite-difference derivatives, such as HOPSPACK, the other group leader procs DO continue past this point.

  if (algorithms[algorithm].least_squares)
    throw std::runtime_error("Error! An algorithm for least-squares problems was chosen, but the problem specified is not least-squares.");

  // Hand control over to one of the concrete Packages to carry out the main work of the optimization.
  package->optimize(this);

  if (!proc0_world) return(std::numeric_limits<double>::quiet_NaN());
  // Only proc0_world continues past this point.

  // Tell the other group leaders to exit.
  int data = -1;
  MPI_Bcast(&data,1,MPI_INT,0,mpi_partition->get_comm_group_leaders());

  memcpy(state_vector, best_state_vector, N_parameters * sizeof(double)); // Make sure we leave state_vector equal to the best state vector seen.

  recorder->finalize();

  if (verbose > 0) {
    std::cout << "Here comes the optimal state_vector from optimize.cpp: " << state_vector[0];
    for (int j=1; j<N_parameters; j++) {
      std::cout << ", " << state_vector[j];
    }
    std::cout << std::endl;
  }

  return(best_objective_function);
}
