#define N_dims 3

#include<iostream>
#include<iomanip>
#include<cstring>
#include<mpi.h>
#include "mango.hpp"

void residual_function(int*, const double*, int*, double*, int*, mango::problem*);

int main(int argc, char *argv[]) {
  int ierr;

  std::cout << "Hello world from quadratic_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  /*  int N_procs, mpi_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &N_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  std::cout << "N_procs=" << N_procs << "mpi_rank=" << mpi_rank;

  std::cout << MANGO_PETSC_NM << "\nHello world!\n";
  */

  double state_vector[N_dims];
  memset(state_vector, 0, N_dims*sizeof(double)); /* Initial condition = 0. */

  double sigmas[N_dims];
  double targets[N_dims];
  for (int j=0; j<N_dims; j++) {
    sigmas[j] = (double) j+1;
    targets[j] = (double) j+1;
  }

  mango::problem myprob(N_dims, state_vector, N_dims, targets, sigmas, &residual_function, argc, argv);

  //std::cout << "Here comes state vector:" << *(myprob.state_vector);
  /*
  myprob.state_vector[0] =  5.0;
  myprob.state_vector[1] = 10.0;
  myprob.state_vector[2] = 15.0;
  myprob.state_vector[3] = 20.0;
  */

  /*  myprob.set_algorithm(mango::PETSC_POUNDERS); */
  //  myprob.set_algorithm(mango::NLOPT_LD_LBFGS);
  // myprob.set_algorithm("nlopt_ln_neldermead");
  myprob.read_input_file("../input/mango_in.quadratic_c");
  myprob.output_filename = "../output/mango_out.quadratic_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  /* myprob.centered_differences = true; */

  myprob.optimize();

  MPI_Finalize();

  return 0;
}


void residual_function(int* N, const double* x, int* M, double* f, int* failed, mango::problem* this_problem) {
  int j;
  std::cout << "C residual function called with N="<< *N << ", M=" << *M << "\n";
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
