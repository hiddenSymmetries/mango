// Minimize f = (x[0] - x[1]) ^ 2 + x[1] ^ 2
// subject to
// x[0] in [2, 4]
// x[1] in [-3, 1]
//
// The solution is (x[0], x[1]) = (2, 1), and at this point f = 2.

#define N_dims 2
#define verbose_level 0

#include<iostream>
#include<iomanip>
#include<cstring>
#include<mpi.h>
#include<stdlib.h>
#include "mango.hpp"

void residual_function(int*, const double*, int*, double*, int*, mango::problem*);

void worker(mango::problem*);

int main(int argc, char *argv[]) {
  int ierr;

  if (verbose_level > 0) std::cout << "Hello world from quadratic_c." << std::endl;

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cerr << "Error in MPI_Init." << std::endl;
    exit(1);
  }

  double state_vector[N_dims] = {3.0, -2.0};
  double sigmas[N_dims] = {1.0, 1.0};
  double targets[N_dims] = {0.0, 0.0};

  double best_residual_function[N_dims];
  mango::problem myprob(N_dims, state_vector, N_dims, targets, sigmas, best_residual_function, &residual_function, argc, argv);

  myprob.verbose = verbose_level;
  myprob.read_input_file("../input/mango_in.active_bound_constraints_c");
  myprob.output_filename = "../output/mango_out.active_bound_constraints_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.max_function_evaluations = 500;

  double lower_bounds[N_dims] = {2.0, -3.0};
  double upper_bounds[N_dims] = {4.0, 1.0};

  myprob.set_bound_constraints(lower_bounds, upper_bounds);

  double best_objective_function;
  if (myprob.mpi_partition.get_proc0_worker_groups()) {
    best_objective_function = myprob.optimize();

    // Make workers stop
    int data[1];
    data[0] = -1;
    MPI_Bcast(data, 1, MPI_INT, 0, myprob.mpi_partition.get_comm_worker_groups());
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


void residual_function(int* N, const double* x, int* M, double* f, int* failed, mango::problem* this_problem) {
  int j;
  if (verbose_level > 0) std::cout << "C residual function called on proc " << this_problem->mpi_partition.get_rank_world() << " with N="<< *N << ", M=" << *M << std::endl;

  // Mobilize the workers in the group with this group leader:
  int data[1];
  data[0] = this_problem->mpi_partition.get_worker_group();
  MPI_Bcast(data, 1, MPI_INT, 0, this_problem->mpi_partition.get_comm_worker_groups());

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



void worker(mango::problem* myprob) {
  bool keep_going = true;
  int data[1];

  while (keep_going) {
    MPI_Bcast(data, 1, MPI_INT, 0, myprob->mpi_partition.get_comm_worker_groups());
    if (data[0] < 0) {
      if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting." << std::endl;
      keep_going = false;
    } else {
      if (verbose_level > 0) std::cout<< "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is doing calculation " << data[0] << std::endl;
      // For this problem, the workers don't actually do any work.
    }
  }
}
