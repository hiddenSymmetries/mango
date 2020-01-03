#define N_dims 3
#define verbose_level 0

#include<iostream>
#include<iomanip>
#include<cstring>
#include<mpi.h>
#include<math.h>
#include<stdlib.h>
#include<cassert>
#include "mango.hpp"

void objective_function(int*, const double*, double*, int*, mango::problem*, void*);

void worker(mango::problem*);

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

  mango::problem myprob(N_dims, state_vector, &objective_function, argc, argv);

  myprob.verbose = verbose_level;
  myprob.read_input_file("../input/mango_in.nondifferentiable_c");
  myprob.output_filename = "../output/mango_out.nondifferentiable_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.centered_differences = true;
  myprob.max_function_evaluations = 2000;

  // Pass some data to the objective function
  int data = 7;
  myprob.user_data = &data;

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
    std::cout << "\nBest objective function: " << best_objective_function << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}


void objective_function(int* N, const double* x, double* f, int* failed, mango::problem* this_problem, void* void_user_data) {
  int j;
  if (verbose_level > 0) std::cout << "C objective function called on proc " << this_problem->mpi_partition.get_rank_world() << " with N="<< *N << "\n";

  // Verify that the user data was passed successfully.
  int* user_data = (int*)void_user_data;
  assert(*user_data == 7);

  // Mobilize the workers in the group with this group leader:
  this_problem->mpi_partition.mobilize_workers();

  *f = 0;
  for (int j=1; j <= *N; j++) {
    *f += fabs(x[j-1] - j) / j;
  }
  *failed = false;
}

void worker(mango::problem* myprob) {
  while (myprob->mpi_partition.continue_worker_loop()) {
    // For this problem, the workers don't actually do any work.
    if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " could do some work here." << std::endl;
  }
  if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting." << std::endl;
}
