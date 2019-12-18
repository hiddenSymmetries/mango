#include<iostream>
#include<mpi.h>
#include<stdexcept>
#include "mango.hpp"

void mango::MPI_Partition::init(MPI_Comm mpi_comm_world_in) {
  int ierr;

  ierr = MPI_Comm_dup(mpi_comm_world_in, &comm_world);
  if (ierr != 0) throw std::runtime_error("Error 1 in mango::MPI_Partition::init.");

  ierr = MPI_Comm_size(comm_world, &N_procs_world);
  ierr = MPI_Comm_rank(comm_world, &rank_world);
  proc0_world = (rank_world == 0);

  // if (proc0_world) std::cout << "Number of worker groups, before validation: " << N_worker_groups << std::endl;

  // Make sure all procs agree on certain variables that will be used here.
  MPI_Bcast(&N_worker_groups, 1, MPI_INT, 0, comm_world);

  // Ensure N_worker_groups is within the range [1, N_procs_world]
  if (N_worker_groups > N_procs_world) N_worker_groups = N_procs_world;
  // Negative or 0 value for N_worker_groups on input means treat each proc as a worker group
  if (N_worker_groups < 1) N_worker_groups = N_procs_world;

  // if (proc0_world) std::cout << "Number of worker groups, after validation: " << N_worker_groups << std::endl;

  worker_group = (rank_world * N_worker_groups) / N_procs_world; // Note integer division, so there is an implied floor()

  // color = worker_group, key = rank_world

  ierr = MPI_Comm_split(comm_world, worker_group, rank_world, &comm_worker_groups);
  if (ierr != 0) throw std::runtime_error("Error 2 in mango::MPI_Partition::init.");
  MPI_Comm_rank(comm_worker_groups, &rank_worker_groups);
  MPI_Comm_size(comm_worker_groups, &N_procs_worker_groups);
  proc0_worker_groups = (rank_worker_groups == 0);

  // Now set up the group_leaders communicator
  int color;
  if (proc0_worker_groups) {
    color = 0;
  } else {
    color = MPI_UNDEFINED;
  }
  ierr = MPI_Comm_split(comm_world, color, rank_world, &comm_group_leaders);
  if (ierr != 0) throw std::runtime_error("Error 3 in mango::MPI_Partition::init.");

  if (proc0_worker_groups) {
    MPI_Comm_rank(comm_group_leaders, &rank_group_leaders);
    MPI_Comm_size(comm_group_leaders, &N_procs_group_leaders);
  } else {
    // We are not allowed to query the rank from procs that are not members of comm_group_leaders.
    rank_group_leaders = -1;
    N_procs_group_leaders = -1;
  }

  print();
  initialized = true;
}

