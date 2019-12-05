#define verbose_level 0

#include<iostream>
#include<iomanip>
#include<mpi.h>
#include<stdlib.h>
#include<stdexcept>
#include<cmath>
#include "mango.hpp"

#define N_terms 214

double y[N_terms];
double t[N_terms];

void init_data();

void partition_work(int, int, int*, int*);

void do_work(int, double*, double*, int, int);

void residual_function(int*, const double*, int*, double*, int*, mango::problem*);

void worker(mango::problem*);

int main(int argc, char *argv[]) {
  int ierr;

  if (verbose_level > 0) std::cout << "Hello world from chwirut_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  init_data();

  double state_vector[3] = {0.15, 0.008, 0.01};
  double targets[N_terms];
  double sigmas[N_terms];
  double best_residual_function[N_terms];
  for (int j=0; j<N_terms; j++) {
    targets[j] = 0.0;
    sigmas[j]  = 1.0;
  }
  mango::problem myprob(3, state_vector, N_terms, targets, sigmas, best_residual_function, &residual_function, argc, argv);

  /*  myprob.set_algorithm(mango::PETSC_POUNDERS); */
  // myprob.set_algorithm("nlopt_ln_neldermead");
  myprob.verbose = verbose_level;
  myprob.read_input_file("../input/mango_in.chwirut_c");
  myprob.output_filename = "../output/mango_out.chwirut_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  /* myprob.centered_differences = true; */
  myprob.max_function_evaluations = 2000;

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
    for (int j=0; j<3; j++) std::cout << state_vector[j] << "  ";
    std::cout << "\nBest objective function: " << best_objective_function << "\nFirst 3 elements of best residual vector: ";
    for (int j=0; j<3; j++) std::cout << best_residual_function[j] << "  ";
    std::cout << "\nBest function evaluation was " << myprob.get_best_function_evaluation() << "\n";
  }

  MPI_Finalize();

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void do_work(int N_parameters, double* x, double* f, int start_index, int stop_index) {
  for (int j=start_index; j <= stop_index; j++) {
    f[j] = y[j] - std::exp(-x[0] * t[j]) / (x[1] + x[2] * t[j]);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void residual_function(int* N_parameters, const double* x, int* N_terms_copy, double* f, int* failed, mango::problem* this_problem) {
  int j, start_index, stop_index;
  if (verbose_level > 0) std::cout << "C residual function called with N="<< *N_parameters << "\n";

  MPI_Status mpi_status;
  MPI_Comm comm_worker_groups = this_problem->mpi_partition.get_comm_worker_groups();
  int N_procs_worker_groups = this_problem->mpi_partition.get_N_procs_worker_groups();

  // Mobilize the workers in the group with this group leader:
  int data = 1; // Any nonnegative value will do here.
  MPI_Bcast(&data, 1, MPI_INT, 0, comm_worker_groups);

  // Send the state vector to all the other procs:
  int N = *N_parameters;
  MPI_Bcast((double*)x, N, MPI_DOUBLE, 0, comm_worker_groups);

  // Compute the residual terms on this proc, or receive the terms from the worker procs.
  for (j=0; j < N_procs_worker_groups; j++) {
    partition_work(j, N_procs_worker_groups, &start_index, &stop_index);
    if (j==0) {
      do_work(*N_parameters, (double*)x, f, start_index, stop_index);
    } else {
      MPI_Recv(&f[start_index], stop_index - start_index + 1, MPI_DOUBLE, j, j, comm_worker_groups, &mpi_status);
    }
  }

  *failed = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void worker(mango::problem* myprob) {
  bool keep_going = true;
  int data[1];

  MPI_Comm mpi_comm_worker_groups = myprob->mpi_partition.get_comm_worker_groups();
  int N_parameters = myprob->get_N_parameters();
  int mpi_rank_worker_groups = myprob->mpi_partition.get_rank_worker_groups();

  int start_index, stop_index;
  double x[3];
  double f[N_terms];

  partition_work(mpi_rank_worker_groups, myprob->mpi_partition.get_N_procs_worker_groups(), &start_index, &stop_index);

  while (keep_going) {
    MPI_Bcast(data, 1, MPI_INT, 0, mpi_comm_worker_groups);
    if (data[0] < 0) {
      if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting.\n";
      keep_going = false;
    } else {
      if (verbose_level > 0) std::cout<< "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is processing indices " << start_index << " to " << stop_index << "\n";

      // Get the state vector
      MPI_Bcast(x, N_parameters, MPI_DOUBLE, 0, mpi_comm_worker_groups);
	
      do_work(N_parameters, x, f, start_index, stop_index);

      // Send my terms of the residual back to the master proc.
      MPI_Send(&f[start_index], stop_index - start_index + 1, MPI_DOUBLE, 0, mpi_rank_worker_groups, mpi_comm_worker_groups);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init_data() {
  int i = 0;

  y[i] =   92.9000;   t[i++] =  0.5000;
  y[i] =    78.7000;  t[i++] =   0.6250;
  y[i] =    64.2000;  t[i++] =   0.7500;
  y[i] =    64.9000;  t[i++] =   0.8750;
  y[i] =    57.1000;  t[i++] =   1.0000;
  y[i] =    43.3000;  t[i++] =   1.2500;
  y[i] =    31.1000;   t[i++] =  1.7500;
  y[i] =    23.6000;   t[i++] =  2.2500;
  y[i] =    31.0500;   t[i++] =  1.7500;
  y[i] =    23.7750;   t[i++] =  2.2500;
  y[i] =    17.7375;   t[i++] =  2.7500;
  y[i] =    13.8000;   t[i++] =  3.2500;
  y[i] =    11.5875;   t[i++] =  3.7500;
  y[i] =     9.4125;   t[i++] =  4.2500;
  y[i] =     7.7250;   t[i++] =  4.7500;
  y[i] =     7.3500;   t[i++] =  5.2500;
  y[i] =     8.0250;   t[i++] =  5.7500;
  y[i] =    90.6000;   t[i++] =  0.5000;
  y[i] =    76.9000;   t[i++] =  0.6250;
  y[i] =    71.6000;   t[i++] = 0.7500;
  y[i] =    63.6000;   t[i++] =  0.8750;
  y[i] =    54.0000;   t[i++] =  1.0000;
  y[i] =    39.2000;   t[i++] =  1.2500;
  y[i] =    29.3000;   t[i++] = 1.7500;
  y[i] =    21.4000;   t[i++] =  2.2500;
  y[i] =    29.1750;   t[i++] =  1.7500;
  y[i] =    22.1250;   t[i++] =  2.2500;
  y[i] =    17.5125;   t[i++] =  2.7500;
  y[i] =    14.2500;   t[i++] =  3.2500;
  y[i] =     9.4500;   t[i++] =  3.7500;
  y[i] =     9.1500;   t[i++] =  4.2500;
  y[i] =     7.9125;   t[i++] =  4.7500;
  y[i] =     8.4750;   t[i++] =  5.2500;
  y[i] =     6.1125;   t[i++] =  5.7500;
  y[i] =    80.0000;   t[i++] =  0.5000;
  y[i] =    79.0000;   t[i++] =  0.6250;
  y[i] =    63.8000;   t[i++] =  0.7500;
  y[i] =    57.2000;   t[i++] =  0.8750;
  y[i] =    53.2000;   t[i++] =  1.0000;
  y[i] =   42.5000;   t[i++] =  1.2500;
  y[i] =   26.8000;   t[i++] =  1.7500;
  y[i] =    20.4000;   t[i++] =  2.2500;
  y[i] =    26.8500;  t[i++] =   1.7500;
  y[i] =    21.0000;  t[i++] =   2.2500;
  y[i] =    16.4625;  t[i++] =   2.7500;
  y[i] =    12.5250;  t[i++] =   3.2500;
  y[i] =    10.5375;  t[i++] =   3.7500;
  y[i] =     8.5875;  t[i++] =   4.2500;
  y[i] =     7.1250;  t[i++] =   4.7500;
  y[i] =     6.1125;  t[i++] =   5.2500;
  y[i] =     5.9625;  t[i++] =   5.7500;
  y[i] =    74.1000;  t[i++] =   0.5000;
  y[i] =    67.3000;  t[i++] =   0.6250;
  y[i] =    60.8000;  t[i++] =   0.7500;
  y[i] =    55.5000;  t[i++] =   0.8750;
  y[i] =    50.3000;  t[i++] =   1.0000;
  y[i] =    41.0000;  t[i++] =   1.2500;
  y[i] =    29.4000;  t[i++] =   1.7500;
  y[i] =    20.4000;  t[i++] =   2.2500;
  y[i] =    29.3625;  t[i++] =   1.7500;
  y[i] =    21.1500;  t[i++] =   2.2500;
  y[i] =    16.7625;  t[i++] =   2.7500;
  y[i] =    13.2000;  t[i++] =   3.2500;
  y[i] =    10.8750;  t[i++] =   3.7500;
  y[i] =     8.1750;  t[i++] =   4.2500;
  y[i] =     7.3500;  t[i++] =   4.7500;
  y[i] =     5.9625;  t[i++] =  5.2500;
  y[i] =     5.6250;  t[i++] =   5.7500;
  y[i] =    81.5000;  t[i++] =    .5000;
  y[i] =    62.4000;  t[i++] =    .7500;
  y[i] =    32.5000;  t[i++] =   1.5000;
  y[i] =    12.4100;  t[i++] =   3.0000;
  y[i] =    13.1200;  t[i++] =   3.0000;
  y[i] =    15.5600;  t[i++] =   3.0000;
  y[i] =     5.6300;  t[i++] =   6.0000;
  y[i] =    78.0000;   t[i++] =   .5000;
  y[i] =    59.9000;  t[i++] =    .7500;
  y[i] =    33.2000;  t[i++] =   1.5000;
  y[i] =    13.8400;  t[i++] =   3.0000;
  y[i] =    12.7500;  t[i++] =   3.0000;
  y[i] =    14.6200;  t[i++] =   3.0000;
  y[i] =     3.9400;  t[i++] =   6.0000;
  y[i] =    76.8000;  t[i++] =    .5000;
  y[i] =    61.0000;  t[i++] =    .7500;
  y[i] =    32.9000;  t[i++] =   1.5000;
  y[i] =   13.8700;   t[i++] = 3.0000;
  y[i] =    11.8100;  t[i++] =   3.0000;
  y[i] =    13.3100;  t[i++] =   3.0000;
  y[i] =     5.4400;  t[i++] =   6.0000;
  y[i] =    78.0000;  t[i++] =    .5000;
  y[i] =    63.5000;  t[i++] =    .7500;
  y[i] =    33.8000;  t[i++] =   1.5000;
  y[i] =    12.5600;  t[i++] =   3.0000;
  y[i] =     5.6300;  t[i++] =   6.0000;
  y[i] =    12.7500;  t[i++] =   3.0000;
  y[i] =    13.1200;  t[i++] =   3.0000;
  y[i] =     5.4400;  t[i++] =   6.0000;
  y[i] =    76.8000;  t[i++] =    .5000;
  y[i] =    60.0000;  t[i++] =    .7500;
  y[i] =    47.8000;  t[i++] =   1.0000;
  y[i] =    32.0000;  t[i++] =   1.5000;
  y[i] =    22.2000;  t[i++] =   2.0000;
  y[i] =    22.5700;  t[i++] =   2.0000;
  y[i] =    18.8200;  t[i++] =   2.5000;
  y[i] =    13.9500;  t[i++] =   3.0000;
  y[i] =    11.2500;  t[i++] =   4.0000;
  y[i] =     9.0000;  t[i++] =   5.0000;
  y[i] =     6.6700;  t[i++] =   6.0000;
  y[i] =    75.8000;  t[i++] =    .5000;
  y[i] =    62.0000;  t[i++] =    .7500;
  y[i] =    48.8000;  t[i++] =   1.0000;
  y[i] =    35.2000;  t[i++] =   1.5000;
  y[i] =    20.0000;  t[i++] =   2.0000;
  y[i] =    20.3200;  t[i++] =   2.0000;
  y[i] =    19.3100;  t[i++] =   2.5000;
  y[i] =    12.7500;  t[i++] =   3.0000;
  y[i] =    10.4200;  t[i++] =   4.0000;
  y[i] =     7.3100;  t[i++] =   5.0000;
  y[i] =     7.4200;  t[i++] =   6.0000;
  y[i] =    70.5000;  t[i++] =    .5000;
  y[i] =    59.5000;  t[i++] =    .7500;
  y[i] =    48.5000;  t[i++] =   1.0000;
  y[i] =    35.8000;  t[i++] =   1.5000;
  y[i] =    21.0000;  t[i++] =   2.0000;
  y[i] =    21.6700;  t[i++] =   2.0000;
  y[i] =    21.0000;  t[i++] =   2.5000;
  y[i] =    15.6400;  t[i++] =   3.0000;
  y[i] =     8.1700;  t[i++] =   4.0000;
  y[i] =     8.5500;  t[i++] =   5.0000;
  y[i] =    10.1200;  t[i++] =   6.0000;
  y[i] =    78.0000;  t[i++] =    .5000;
  y[i] =    66.0000;  t[i++] =    .6250;
  y[i] =    62.0000;  t[i++] =    .7500;
  y[i] =    58.0000;  t[i++] =    .8750;
  y[i] =    47.7000;  t[i++] =   1.0000;
  y[i] =    37.8000;  t[i++] =   1.2500;
  y[i] =    20.2000;  t[i++] =   2.2500;
  y[i] =    21.0700;  t[i++] =   2.2500;
  y[i] =    13.8700;  t[i++] =   2.7500;
  y[i] =     9.6700;  t[i++] =   3.2500;
  y[i] =     7.7600;  t[i++] =   3.7500;
  y[i] =    5.4400;   t[i++] =  4.2500;
  y[i] =    4.8700;   t[i++] =  4.7500;
  y[i] =     4.0100;  t[i++] =   5.2500;
  y[i] =     3.7500;  t[i++] =   5.7500;
  y[i] =    24.1900;  t[i++] =   3.0000;
  y[i] =    25.7600;  t[i++] =   3.0000;
  y[i] =    18.0700;  t[i++] =   3.0000;
  y[i] =    11.8100;  t[i++] =   3.0000;
  y[i] =    12.0700;  t[i++] =   3.0000;
  y[i] =    16.1200;  t[i++] =   3.0000;
  y[i] =    70.8000;  t[i++] =    .5000;
  y[i] =    54.7000;  t[i++] =    .7500;
  y[i] =    48.0000;  t[i++] =   1.0000;
  y[i] =    39.8000;  t[i++] =   1.5000;
  y[i] =    29.8000;  t[i++] =   2.0000;
  y[i] =    23.7000;  t[i++] =   2.5000;
  y[i] =    29.6200;  t[i++] =   2.0000;
  y[i] =    23.8100;  t[i++] =   2.5000;
  y[i] =    17.7000;  t[i++] =   3.0000;
  y[i] =    11.5500;  t[i++] =   4.0000;
  y[i] =    12.0700;  t[i++] =   5.0000;
  y[i] =     8.7400;  t[i++] =   6.0000;
  y[i] =    80.7000;  t[i++] =    .5000;
  y[i] =    61.3000;  t[i++] =    .7500;
  y[i] =    47.5000;  t[i++] =   1.0000;
   y[i] =   29.0000;  t[i++] =   1.5000;
   y[i] =   24.0000;  t[i++] =   2.0000;
  y[i] =    17.7000;  t[i++] =   2.5000;
  y[i] =    24.5600;  t[i++] =   2.0000;
  y[i] =    18.6700;  t[i++] =   2.5000;
   y[i] =   16.2400;  t[i++] =   3.0000;
  y[i] =     8.7400;  t[i++] =   4.0000;
  y[i] =     7.8700;  t[i++] =   5.0000;
  y[i] =     8.5100;  t[i++] =   6.0000;
  y[i] =    66.7000;  t[i++] =    .5000;
  y[i] =    59.2000;  t[i++] =    .7500;
  y[i] =    40.8000;  t[i++] =   1.0000;
  y[i] =    30.7000;  t[i++] =   1.5000;
  y[i] =    25.7000;  t[i++] =   2.0000;
  y[i] =    16.3000;  t[i++] =   2.5000;
  y[i] =    25.9900;  t[i++] =   2.0000;
  y[i] =    16.9500;  t[i++] =   2.5000;
  y[i] =    13.3500;  t[i++] =   3.0000;
  y[i] =     8.6200;  t[i++] =   4.0000;
  y[i] =     7.2000;  t[i++] =   5.0000;
  y[i] =     6.6400;  t[i++] =   6.0000;
  y[i] =    13.6900;  t[i++] =   3.0000;
  y[i] =    81.0000;  t[i++] =    .5000;
  y[i] =    64.5000;  t[i++] =    .7500;
  y[i] =    35.5000;  t[i++] =   1.5000;
   y[i] =   13.3100;  t[i++] =   3.0000;
  y[i] =     4.8700;  t[i++] =   6.0000;
  y[i] =    12.9400;  t[i++] =   3.0000;
  y[i] =     5.0600;  t[i++] =   6.0000;
  y[i] =    15.1900;  t[i++] =   3.0000;
  y[i] =    14.6200;  t[i++] =   3.0000;
  y[i] =    15.6400;  t[i++] =   3.0000;
  y[i] =    25.5000;  t[i++] =   1.7500;
  y[i] =    25.9500;  t[i++] =   1.7500;
  y[i] =    81.7000;  t[i++] =    .5000;
  y[i] =    61.6000;  t[i++] =    .7500;
  y[i] =    29.8000;  t[i++] =   1.7500;
  y[i] =    29.8100;  t[i++] =   1.7500;
  y[i] =    17.1700;  t[i++] =   2.7500;
  y[i] =    10.3900;  t[i++] =   3.7500;
  y[i] =    28.4000;  t[i++] =   1.7500;
  y[i] =    28.6900;  t[i++] =   1.7500;
  y[i] =    81.3000;  t[i++] =    .5000;
  y[i] =    60.9000;  t[i++] =    .7500;
  y[i] =    16.6500;  t[i++] =   2.7500;
  y[i] =    10.0500;  t[i++] =   3.7500;
  y[i] =    28.9000;  t[i++] =   1.7500;
  y[i] =    28.9500;  t[i++] =   1.7500;

  if (i != N_terms) throw std::runtime_error("Number of data values does not equal N_terms!");
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void partition_work(int j, int N_procs, int* start_index, int* stop_index) {
  // start_index and stop_index are 0-based (not 1-based).
  // Each given proc should process elements through stop_index inclusive, rather than stopping after element stop_index-1.

  *start_index = (int)std::floor(N_terms*(j+0.0d+0)/N_procs);
  *stop_index  = (int)std::floor(N_terms*(j+1.0d+0)/N_procs) - 1;
}
