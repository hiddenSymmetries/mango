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
#include "mango.hpp"
#include "Solver.hpp"

void mango::Solver::group_leaders_loop() {
  double* state_vector = new double[N_parameters];
  double* gradient = new double[N_parameters];

  int data;
  double dummy;

  if (verbose > 0) std::cout << "Proc " << mpi_partition->get_rank_world() << " in entering group_leaders_loop." << std::endl;

  bool keep_going = true;
  while (keep_going) {
    // Wait for proc 0 to send us a message.
    MPI_Bcast(&data,1,MPI_INT,0,mpi_partition->get_comm_group_leaders());
    if (data < 0) {
      if (verbose > 0) std::cout << "proc " << mpi_partition->get_rank_world() << " (a group leader) is exiting." << std::endl;
      keep_going = false;
    } else {
      if (verbose > 0) std::cout << "proc " << mpi_partition->get_rank_world() << " (a group leader) is starting finite-difference gradient calculation." << std::endl;
      finite_difference_gradient(state_vector, &dummy, gradient); 
    }
  }

  delete[] state_vector;
  delete[] gradient;
}
