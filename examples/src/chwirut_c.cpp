// This example is based on the PETSc example src/tao/leastsquares/examples/tutorials/chwirut*
// This example demonstrates several things:
// * Passing a data structure to the objective/residual function through the user_data field.
// * Distributing work among workers within each worker group.

#define verbose_level 0

#include<iostream>
#include<iomanip>
#include<mpi.h>
#include<stdlib.h>
#include<stdexcept>
#include<cmath>
#include "mango.hpp"

#define N_terms 214

typedef struct {
  double y;
  double t;
} datapoint;

void init_data(datapoint*);

void partition_work(int, int, int*, int*);

void do_work(int, double*, double*, int, int, datapoint*);

void residual_function(int*, const double*, int*, double*, int*, mango::problem*, void*);

void worker(mango::problem*, datapoint*);

int main(int argc, char *argv[]) {
  int ierr;

  if (verbose_level > 0) std::cout << "Hello world from chwirut_c.\n";

  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0) {
    std::cout << "\nError in MPI_Init.\n";
    exit(1);
  }

  datapoint yt_data[N_terms];

  init_data(yt_data);

  double state_vector[3] = {0.15, 0.008, 0.01};
  double targets[N_terms];
  double sigmas[N_terms];
  double best_residual_function[N_terms];
  for (int j=0; j<N_terms; j++) {
    targets[j] = 0.0;
    sigmas[j]  = 1.0;
  }
  mango::problem myprob(3, state_vector, N_terms, targets, sigmas, best_residual_function, &residual_function, argc, argv);

  //  myprob.set_algorithm(mango::PETSC_POUNDERS);
  // myprob.set_algorithm("nlopt_ln_neldermead");
  myprob.verbose = verbose_level;
  myprob.read_input_file("../input/mango_in.chwirut_c");
  myprob.output_filename = "../output/mango_out.chwirut_c";
  myprob.mpi_init(MPI_COMM_WORLD);
  // myprob.centered_differences = true;
  myprob.max_function_evaluations = 2000;
  myprob.print_residuals_in_output_file = false;
  myprob.user_data = yt_data; // This passes the (y,t) data to the residual function

  double best_objective_function;
  if (myprob.mpi_partition.get_proc0_worker_groups()) {
    best_objective_function = myprob.optimize();
    myprob.mpi_partition.stop_workers();
  } else {
    worker(&myprob, yt_data);
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

void do_work(int N_parameters, double* x, double* f, int start_index, int stop_index, datapoint* data) {
  for (int j=start_index; j <= stop_index; j++) {
    f[j] = data[j].y - std::exp(-x[0] * data[j].t) / (x[1] + x[2] * data[j].t);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void residual_function(int* N_parameters, const double* x, int* N_terms_copy, double* f, int* failed, mango::problem* this_problem, void* void_user_data) {
  int j, start_index, stop_index;
  if (verbose_level > 0) std::cout << "C residual function called with N="<< *N_parameters << "\n";

  datapoint* yt_data = (datapoint*)void_user_data;

  MPI_Status mpi_status;
  MPI_Comm comm_worker_groups = this_problem->mpi_partition.get_comm_worker_groups();
  int N_procs_worker_groups = this_problem->mpi_partition.get_N_procs_worker_groups();

  // Mobilize the workers in the group with this group leader:
  this_problem->mpi_partition.mobilize_workers();

  // Send the state vector to all the other procs:
  int N = *N_parameters;
  MPI_Bcast((double*)x, N, MPI_DOUBLE, 0, comm_worker_groups);

  // Compute the residual terms on this proc, or receive the terms from the worker procs.
  for (j=0; j < N_procs_worker_groups; j++) {
    partition_work(j, N_procs_worker_groups, &start_index, &stop_index);
    if (j==0) {
      do_work(*N_parameters, (double*)x, f, start_index, stop_index, yt_data);
    } else {
      MPI_Recv(&f[start_index], stop_index - start_index + 1, MPI_DOUBLE, j, j, comm_worker_groups, &mpi_status);
    }
  }

  *failed = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void worker(mango::problem* myprob, datapoint* yt_data) {
  MPI_Comm mpi_comm_worker_groups = myprob->mpi_partition.get_comm_worker_groups();
  int N_parameters = myprob->get_N_parameters();
  int mpi_rank_worker_groups = myprob->mpi_partition.get_rank_worker_groups();

  int start_index, stop_index;
  double x[3];
  double f[N_terms];

  partition_work(mpi_rank_worker_groups, myprob->mpi_partition.get_N_procs_worker_groups(), &start_index, &stop_index);

  while (myprob->mpi_partition.continue_worker_loop()) {
    if (verbose_level > 0) std::cout<< "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is processing indices " << start_index << " to " << stop_index << "\n";

    // Get the state vector
    MPI_Bcast(x, N_parameters, MPI_DOUBLE, 0, mpi_comm_worker_groups);
	
    do_work(N_parameters, x, f, start_index, stop_index, yt_data);

    // Send my terms of the residual back to the master proc.
    MPI_Send(&f[start_index], stop_index - start_index + 1, MPI_DOUBLE, 0, mpi_rank_worker_groups, mpi_comm_worker_groups);
  }
  if (verbose_level > 0) std::cout << "Proc " << std::setw(5) << myprob->mpi_partition.get_rank_world() << " is exiting.\n";
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

void init_data(datapoint* data) {
  int i = 0;
 
  data[i].y =   92.9000;   data[i++].t =  0.5000;
  data[i].y =    78.7000;  data[i++].t =   0.6250;
  data[i].y =    64.2000;  data[i++].t =   0.7500;
  data[i].y =    64.9000;  data[i++].t =   0.8750;
  data[i].y =    57.1000;  data[i++].t =   1.0000;
  data[i].y =    43.3000;  data[i++].t =   1.2500;
  data[i].y =    31.1000;   data[i++].t =  1.7500;
  data[i].y =    23.6000;   data[i++].t =  2.2500;
  data[i].y =    31.0500;   data[i++].t =  1.7500;
  data[i].y =    23.7750;   data[i++].t =  2.2500;
  data[i].y =    17.7375;   data[i++].t =  2.7500;
  data[i].y =    13.8000;   data[i++].t =  3.2500;
  data[i].y =    11.5875;   data[i++].t =  3.7500;
  data[i].y =     9.4125;   data[i++].t =  4.2500;
  data[i].y =     7.7250;   data[i++].t =  4.7500;
  data[i].y =     7.3500;   data[i++].t =  5.2500;
  data[i].y =     8.0250;   data[i++].t =  5.7500;
  data[i].y =    90.6000;   data[i++].t =  0.5000;
  data[i].y =    76.9000;   data[i++].t =  0.6250;
  data[i].y =    71.6000;   data[i++].t = 0.7500;
  data[i].y =    63.6000;   data[i++].t =  0.8750;
  data[i].y =    54.0000;   data[i++].t =  1.0000;
  data[i].y =    39.2000;   data[i++].t =  1.2500;
  data[i].y =    29.3000;   data[i++].t = 1.7500;
  data[i].y =    21.4000;   data[i++].t =  2.2500;
  data[i].y =    29.1750;   data[i++].t =  1.7500;
  data[i].y =    22.1250;   data[i++].t =  2.2500;
  data[i].y =    17.5125;   data[i++].t =  2.7500;
  data[i].y =    14.2500;   data[i++].t =  3.2500;
  data[i].y =     9.4500;   data[i++].t =  3.7500;
  data[i].y =     9.1500;   data[i++].t =  4.2500;
  data[i].y =     7.9125;   data[i++].t =  4.7500;
  data[i].y =     8.4750;   data[i++].t =  5.2500;
  data[i].y =     6.1125;   data[i++].t =  5.7500;
  data[i].y =    80.0000;   data[i++].t =  0.5000;
  data[i].y =    79.0000;   data[i++].t =  0.6250;
  data[i].y =    63.8000;   data[i++].t =  0.7500;
  data[i].y =    57.2000;   data[i++].t =  0.8750;
  data[i].y =    53.2000;   data[i++].t =  1.0000;
  data[i].y =   42.5000;   data[i++].t =  1.2500;
  data[i].y =   26.8000;   data[i++].t =  1.7500;
  data[i].y =    20.4000;   data[i++].t =  2.2500;
  data[i].y =    26.8500;  data[i++].t =   1.7500;
  data[i].y =    21.0000;  data[i++].t =   2.2500;
  data[i].y =    16.4625;  data[i++].t =   2.7500;
  data[i].y =    12.5250;  data[i++].t =   3.2500;
  data[i].y =    10.5375;  data[i++].t =   3.7500;
  data[i].y =     8.5875;  data[i++].t =   4.2500;
  data[i].y =     7.1250;  data[i++].t =   4.7500;
  data[i].y =     6.1125;  data[i++].t =   5.2500;
  data[i].y =     5.9625;  data[i++].t =   5.7500;
  data[i].y =    74.1000;  data[i++].t =   0.5000;
  data[i].y =    67.3000;  data[i++].t =   0.6250;
  data[i].y =    60.8000;  data[i++].t =   0.7500;
  data[i].y =    55.5000;  data[i++].t =   0.8750;
  data[i].y =    50.3000;  data[i++].t =   1.0000;
  data[i].y =    41.0000;  data[i++].t =   1.2500;
  data[i].y =    29.4000;  data[i++].t =   1.7500;
  data[i].y =    20.4000;  data[i++].t =   2.2500;
  data[i].y =    29.3625;  data[i++].t =   1.7500;
  data[i].y =    21.1500;  data[i++].t =   2.2500;
  data[i].y =    16.7625;  data[i++].t =   2.7500;
  data[i].y =    13.2000;  data[i++].t =   3.2500;
  data[i].y =    10.8750;  data[i++].t =   3.7500;
  data[i].y =     8.1750;  data[i++].t =   4.2500;
  data[i].y =     7.3500;  data[i++].t =   4.7500;
  data[i].y =     5.9625;  data[i++].t =  5.2500;
  data[i].y =     5.6250;  data[i++].t =   5.7500;
  data[i].y =    81.5000;  data[i++].t =    .5000;
  data[i].y =    62.4000;  data[i++].t =    .7500;
  data[i].y =    32.5000;  data[i++].t =   1.5000;
  data[i].y =    12.4100;  data[i++].t =   3.0000;
  data[i].y =    13.1200;  data[i++].t =   3.0000;
  data[i].y =    15.5600;  data[i++].t =   3.0000;
  data[i].y =     5.6300;  data[i++].t =   6.0000;
  data[i].y =    78.0000;   data[i++].t =   .5000;
  data[i].y =    59.9000;  data[i++].t =    .7500;
  data[i].y =    33.2000;  data[i++].t =   1.5000;
  data[i].y =    13.8400;  data[i++].t =   3.0000;
  data[i].y =    12.7500;  data[i++].t =   3.0000;
  data[i].y =    14.6200;  data[i++].t =   3.0000;
  data[i].y =     3.9400;  data[i++].t =   6.0000;
  data[i].y =    76.8000;  data[i++].t =    .5000;
  data[i].y =    61.0000;  data[i++].t =    .7500;
  data[i].y =    32.9000;  data[i++].t =   1.5000;
  data[i].y =   13.8700;   data[i++].t = 3.0000;
  data[i].y =    11.8100;  data[i++].t =   3.0000;
  data[i].y =    13.3100;  data[i++].t =   3.0000;
  data[i].y =     5.4400;  data[i++].t =   6.0000;
  data[i].y =    78.0000;  data[i++].t =    .5000;
  data[i].y =    63.5000;  data[i++].t =    .7500;
  data[i].y =    33.8000;  data[i++].t =   1.5000;
  data[i].y =    12.5600;  data[i++].t =   3.0000;
  data[i].y =     5.6300;  data[i++].t =   6.0000;
  data[i].y =    12.7500;  data[i++].t =   3.0000;
  data[i].y =    13.1200;  data[i++].t =   3.0000;
  data[i].y =     5.4400;  data[i++].t =   6.0000;
  data[i].y =    76.8000;  data[i++].t =    .5000;
  data[i].y =    60.0000;  data[i++].t =    .7500;
  data[i].y =    47.8000;  data[i++].t =   1.0000;
  data[i].y =    32.0000;  data[i++].t =   1.5000;
  data[i].y =    22.2000;  data[i++].t =   2.0000;
  data[i].y =    22.5700;  data[i++].t =   2.0000;
  data[i].y =    18.8200;  data[i++].t =   2.5000;
  data[i].y =    13.9500;  data[i++].t =   3.0000;
  data[i].y =    11.2500;  data[i++].t =   4.0000;
  data[i].y =     9.0000;  data[i++].t =   5.0000;
  data[i].y =     6.6700;  data[i++].t =   6.0000;
  data[i].y =    75.8000;  data[i++].t =    .5000;
  data[i].y =    62.0000;  data[i++].t =    .7500;
  data[i].y =    48.8000;  data[i++].t =   1.0000;
  data[i].y =    35.2000;  data[i++].t =   1.5000;
  data[i].y =    20.0000;  data[i++].t =   2.0000;
  data[i].y =    20.3200;  data[i++].t =   2.0000;
  data[i].y =    19.3100;  data[i++].t =   2.5000;
  data[i].y =    12.7500;  data[i++].t =   3.0000;
  data[i].y =    10.4200;  data[i++].t =   4.0000;
  data[i].y =     7.3100;  data[i++].t =   5.0000;
  data[i].y =     7.4200;  data[i++].t =   6.0000;
  data[i].y =    70.5000;  data[i++].t =    .5000;
  data[i].y =    59.5000;  data[i++].t =    .7500;
  data[i].y =    48.5000;  data[i++].t =   1.0000;
  data[i].y =    35.8000;  data[i++].t =   1.5000;
  data[i].y =    21.0000;  data[i++].t =   2.0000;
  data[i].y =    21.6700;  data[i++].t =   2.0000;
  data[i].y =    21.0000;  data[i++].t =   2.5000;
  data[i].y =    15.6400;  data[i++].t =   3.0000;
  data[i].y =     8.1700;  data[i++].t =   4.0000;
  data[i].y =     8.5500;  data[i++].t =   5.0000;
  data[i].y =    10.1200;  data[i++].t =   6.0000;
  data[i].y =    78.0000;  data[i++].t =    .5000;
  data[i].y =    66.0000;  data[i++].t =    .6250;
  data[i].y =    62.0000;  data[i++].t =    .7500;
  data[i].y =    58.0000;  data[i++].t =    .8750;
  data[i].y =    47.7000;  data[i++].t =   1.0000;
  data[i].y =    37.8000;  data[i++].t =   1.2500;
  data[i].y =    20.2000;  data[i++].t =   2.2500;
  data[i].y =    21.0700;  data[i++].t =   2.2500;
  data[i].y =    13.8700;  data[i++].t =   2.7500;
  data[i].y =     9.6700;  data[i++].t =   3.2500;
  data[i].y =     7.7600;  data[i++].t =   3.7500;
  data[i].y =    5.4400;   data[i++].t =  4.2500;
  data[i].y =    4.8700;   data[i++].t =  4.7500;
  data[i].y =     4.0100;  data[i++].t =   5.2500;
  data[i].y =     3.7500;  data[i++].t =   5.7500;
  data[i].y =    24.1900;  data[i++].t =   3.0000;
  data[i].y =    25.7600;  data[i++].t =   3.0000;
  data[i].y =    18.0700;  data[i++].t =   3.0000;
  data[i].y =    11.8100;  data[i++].t =   3.0000;
  data[i].y =    12.0700;  data[i++].t =   3.0000;
  data[i].y =    16.1200;  data[i++].t =   3.0000;
  data[i].y =    70.8000;  data[i++].t =    .5000;
  data[i].y =    54.7000;  data[i++].t =    .7500;
  data[i].y =    48.0000;  data[i++].t =   1.0000;
  data[i].y =    39.8000;  data[i++].t =   1.5000;
  data[i].y =    29.8000;  data[i++].t =   2.0000;
  data[i].y =    23.7000;  data[i++].t =   2.5000;
  data[i].y =    29.6200;  data[i++].t =   2.0000;
  data[i].y =    23.8100;  data[i++].t =   2.5000;
  data[i].y =    17.7000;  data[i++].t =   3.0000;
  data[i].y =    11.5500;  data[i++].t =   4.0000;
  data[i].y =    12.0700;  data[i++].t =   5.0000;
  data[i].y =     8.7400;  data[i++].t =   6.0000;
  data[i].y =    80.7000;  data[i++].t =    .5000;
  data[i].y =    61.3000;  data[i++].t =    .7500;
  data[i].y =    47.5000;  data[i++].t =   1.0000;
   data[i].y =   29.0000;  data[i++].t =   1.5000;
   data[i].y =   24.0000;  data[i++].t =   2.0000;
  data[i].y =    17.7000;  data[i++].t =   2.5000;
  data[i].y =    24.5600;  data[i++].t =   2.0000;
  data[i].y =    18.6700;  data[i++].t =   2.5000;
   data[i].y =   16.2400;  data[i++].t =   3.0000;
  data[i].y =     8.7400;  data[i++].t =   4.0000;
  data[i].y =     7.8700;  data[i++].t =   5.0000;
  data[i].y =     8.5100;  data[i++].t =   6.0000;
  data[i].y =    66.7000;  data[i++].t =    .5000;
  data[i].y =    59.2000;  data[i++].t =    .7500;
  data[i].y =    40.8000;  data[i++].t =   1.0000;
  data[i].y =    30.7000;  data[i++].t =   1.5000;
  data[i].y =    25.7000;  data[i++].t =   2.0000;
  data[i].y =    16.3000;  data[i++].t =   2.5000;
  data[i].y =    25.9900;  data[i++].t =   2.0000;
  data[i].y =    16.9500;  data[i++].t =   2.5000;
  data[i].y =    13.3500;  data[i++].t =   3.0000;
  data[i].y =     8.6200;  data[i++].t =   4.0000;
  data[i].y =     7.2000;  data[i++].t =   5.0000;
  data[i].y =     6.6400;  data[i++].t =   6.0000;
  data[i].y =    13.6900;  data[i++].t =   3.0000;
  data[i].y =    81.0000;  data[i++].t =    .5000;
  data[i].y =    64.5000;  data[i++].t =    .7500;
  data[i].y =    35.5000;  data[i++].t =   1.5000;
   data[i].y =   13.3100;  data[i++].t =   3.0000;
  data[i].y =     4.8700;  data[i++].t =   6.0000;
  data[i].y =    12.9400;  data[i++].t =   3.0000;
  data[i].y =     5.0600;  data[i++].t =   6.0000;
  data[i].y =    15.1900;  data[i++].t =   3.0000;
  data[i].y =    14.6200;  data[i++].t =   3.0000;
  data[i].y =    15.6400;  data[i++].t =   3.0000;
  data[i].y =    25.5000;  data[i++].t =   1.7500;
  data[i].y =    25.9500;  data[i++].t =   1.7500;
  data[i].y =    81.7000;  data[i++].t =    .5000;
  data[i].y =    61.6000;  data[i++].t =    .7500;
  data[i].y =    29.8000;  data[i++].t =   1.7500;
  data[i].y =    29.8100;  data[i++].t =   1.7500;
  data[i].y =    17.1700;  data[i++].t =   2.7500;
  data[i].y =    10.3900;  data[i++].t =   3.7500;
  data[i].y =    28.4000;  data[i++].t =   1.7500;
  data[i].y =    28.6900;  data[i++].t =   1.7500;
  data[i].y =    81.3000;  data[i++].t =    .5000;
  data[i].y =    60.9000;  data[i++].t =    .7500;
  data[i].y =    16.6500;  data[i++].t =   2.7500;
  data[i].y =    10.0500;  data[i++].t =   3.7500;
  data[i].y =    28.9000;  data[i++].t =   1.7500;
  data[i].y =    28.9500;  data[i++].t =   1.7500;

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
