#include<iostream>
#include<mpi.h>
#include<stdexcept>
#include "mango.hpp"

void mango::MPI_Partition::set_custom(MPI_Comm comm_world_in, MPI_Comm comm_group_leaders_in, MPI_Comm comm_worker_groups_in) {
  // All processes should call this subroutine!

  int ierr;
  
  comm_world = comm_world_in;
  comm_group_leaders = comm_group_leaders_in;
  comm_worker_groups = comm_worker_groups_in;

  // Get the information about the world communicator.
  ierr = MPI_Comm_size(comm_world, &N_procs_world);
  if (ierr != 0) throw std::runtime_error("Error with the supplied mpi_comm_world in mango::MPI_Partition::set_custom.");
  ierr = MPI_Comm_rank(comm_world, &rank_world);
  proc0_world = (rank_world == 0);

  // Get the information about the worker_groups communicator.
  MPI_Comm_rank(comm_worker_groups, &rank_worker_groups);
  if (ierr != 0) throw std::runtime_error("Error with the supplied mpi_comm_worker_groups in mango::MPI_Partition::set_custom.");
  MPI_Comm_size(comm_worker_groups, &N_procs_worker_groups);
  proc0_worker_groups = (rank_worker_groups == 0);

  // Get the information about the group_leaders communicator.
  if (proc0_worker_groups) {
    MPI_Comm_rank(comm_group_leaders, &rank_group_leaders);
    if (ierr != 0) throw std::runtime_error("Error with the supplied mpi_comm_group_leaders in mango::MPI_Partition::set_custom.");
    MPI_Comm_size(comm_group_leaders, &N_procs_group_leaders);
  } else {
    // We are not allowed to query the rank from procs that are not members of comm_group_leaders.
    rank_group_leaders = -1;
    N_procs_group_leaders = -1;
  }

  // Determine how many worker groups there are.
  N_worker_groups = (proc0_worker_groups ? 1 : 0);
  MPI_Allreduce(MPI_IN_PLACE, &N_worker_groups, 1, MPI_INT, MPI_SUM, comm_world);

  // Determine which worker group each processor is in.
  worker_group = rank_group_leaders; // We'll say the worker group corresponds to the rank of the corresponding master proc in comm_group_leaders.
  MPI_Bcast(&worker_group, 1, MPI_INT, 0, comm_worker_groups);

  print();
  initialized = true;
}
