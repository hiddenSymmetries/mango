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

// The standard 2D Rosenbrock banana function:
// f(x,y) = (x - 1)^2 + 100 * (y - x^2)^2
// The optimum is at (x,y) = (1,1), and the objective function there is f=0.
//
// This example demonstrates several things:
// * Unconstrained least-squares minimization.
// * The problem is more challenging than the 'quadratic_c' example due to the objective function having a steep curved valley.
// * Passing an integer parameter to the objective function through the 'user_data'.

#define verbose_level 0

#include <iostream>
#include <iomanip>
#include <mpi.h>
#include <stdlib.h>
#include <cassert>
#include "mango.hpp"

void residual_function(int*, const double*, int*, double*, int*, mango::Problem*, void*);

void worker(mango::Least_squares_problem*);

int main(int argc, char *argv[]) {
  int ierr;

  if (verbose_level > 0) std::cout << "Hello world from rosenbrock_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  double state_vector[2] = {0.0, 0.0};
  double targets[2] = {1.0, 0.0};
  double sigmas[2] = {1.0, 0.1};
  double best_residual_function[2];
  mango::Least_squares_problem myprob(2, state_vector, 2, targets, sigmas, best_residual_function, &residual_function, argc, argv);

  std::string extension = "rosenbrock_c";
  //  myprob.set_algorithm(mango::PETSC_POUNDERS);
  // myprob.set_algorithm("nlopt_ln_neldermead");
  myprob.set_verbose(verbose_level);
  myprob.read_input_file("../input/mango_in." + extension);
  myprob.set_output_filename("../output/mango_out." + extension);
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.mpi_partition.write("../output/mango_mpi." + extension);
  // myprob.set_centered_differences(true);
  myprob.set_max_function_evaluations(2000);
  myprob.set_N_line_search(3); // To make results independent of the # of MPI processes, N_line_search must be set to any positive integer.

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
    for (int j=0; j<2; j++) std::cout << state_vector[j] << "  ";
    std::cout << "\nBest objective function: " << best_objective_function << "\nBest residual vector:";
    for (int j=0; j<2; j++) std::cout << best_residual_function[j] << "  ";
    std::cout << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}


void residual_function(int* N_parameters, const double* x, int* N_terms, double* f, int* failed, mango::Problem* this_problem, void* void_user_data) {
  int j;
  if (verbose_level > 0) std::cout << "C residual function called with N="<< *N_parameters << "\n";

  // Mobilize the workers in the group with this group leader:
  this_problem->mpi_partition.mobilize_workers();

  // Verify that the user data was passed successfully.
  int* user_data = (int*)void_user_data;
  assert(*user_data == 7);

  //   *f = (x[0] - 1) * (x[0] - 1) + 100 * (x[1] - x[0]*x[0]) * (x[1] - x[0]*x[0]); 
  f[0] = x[0];
  f[1] = x[1] - x[0] * x[0];
  *failed = false;
}


void worker(mango::Least_squares_problem* myprob) {
  while (myprob->mpi_partition.continue_worker_loop()) {
    // For this problem, the workers don't actually do any work.
    if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " could do some work here." << std::endl;
  }
  if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting." << std::endl;
}
