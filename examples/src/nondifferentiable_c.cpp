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

#define N_dims 3
#define verbose_level 0

#include <iostream>
#include <iomanip>
#include <cstring>
#include <mpi.h>
#include <math.h>
#include <stdlib.h>
#include <cassert>
#include "mango.hpp"

void objective_function(int*, const double*, double*, int*, mango::Problem*, void*);

void worker(mango::MPI_Partition*);

int main(int argc, char *argv[]) {
  int ierr;

  if (verbose_level > 0) std::cout << "Hello world from nondifferentiable_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  double state_vector[N_dims];
  memset(state_vector, 0, N_dims*sizeof(double));

  mango::Problem myprob(N_dims, state_vector, &objective_function, argc, argv);

  std::string extension = "nondifferentiable_c";
  myprob.set_verbose(verbose_level);
  myprob.read_input_file("../input/mango_in." + extension);
  myprob.set_output_filename("../output/mango_out." + extension);
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.mpi_partition.write("../output/mango_mpi." + extension);
  myprob.set_centered_differences(true);
  myprob.set_max_function_evaluations(2000);

  // Pass some data to the objective function
  int data = 7;
  myprob.set_user_data((void*)&data);

  double best_objective_function;
  if (myprob.mpi_partition.get_proc0_worker_groups()) {
    best_objective_function = myprob.optimize();
    myprob.mpi_partition.stop_workers();
  } else {
    worker(&(myprob.mpi_partition));
  }

  if (myprob.mpi_partition.get_proc0_world() && (verbose_level > 0)) {
    std::cout << "Best state vector: " << std::setprecision(16);
    for (int j=0; j<N_dims; j++) std::cout << state_vector[j] << "  ";
    std::cout << "\nBest objective function: " << best_objective_function << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}


void objective_function(int* N, const double* x, double* f, int* failed, mango::Problem* problem, void* void_user_data) {
  int j;
  if (verbose_level > 0) std::cout << "C objective function called on proc " << problem->mpi_partition.get_rank_world() << " with N="<< *N << "\n";

  // Verify that the user data was passed successfully.
  int* user_data = (int*)void_user_data;
  assert(*user_data == 7);

  // Mobilize the workers in the group with this group leader:
  problem->mpi_partition.mobilize_workers();

  *f = 0;
  for (int j=1; j <= *N; j++) {
    *f += fabs(x[j-1] - j) / j;
  }
  *failed = false;
}

void worker(mango::MPI_Partition* mpi_partition) {
  while (mpi_partition->continue_worker_loop()) {
    // For this problem, the workers don't actually do any work.
    if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << mpi_partition->get_rank_world() << " could do some work here." << std::endl;
  }
  if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << mpi_partition->get_rank_world() << " is exiting." << std::endl;
}
