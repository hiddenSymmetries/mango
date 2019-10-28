#include<iostream>
#include<iomanip>
#include<mpi.h>
#include "mango.hpp"

void residual_function(int*, const double*, int*, double*, int*, mango::problem*);

void worker(mango::problem*);

int main(int argc, char *argv[]) {
  int ierr;

  std::cout << "Hello world from rosenbrock_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  double state_vector[2] = {0.0, 0.0};
  double targets[2] = {1.0, 0.0};
  double sigmas[2] = {1.0, 0.1};
  double best_residual_function[2];
  mango::problem myprob(2, state_vector, 2, targets, sigmas, best_residual_function, &residual_function, argc, argv);


  /*  myprob.set_algorithm(mango::PETSC_POUNDERS); */
  // myprob.set_algorithm("nlopt_ln_neldermead");
  myprob.read_input_file("../input/mango_in.rosenbrock_c");
  myprob.output_filename = "../output/mango_out.rosenbrock_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  /* myprob.centered_differences = true; */
  myprob.max_function_evaluations = 2000;

  double best_objective_function;
  if (myprob.is_proc0_worker_groups()) {
    best_objective_function = myprob.optimize();

    /* Make workers stop */
    int data[1];
    data[0] = -1;
    MPI_Bcast(data, 1, MPI_INT, 0, myprob.get_mpi_comm_worker_groups());
  } else {
    worker(&myprob);
  }

  if (myprob.is_proc0_world()) {
    std::cout << "Best state vector: " << std::setprecision(16);
    for (int j=0; j<2; j++) std::cout << state_vector[j] << "  ";
    std::cout << "\nBest objective function: " << best_objective_function << "\nBest residual vector:";
    for (int j=0; j<2; j++) std::cout << best_residual_function[j] << "  ";
    std::cout << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}


void residual_function(int* N_parameters, const double* x, int* N_terms, double* f, int* failed, mango::problem* this_problem) {
  int j;
  std::cout << "C residual function called with N="<< *N_parameters << "\n";
  /*   *f = (x[0] - 1) * (x[0] - 1) + 100 * (x[1] - x[0]*x[0]) * (x[1] - x[0]*x[0]); */
  f[0] = x[0];
  f[1] = x[1] - x[0] * x[0];
  *failed = false;
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
