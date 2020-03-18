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

// The purpose of this example is to test the behavior of least-squares algorithms
// when the number of parameters exceeds the number of least-squares terms. Some algorithms
// like those in GSL fail in this case.
//
// Objective function: f = [z - x^2 - y^2]^2 + [x + y + z - (-1)]^2
//
// The first term is minimized for points on the graph z = x^2 + y^2.
// The second term is minimized for points on the plane x + y + z = -1.
// Since this graph and plane do not intersect, the optimum of the total f is >0.
// The optimum is achieved at a point near the point of closest approach between the graph and plane.
// State vector at optimum: [-0.5, -0.5, 0.25]
// Minimum objective function: f = 0.125
// Residual functions at optimum: [-0.25, -0.75]

#define verbose_level 0

#include <iostream>
#include <iomanip>
#include <mpi.h>
#include <stdlib.h>
#include <cassert>
#include "mango.hpp"

#define N_parameters 3
#define N_terms 2

void residual_function(int*, const double*, int*, double*, int*, mango::Problem*, void*);

void worker(mango::Least_squares_problem*);

int main(int argc, char *argv[]) {
  int ierr;

  if (verbose_level > 0) std::cout << "Hello world from more_parameters_than_terms_c." << std::endl;

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cerr << "Error in MPI_Init." << std::endl;
    exit(1);
  }

  double state_vector[N_parameters] = {0.0, 0.0, 0.0};
  double sigmas[N_terms] = {1.0, 1.0};
  double targets[N_terms] = {0.0, -1.0};

  if (verbose_level > 0) {
    std::cout << "Is foobar a valid algorithm? " << mango::does_algorithm_exist("foobar") << "\n";
    std::cout << "Is petsc_nm a valid algorithm? " << mango::does_algorithm_exist("petsc_nm") << "\n";
  }

  double best_residual_function[N_terms];
  mango::Least_squares_problem myprob(N_parameters, state_vector, N_terms, targets, sigmas, best_residual_function, &residual_function, argc, argv);

  myprob.set_verbose(verbose_level);
  myprob.read_input_file("../input/mango_in.more_parameters_than_terms_c");
  myprob.set_output_filename("../output/mango_out.more_parameters_than_terms_c");
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.set_centered_differences(true);
  myprob.set_max_function_evaluations(2000);
  myprob.set_N_line_search(3); // To make results independent of the # of MPI processes, N_line_search must be set to any positive integer.

  // Pass some data to the objective function
  int data = 7;
  myprob.set_user_data((void*)&data);

  double lower_bounds[N_parameters];
  double upper_bounds[N_parameters];
  for (int j=0; j<N_parameters; j++) {
    lower_bounds[j] = -5.0;
    upper_bounds[j] =  5.0;
  }

  myprob.set_bound_constraints(lower_bounds, upper_bounds);

  double best_objective_function;
  if (myprob.mpi_partition.get_proc0_worker_groups()) {
    best_objective_function = myprob.optimize();
    myprob.mpi_partition.stop_workers();
  } else {
    worker(&myprob);
  }

  if (myprob.mpi_partition.get_proc0_world() && (verbose_level > 0)) {
    std::cout << "Best state vector: " << std::setprecision(16);
    for (int j=0; j<N_parameters; j++) std::cout << state_vector[j] << "  ";
    std::cout << "\nBest objective function: " << best_objective_function << "\nBest residual vector:";
    for (int j=0; j<2; j++) std::cout << best_residual_function[j] << "  ";
    std::cout << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}


void residual_function(int* N, const double* x, int* M, double* f, int* failed, mango::Problem* this_problem, void* void_user_data) {
  int j;
  if (verbose_level > 0) std::cout << "C residual function called on proc " << this_problem->mpi_partition.get_rank_world() << " with N="<< *N << ", M=" << *M << "\n";

  // Verify that the user data was passed successfully.
  int* user_data = (int*)void_user_data;
  assert(*user_data == 7);

  assert(*N == N_parameters);
  assert(*M == N_terms);

  // Mobilize the workers in the group with this group leader:
  this_problem->mpi_partition.mobilize_workers();

  f[0] = x[2] - x[0] * x[0] - x[1] * x[1];
  f[1] = x[0] + x[1] + x[2];

  *failed = false;
  
  if (verbose_level > 0) {
    std::cout << "state_vector:";
    for (j=0; j < *N; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x[j];
    }
    std::cout << "\n";
    std::cout << "residual:";
    for (j=0; j < *M; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << f[j];
    }
    std::cout << "\n" << std::flush;
  }
}


void worker(mango::Least_squares_problem* myprob) {
  while (myprob->mpi_partition.continue_worker_loop()) {
    // For this problem, the workers don't actually do any work.
    if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " could do some work here." << std::endl;
  }
  if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting." << std::endl;
}
