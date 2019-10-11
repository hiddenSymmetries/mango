#include<iostream>
#include<mpi.h>
#include "mango.hpp"

void objective_function(int*, const double*, double*, int*);

int main(int argc, char *argv[]) {
  int ierr;

  std::cout << "Hello world from rosenbrock_c.\n";

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

  double state_vector[2] = {0.0, 0.0};

  mango::problem myprob(2, state_vector, &objective_function, argc, argv);

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
  myprob.read_input_file("../input/mango_in.rosenbrock_c");
  myprob.output_filename = "../output/mango_out.rosenbrock_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  /* myprob.centered_differences = true; */

  myprob.optimize();

  MPI_Finalize();

  return 0;
}


void objective_function(int* N, const double* x, double* f, int* failed) {
  int j;
  std::cout << "C objective function called with N="<< *N << "\n";
  *f = (x[0] - 1) * (x[0] - 1) + 100 * (x[1] - x[0]*x[0]) * (x[1] - x[0]*x[0]);
  *failed = false;
}
