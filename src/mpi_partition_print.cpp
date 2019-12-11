#include<iostream>
#include<mpi.h>
#include "mango.hpp"

void mango::MPI_Partition::print() {

  if (!verbose) return;

  // Print the processor assignments in order of their rank in mpi_comm_world.
  const int buffer_length = 1000;
  char proc_assignments_string[buffer_length];
  MPI_Status status;
  sprintf(proc_assignments_string,"Proc%5d of%5d in comm_world is in worker group%5d, has rank%5d of%5d in comm_worker_groups, and has rank%5d of%5d in comm_group_leaders.\n",rank_world, N_procs_world, worker_group, rank_worker_groups, N_procs_worker_groups, rank_group_leaders, N_procs_group_leaders);
  int tag;
  MPI_Barrier(comm_world);
  if (proc0_world) {
    std::cout << proc_assignments_string;
    for (tag = 1; tag < N_procs_world; tag++) {
      MPI_Recv(proc_assignments_string, buffer_length, MPI_CHAR, tag, tag, comm_world, &status);
      std::cout << proc_assignments_string;
    }
  } else {
    tag = rank_world;
    MPI_Send(proc_assignments_string, buffer_length, MPI_CHAR, 0, tag, comm_world);
  }

}
