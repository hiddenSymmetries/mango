#include<iostream>
#include<mpi.h>
#include "mango.hpp"

#define bold_line "****************************************************************************************\n"

void mango::problem::mpi_init(MPI_Comm mpi_comm_world_in) {
  int ierr;

  /* mpi_comm_world = mpi_comm_world_in; */
  ierr = MPI_Comm_dup(mpi_comm_world_in, &mpi_comm_world);

  if (ierr != 0) {
    std::cout << "\nError in mango::mpi_init.\n";
    exit(1);
  }

  ierr = MPI_Comm_size(mpi_comm_world, &N_procs_world);
  ierr = MPI_Comm_rank(mpi_comm_world, &mpi_rank_world);
  proc0_world = (mpi_rank_world == 0);

  if (proc0_world) std::cout << "Number of worker groups, before validation: " << N_worker_groups << "\n";

  /* Make sure all procs agree on certain variables that will be used here. */
  MPI_Bcast(&N_worker_groups, 1, MPI_INT, 0, mpi_comm_world);
  MPI_Bcast(&algorithm, 1, MPI_INT, 0, mpi_comm_world);
  load_algorithm_properties(); /* Now that all procs agree on algorithm, all procs will get the correct algorithm properties. */

  /* Ensure N_worker_groups is within the range [1, N_procs_world] */
  if (N_worker_groups > N_procs_world) N_worker_groups = N_procs_world;
  /* Negative or 0 value for N_worker_groups on input means treat each proc as a worker group */
  if (N_worker_groups < 1) N_worker_groups = N_procs_world;

  if (algorithm < 0) {
    std::cout << "\nAlgorithm cannot be negative.\n";
    exit(1);
  }
  if (algorithm >= NUM_ALGORITHMS) {
    std::cout << "\nAlgorithm is too large.\n";
    exit(1);
  }

  if (proc0_world) std::cout << "Algorithm chosen: " << algorithm_name << "\n";
  if (algorithm_uses_derivatives) { /* May want to change this, since HOPSPACK does not use derivatives but it does exploit concurrent function evaluations. */
    if (N_procs_world > 1 && N_worker_groups == 1 && proc0_world) {
      std::cout << bold_line;
      std::cout << bold_line;
      std::cout << "WARNING!!! You have chosen an algorithm that can exploit concurrent function evaluations\n";
      std::cout << "but you have set N_worker_groups=1. You probably want a larger value.\n";
      std::cout << bold_line;
      std::cout << bold_line;
    }
  } else {
    /* There is no point having >1 worker groups with these algorithms. */
    N_worker_groups = 1;
  }

  if (proc0_world) std::cout << "Number of worker groups, after validation: " << N_worker_groups << "\n";

  worker_group = (mpi_rank_world * N_worker_groups) / N_procs_world; /* Note integer division, so there is an implied floor() */

  /* color = worker_group, key = mpi_rank_world */

  ierr = MPI_Comm_split(mpi_comm_world, worker_group, mpi_rank_world, &mpi_comm_worker_groups);
  if (ierr != 0) {
    std::cout << "\nError 2 in mango::mpi_init.\n";
    exit(1);
  }
  MPI_Comm_rank(mpi_comm_worker_groups, &mpi_rank_worker_groups);
  MPI_Comm_size(mpi_comm_worker_groups, &N_procs_worker_groups);
  proc0_worker_groups = (mpi_rank_worker_groups == 0);

  /* Now set up the group_leaders communicator */
  int color;
  if (proc0_worker_groups) {
    color = 0;
  } else {
    color = MPI_UNDEFINED;
  }
  ierr = MPI_Comm_split(mpi_comm_world, color, mpi_rank_world, &mpi_comm_group_leaders);
  if (ierr != 0) {
    std::cout << "\nError 3 in mango::mpi_init.\n";
    exit(1);
  }
  if (proc0_worker_groups) {
    MPI_Comm_rank(mpi_comm_group_leaders, &mpi_rank_group_leaders);
    MPI_Comm_size(mpi_comm_group_leaders, &N_procs_group_leaders);
  } else {
    /* We are not allowed to query the rank from procs that are not members of mpi_comm_group_leaders. */
    mpi_rank_group_leaders = -1;
    N_procs_group_leaders = -1;
  }

  /* Print the processor assignments in order of their rank in mpi_comm_world. */
  const int buffer_length = 1000;
  char proc_assignments_string[buffer_length];
  MPI_Status status;
  sprintf(proc_assignments_string,"Proc%5d of%5d in mpi_comm_world is in worker group%5d, has rank%5d of%5d in mpi_comm_worker_groups, and has rank%5d of%5d in mpi_comm_group_leaders.\n",mpi_rank_world, N_procs_world, worker_group, mpi_rank_worker_groups, N_procs_worker_groups, mpi_rank_group_leaders, N_procs_group_leaders);
  int tag;
  MPI_Barrier(mpi_comm_world);
  if (proc0_world) {
    std::cout << proc_assignments_string;
    for (tag = 1; tag < N_procs_world; tag++) {
      MPI_Recv(proc_assignments_string, buffer_length, MPI_CHAR, tag, tag, mpi_comm_world, &status);
      std::cout << proc_assignments_string;
    }
  } else {
    tag = mpi_rank_world;
    MPI_Send(proc_assignments_string, buffer_length, MPI_CHAR, 0, tag, mpi_comm_world);
  }

}
