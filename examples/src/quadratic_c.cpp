#define N_dims 3

#include<iostream>
#include<iomanip>
#include<cstring>
#include<mpi.h>
#include "mango.hpp"

void residual_function(int*, const double*, int*, double*, int*, mango::problem*);

void worker(mango::problem*);

int main(int argc, char *argv[]) {
  int ierr;

  std::cout << "Hello world from quadratic_c.\n";

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

  mango::problem myprob(N_dims, state_vector, N_dims, targets, sigmas, &residual_function, argc, argv);

  myprob.read_input_file("../input/mango_in.quadratic_c");
  myprob.output_filename = "../output/mango_out.quadratic_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.centered_differences = true; 

  if (myprob.is_proc0_worker_groups()) {
    myprob.optimize();

    /* Make workers stop */
    int data[1];
    data[0] = -1;
    MPI_Bcast(data, 1, MPI_INT, 0, myprob.get_mpi_comm_worker_groups());
  } else {
    worker(&myprob);
  }

  MPI_Finalize();

  return 0;
}


void residual_function(int* N, const double* x, int* M, double* f, int* failed, mango::problem* this_problem) {
  int j;
  std::cout << "C residual function called on proc " << this_problem->get_mpi_rank_world() << " with N="<< *N << ", M=" << *M << "\n";

  /* Mobilize the workers in the group with this group leader: */
  int data[1];
  data[0] = this_problem->get_worker_group();
  MPI_Bcast(data, 1, MPI_INT, 0, this_problem->get_mpi_comm_worker_groups());

  for (j=0; j < *N; j++) {
    f[j] = x[j];
  }
  *failed = false;
  
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



void worker(mango::problem* myprob) {
  bool keep_going = true;
  int data[1];

  while (keep_going) {
    MPI_Bcast(data, 1, MPI_INT, 0, myprob->get_mpi_comm_worker_groups());
    if (data[0] < 0) {
      std::cout << "Proc " << std::setw(5) << myprob->get_mpi_rank_world() << " is exiting.\n";
      keep_going = false;
    } else {
      std::cout<< "Proc " << std::setw(5) << myprob->get_mpi_rank_world() << " is doing calculation " << data[0] << "\n";
      /* For this problem, the workers don't actually do any work. */
    }
  }
}
