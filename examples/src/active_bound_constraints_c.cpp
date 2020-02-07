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

////////////////////////////////////////////////////////////////////////

// Minimize f = (x[0] - x[1]) ^ 2 + x[1] ^ 2
// subject to
// x[0] in [2, 4]
// x[1] in [-3, 1]
//
// The solution is (x[0], x[1]) = (2, 1), and at this point f = 2.
//
// This script also demonstrates set_relative_bound_constraints().

#define N_dims 2
#define verbose_level 0

#include <iostream>
#include <iomanip>
#include <cstring>
#include <mpi.h>
#include <stdlib.h>
#include <cassert>
#include "mango.hpp"

void residual_function(int*, const double*, int*, double*, int*, mango::Problem*, void*);

void worker(mango::Least_squares_problem*);

int main(int argc, char *argv[]) {
  int ierr;

  if (verbose_level > 0) std::cout << "Hello world from active_bound_constraints_c." << std::endl;

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cerr << "Error in MPI_Init." << std::endl;
    exit(1);
  }

  double state_vector[N_dims] = {3.0, -2.0};
  double sigmas[N_dims] = {1.0, 1.0};
  double targets[N_dims] = {0.0, 0.0};

  double best_residual_function[N_dims];
  mango::Least_squares_problem myprob(N_dims, state_vector, N_dims, targets, sigmas, best_residual_function, &residual_function, argc, argv);

  std::string extension = "active_bound_constraints_c";
  myprob.set_verbose(verbose_level);
  myprob.read_input_file("../input/mango_in." + extension);
  myprob.set_output_filename("../output/mango_out." + extension);
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.set_max_function_evaluations(500);

  double lower_bounds[N_dims];
  double upper_bounds[N_dims];

  myprob.set_bound_constraints(lower_bounds, upper_bounds);

  if (verbose_level>0) {
    myprob.set_relative_bound_constraints(0.5, 2.0, 0.0, false);
    std::cout << "After calling set_relative_bound_constraints(0.5, 2.0, 0.0, false)," << std::endl;
    std::cout << "  Lower bounds = " << lower_bounds[0] << ", " << lower_bounds[1] << std::endl;
    std::cout << "  Upper bounds = " << upper_bounds[0] << ", " << upper_bounds[1] << std::endl;
    myprob.set_relative_bound_constraints(0.5, 2.0, 0.0, true);
    std::cout << "After calling set_relative_bound_constraints(0.5, 2.0, 0.0, true)," << std::endl;
    std::cout << "  Lower bounds = " << lower_bounds[0] << ", " << lower_bounds[1] << std::endl;
    std::cout << "  Upper bounds = " << upper_bounds[0] << ", " << upper_bounds[1] << std::endl;
    // Restore bounds to their original values
  }

  lower_bounds[0] =  2.0;
  lower_bounds[1] = -3.0;
  upper_bounds[0] =  4.0;
  upper_bounds[1] =  1.0;

  // Pass some data to the objective function
  int data = 7;
  myprob.set_user_data((void*)&data);

  double best_objective_function;
  if (myprob.mpi_partition.get_proc0_worker_groups()) {
    best_objective_function = myprob.optimize();
    myprob.mpi_partition.stop_workers();
  } else {
    worker(&myprob);
  }

  if (myprob.mpi_partition.get_proc0_world() && (verbose_level > 0)) {
    std::cout << "Best state vector: " << std::setprecision(16);
    for (int j=0; j<N_dims; j++) std::cout << state_vector[j] << "  ";
    std::cout << std::endl << "Best objective function: " << best_objective_function << std::endl << "Best residual vector:";
    for (int j=0; j<N_dims; j++) std::cout << best_residual_function[j] << "  ";
    std::cout << std::endl << "Best function evaluation was " << myprob.get_best_function_evaluation() << std::endl;
  }

  MPI_Finalize();

  return 0;
}


void residual_function(int* N, const double* x, int* M, double* f, int* failed, mango::Problem* this_problem, void* void_user_data) {
  int j;
  if (verbose_level > 0) std::cout << "C residual function called on proc " << this_problem->mpi_partition.get_rank_world() << " with N="<< *N << ", M=" << *M << std::endl;

  // Verify that the user data was passed successfully.
  int* user_data = (int*)void_user_data;
  assert(*user_data == 7);

  // Mobilize the workers in the group with this group leader:
  this_problem->mpi_partition.mobilize_workers();

  f[0] = x[0] - x[1];
  f[1] = x[1];
  *failed = false;
  
  if (verbose_level > 0) {
    std::cout << "state_vector:";
    for (j=0; j < *N; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x[j];
    }
    std::cout << std::endl;
    std::cout << "residual:";
    for (j=0; j < *M; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << f[j];
    }
    std::cout << std::endl << std::flush;
  }
}



void worker(mango::Least_squares_problem* myprob) {
  while (myprob->mpi_partition.continue_worker_loop()) {
    // For this problem, the workers don't actually do any work.
    if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " could do some work here." << std::endl;
  }
  if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting." << std::endl;
}
