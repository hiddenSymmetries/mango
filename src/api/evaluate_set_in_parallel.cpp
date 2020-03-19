// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <ctime>
#include "mpi.h"
#include "mango.hpp"
#include "Least_squares_solver.hpp"
 
void mango::Solver::evaluate_set_in_parallel(vector_function_type vector_function, int N_terms, int N_set, double* state_vectors, double* results, bool* failures) {

  // state_vectors should have been allocated with size N_parameters * N_set.
  // results should have been allocated with size N_terms * N_set.
  // failures should have been allocated with size N_set.

  // All group leaders (but not workers) should call this subroutine.

  // To simplify code in this file, make some copies of variables.
  MPI_Comm mpi_comm_group_leaders = mpi_partition->get_comm_group_leaders();
  bool proc0_world = mpi_partition->get_proc0_world();
  int mpi_rank_world = mpi_partition->get_rank_world();
  int mpi_rank_group_leaders = mpi_partition->get_rank_group_leaders();
  int N_worker_groups = mpi_partition->get_N_worker_groups();

  int data;
  int j_set, j_parameter;

  if (verbose > 0) std::cout << "Hello from evaluate_set_in_parallel from proc " << mpi_rank_world << std::endl;

  /*
  if (proc0_world) {
    // Tell the group leaders to start this subroutine 
    data = 1;
    MPI_Bcast(&data,1,MPI_INT,0,mpi_comm_group_leaders);
  }
  */

  // Make sure all procs agree on the input data.
  MPI_Bcast(&N_set, 1, MPI_INT, 0, mpi_comm_group_leaders);
  MPI_Bcast(&N_parameters, 1, MPI_INT, 0, mpi_comm_group_leaders);
  MPI_Bcast(state_vectors, N_set*N_parameters, MPI_DOUBLE, 0, mpi_comm_group_leaders);
  // We didn't actually need to send all the state vectors to all procs, only the subset of the state vectors
  // that a given proc will actually be responsible for. But the communication time is negligible compared
  // to evaluations of the objective function usually, and the approach here has the benefit of simplicity.

  // Each proc now evaluates the user function for its share of the set
  int failed_int;
  for(j_set=0; j_set < N_set; j_set++) {
    if ((j_set % N_worker_groups) == mpi_rank_group_leaders) {
      // Note that the use of &residual_functions[j_set*N_terms] in the next line means that j_terms must be the least-signficiant dimension in residual_functions.
      vector_function(&N_parameters, &state_vectors[j_set*N_parameters], &N_terms, &results[j_set*N_terms], &failed_int, problem, user_data);
      // I should record the failures here.
    }
  }

  // Send results back to the world master.
  // Make sure not to reduce over MPI_COMM_WORLD, since then the residual function values will be multiplied by # of workers per worker group.
  if (proc0_world) {
    MPI_Reduce(MPI_IN_PLACE, results, N_set * N_terms, MPI_DOUBLE, MPI_SUM, 0, mpi_comm_group_leaders);
  } else {
    MPI_Reduce(results,      results, N_set * N_terms, MPI_DOUBLE, MPI_SUM, 0, mpi_comm_group_leaders);
  }

  // Record the results in order in the output file. At the same time, check for any best-yet values of the
  // objective function.
  double total_objective_function;
  bool failed = false;
  clock_t now;
  if (proc0_world) {
    for(j_set=0; j_set<N_set; j_set++) {
      //current_residuals = &residual_functions[j_set*N_terms];
      //total_objective_function = residuals_to_single_objective(current_residuals);
      record_function_evaluation_pointer(&state_vectors[j_set*N_parameters], &results[j_set*N_terms], failed);
    }
  }
  
}
