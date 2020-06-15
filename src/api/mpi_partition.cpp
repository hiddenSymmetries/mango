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
#include <string>
#include <stdexcept>
#include "mango.hpp"

// Constructor
mango::MPI_Partition::MPI_Partition() {
  N_worker_groups = -1;
  initialized = false;
  verbose = false;
}

// Destructor
mango::MPI_Partition::~MPI_Partition() {
}

void mango::MPI_Partition::verify_initialized() {
  if (!initialized) {
    throw std::runtime_error("Error! MPI_Partition get method was called before initialization.");
  }
}

MPI_Comm mango::MPI_Partition::get_comm_world() {
  verify_initialized();
  return comm_world;
}

MPI_Comm mango::MPI_Partition::get_comm_worker_groups() {
  verify_initialized();
  return comm_worker_groups;
}

MPI_Comm mango::MPI_Partition::get_comm_group_leaders() {
  verify_initialized();
  return comm_group_leaders;
}

bool mango::MPI_Partition::get_proc0_world() {
  verify_initialized();
  return proc0_world;
}

bool mango::MPI_Partition::get_proc0_worker_groups() {
  verify_initialized();
  return proc0_worker_groups;
}

int mango::MPI_Partition::get_rank_world() {
  verify_initialized();
  return rank_world;
}

int mango::MPI_Partition::get_rank_worker_groups() {
  verify_initialized();
  return rank_worker_groups;
}

int mango::MPI_Partition::get_rank_group_leaders() {
  verify_initialized();
  return rank_group_leaders;
}

int mango::MPI_Partition::get_N_procs_world() {
  verify_initialized();
  return N_procs_world;
}

int mango::MPI_Partition::get_N_procs_worker_groups() {
  verify_initialized();
  return N_procs_worker_groups;
}

int mango::MPI_Partition::get_N_procs_group_leaders() {
  verify_initialized();
  return N_procs_group_leaders;
}

int mango::MPI_Partition::get_worker_group() {
  verify_initialized();
  return worker_group;
}

int mango::MPI_Partition::get_N_worker_groups() {
  // Don't call verify_initialized() for this get method. problem::mpi_init fails otherwise, and there is no problem with querying N_worker_groups before initialization.
  return N_worker_groups;
}

void mango::MPI_Partition::set_N_worker_groups(int N_worker_groups_in) {
  if (initialized) throw std::runtime_error("Error! MPI_Partition::set_N_worker_groups called after initialization.");
  N_worker_groups = N_worker_groups_in;
}

void mango::MPI_Partition::stop_workers() {
  // This method should only be called from group leaders.
  if (!proc0_worker_groups) throw std::runtime_error("mango::MPI_Partition::stop_workers() should only be called from group leaders.");
  int data = -1; // Any negative value will do here.
  MPI_Bcast(&data, 1, MPI_INT, 0, comm_worker_groups);
}

void mango::MPI_Partition::mobilize_workers() {
  // This method should only be called from group leaders.
  if (!proc0_worker_groups) throw std::runtime_error("mango::MPI_Partition::mobilize_workers() should only be called from group leaders.");
  int data = 1; // Any nonnegative value will do here.
  std::cout << "Hello from mango::MPI_Partition::mobilize_workers" << std::endl;
  MPI_Bcast(&data, 1, MPI_INT, 0, comm_worker_groups);
  std::cout << "Bye from mango::MPI_Partition::mobilize_workers" << std::endl;
}

bool mango::MPI_Partition::continue_worker_loop() {
  // This method should NOT be called from group leaders.
  if (proc0_worker_groups) throw std::runtime_error("mango::MPI_Partition::continue_worker_loop() should not be called from group leaders.");
  int data;
  MPI_Bcast(&data, 1, MPI_INT, 0, comm_worker_groups);
  return (data >= 0);
}
