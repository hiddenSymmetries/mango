#define N_dims 3

#include<iostream>
#include<iomanip>
#include<cstring>
#include<mpi.h>
#include<math.h>
#include "mango.hpp"

void objective_function(int*, const double*, double*, int*, mango::problem*);

void worker(mango::problem*);

int main(int argc, char *argv[]) {
  int ierr;

  std::cout << "Hello world from nondifferentiable_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  double state_vector[N_dims];
  memset(state_vector, 0, N_dims*sizeof(double));

  mango::problem myprob(N_dims, state_vector, &objective_function, argc, argv);

  myprob.read_input_file("../input/mango_in.nondifferentiable_c");
  myprob.output_filename = "../output/mango_out.nondifferentiable_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  myprob.centered_differences = true;
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
    for (int j=0; j<N_dims; j++) std::cout << state_vector[j] << "  ";
    std::cout << "\nBest objective function: " << best_objective_function << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}


void objective_function(int* N, const double* x, double* f, int* failed, mango::problem* this_problem) {
  int j;
  std::cout << "C objective function called on proc " << this_problem->get_mpi_rank_world() << " with N="<< *N << "\n";


  /* Mobilize the workers in the group with this group leader: */
  int data[1];
  data[0] = this_problem->get_worker_group();
  MPI_Bcast(data, 1, MPI_INT, 0, this_problem->get_mpi_comm_worker_groups());

  *f = 0;
  for (int j=1; j <= *N; j++) {
    *f += fabs(x[j-1] - j) / j;
  }
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
      std::cout<< "Proc " << std::setw(5) << myprob->get_mpi_rank_world() << " is doing calculation \
" << data[0] << "\n";
      /* For this problem, the workers don't actually do any work. */
    }
  }
}
