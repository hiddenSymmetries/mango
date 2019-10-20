#include<iostream>
#include "mango.hpp"

void mango::problem::group_leaders_least_squares_loop() {
  double* state_vector = new double[N_parameters];
  double* base_case_residual = new double[N_terms];
  double* Jacobian = new double[N_parameters*N_terms];

  int data;

  bool keep_going = true;
  while (keep_going) {
    /* Wait for proc 0 to send us a message that we should start. */
    MPI_Bcast(&data,1,MPI_INT,0,mpi_comm_group_leaders);
    if (data < 0) {
      std::cout << "proc " << mpi_rank_world << " (a group leader) is exiting.\n";
      keep_going = false;
    } else {
      std::cout << "proc " << mpi_rank_world << " (a group leader) is starting finite-difference Jacobian calculation.\n";
      finite_difference_Jacobian(state_vector, base_case_residual, Jacobian);
    }
  }

  delete[] state_vector;
  delete[] base_case_residual;
  delete[] Jacobian;
  
}
