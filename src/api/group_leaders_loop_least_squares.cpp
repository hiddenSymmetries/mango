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
#include "Least_squares_solver.hpp"

void mango::Least_squares_solver::group_leaders_loop() {
  // This method overrides mango::Solver::group_leaders_loop().

  double* state_vector = new double[N_parameters];
  double* base_case_residual = new double[N_terms];
  double* Jacobian = new double[N_parameters*N_terms];

  int data;

  bool keep_going = true;
  while (keep_going) {
    // Wait for proc 0 to send us a message that we should start.
    MPI_Bcast(&data,1,MPI_INT,0,mpi_partition->get_comm_group_leaders());
    if (data < 0) {
      if (verbose > 0) std::cout << "proc " << mpi_partition->get_rank_world() << 
			 " (a group leader) is exiting." << std::endl;
      keep_going = false;
    } else {
      if (verbose > 0) std::cout << "proc " << mpi_partition->get_rank_world() << 
			 " (a group leader) is starting finite-difference Jacobian calculation." << std::endl;
      finite_difference_Jacobian(state_vector, base_case_residual, Jacobian);
    }
  }

  delete[] state_vector;
  delete[] base_case_residual;
  delete[] Jacobian;
  
}
