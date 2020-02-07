// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <mpi.h>
#include "mango.hpp"

void mango::MPI_Partition::print() {

  if (verbose <= 0) return;

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


void mango::MPI_Partition::write(std::string filename) {
  const int N_data_items = 8;
  std::string columns[N_data_items] = {"rank_world","N_procs_world","worker_group","N_worker_groups","rank_worker_groups","N_procs_worker_groups","rank_group_leaders","N_procs_group_leaders"};
  int            data[N_data_items] = { rank_world , N_procs_world , worker_group , N_worker_groups , rank_worker_groups , N_procs_worker_groups , rank_group_leaders , N_procs_group_leaders };

  std::ofstream output_file;
  int j;

  if (proc0_world) {
    // Open the file
    output_file.open(filename.c_str());
    if (!output_file.is_open()) {
      std::cerr << "MPI_Partition output file: " << filename << std::endl;
      throw std::runtime_error("Error! Unable to open MPI_Partition output file.");
    }
    
    // Write the header line
    output_file << columns[0];
    for (j=1; j<N_data_items; j++) output_file << ", " << columns[j];
    output_file << std::endl;
  }

  MPI_Status status;
  int tag;
  MPI_Barrier(comm_world);
  // Each processor sends their data to proc0_world, and proc0_world writes the result to the file in order.
  if (proc0_world) {
    write_line(output_file, N_data_items, columns, data);
    for (tag = 1; tag < N_procs_world; tag++) {
      MPI_Recv(data, N_data_items, MPI_INT, tag, tag, comm_world, &status);
      write_line(output_file, N_data_items, columns, data);
    }
  } else {
    tag = rank_world;
    MPI_Send(data, N_data_items, MPI_INT, 0, tag, comm_world);
  }


  if (proc0_world) output_file.close();
}


void mango::MPI_Partition::write_line(std::ofstream& output_file, int N_data_items, std::string columns[], int data[]) {
  // This subroutine writes one line of the mango_mpi output file.
  output_file << std::setw(columns[0].length()) << data[0];
  for (int j=1; j<N_data_items; j++)  output_file << ", " << std::setw(columns[j].length()) << data[j];
  output_file << std::endl;
}
