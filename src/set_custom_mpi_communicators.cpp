#include<iostream>
#include<mpi.h>
#include "mango.hpp"

#define bold_line "****************************************************************************************\n"

void mango::problem::set_custom_mpi_communicators(MPI_Comm mpi_comm_world_in, MPI_Comm mpi_comm_group_leaders_in, MPI_Comm mpi_comm_worker_groups_in) {
  /* All processes should call this subroutine! */

  int ierr;
  
  mpi_comm_world = mpi_comm_world_in;
  mpi_comm_group_leaders = mpi_comm_group_leaders_in;
  mpi_comm_worker_groups = mpi_comm_worker_groups_in;

  /* Get the information about the world communicator. */
  ierr = MPI_Comm_size(mpi_comm_world, &N_procs_world);
  if (ierr != 0) {
    std::cout << "\nError with the supplied mpi_comm_world in mango::problem::set_custom_mpi_communicators.\n";
    exit(1);
  }
  ierr = MPI_Comm_rank(mpi_comm_world, &mpi_rank_world);
  proc0_world = (mpi_rank_world == 0);

  /* Get the information about the worker_groups communicator. */
  MPI_Comm_rank(mpi_comm_worker_groups, &mpi_rank_worker_groups);
  if (ierr != 0) {
    std::cout << "\nError with the supplied mpi_comm_worker_groups in mango::problem::set_custom_mpi_communicators.\n";
    exit(1);
  }
  MPI_Comm_size(mpi_comm_worker_groups, &N_procs_worker_groups);
  proc0_worker_groups = (mpi_rank_worker_groups == 0);

  /* Get the information about the group_leaders communicator. */
  if (proc0_worker_groups) {
    MPI_Comm_rank(mpi_comm_group_leaders, &mpi_rank_group_leaders);
    if (ierr != 0) {
      std::cout << "\nError with the supplied mpi_comm_group_leaders in mango::problem::set_custom_mpi_communicators.\n";
      exit(1);
    }
    MPI_Comm_size(mpi_comm_group_leaders, &N_procs_group_leaders);
  } else {
    /* We are not allowed to query the rank from procs that are not members of mpi_comm_group_leaders. */
    mpi_rank_group_leaders = -1;
    N_procs_group_leaders = -1;
  }

  /* Determine how many worker groups there are. */
  N_worker_groups = (proc0_worker_groups ? 1 : 0);
  if (proc0_world) {
    MPI_Reduce(MPI_IN_PLACE,     &N_worker_groups, 1, MPI_INT, MPI_SUM, 0, mpi_comm_world);
  } else {
    MPI_Reduce(&N_worker_groups, &N_worker_groups, 1, MPI_INT, MPI_SUM, 0, mpi_comm_world);
  }

  /* Determine which worker group each processor is in. */
  worker_group = mpi_rank_group_leaders; /* We'll say the worker group corresponds to the rank of the corresponding master proc in mpi_comm_group_leaders. */
  MPI_Bcast(&worker_group, 1, MPI_INT, 0, mpi_comm_worker_groups);

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
