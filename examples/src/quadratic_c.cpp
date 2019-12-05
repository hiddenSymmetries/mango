#define N_dims 3
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

  if (verbose_level > 0) std::cout << "Hello world from quadratic_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  double state_vector[N_dims];
  memset(state_vector, 0, N_dims*sizeof(double)); /* Initial condition = 0. */

  double sigmas[N_dims];
  double targets[N_dims];
  for (int j=0; j<N_dims; j++) {
    sigmas[j] = (double) j+1;
    targets[j] = (double) j+1;
  }

  if (verbose_level > 0) {
    std::cout << "Is foobar a valid algorithm? " << mango::does_algorithm_exist("foobar") << "\n";
    std::cout << "Is petsc_nm a valid algorithm? " << mango::does_algorithm_exist("petsc_nm") << "\n";
  }

  double best_residual_function[N_dims];
  mango::problem myprob(N_dims, state_vector, N_dims, targets, sigmas, best_residual_function, &residual_function, argc, argv);

  myprob.verbose = verbose_level;
  myprob.read_input_file("../input/mango_in.quadratic_c");
  myprob.output_filename = "../output/mango_out.quadratic_c";
  if (myprob.get_algorithm() == 1) myprob.verbose = 1;
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.centered_differences = true; 
  myprob.max_function_evaluations = 2000;

  double lower_bounds[N_dims];
  double upper_bounds[N_dims];
  for (int j=0; j<N_dims; j++) {
    lower_bounds[j] = -5.0;
    upper_bounds[j] =  5.0;
  }

  myprob.set_bound_constraints(lower_bounds, upper_bounds);

  double best_objective_function;
  if (myprob.mpi_partition.get_proc0_worker_groups()) {
    best_objective_function = myprob.optimize();

    /* Make workers stop */
    int data[1];
    data[0] = -1;
    MPI_Bcast(data, 1, MPI_INT, 0, myprob.mpi_partition.get_comm_worker_groups());
  } else {
    worker(&myprob);
  }

  if (myprob.mpi_partition.get_proc0_world() && (verbose_level > 0)) {
    std::cout << "Best state vector: " << std::setprecision(16);
    for (int j=0; j<N_dims; j++) std::cout << state_vector[j] << "  ";
    std::cout << "\nBest objective function: " << best_objective_function << "\nBest residual vector:";
    for (int j=0; j<N_dims; j++) std::cout << best_residual_function[j] << "  ";
    std::cout << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}


void residual_function(int* N, const double* x, int* M, double* f, int* failed, mango::problem* this_problem) {
  int j;
  if (verbose_level > 0) std::cout << "C residual function called on proc " << this_problem->mpi_partition.get_rank_world() << " with N="<< *N << ", M=" << *M << "\n";

  /* Mobilize the workers in the group with this group leader: */
  int data[1];
  data[0] = this_problem->mpi_partition.get_worker_group();
  MPI_Bcast(data, 1, MPI_INT, 0, this_problem->mpi_partition.get_comm_worker_groups());

  for (j=0; j < *N; j++) {
    f[j] = x[j];
  }
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



void worker(mango::problem* myprob) {
  bool keep_going = true;
  int data[1];

  while (keep_going) {
    MPI_Bcast(data, 1, MPI_INT, 0, myprob->mpi_partition.get_comm_worker_groups());
    if (data[0] < 0) {
      if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting.\n";
      keep_going = false;
    } else {
      if (verbose_level > 0) std::cout<< "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is doing calculation " << data[0] << "\n";
      /* For this problem, the workers don't actually do any work. */
    }
  }
}
