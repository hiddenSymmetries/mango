#include <iostream>
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Least_squares_data.hpp"

void mango::Least_squares_data::group_leaders_least_squares_loop() {
  double* state_vector = new double[problem_data->N_parameters];
  double* base_case_residual = new double[N_terms];
  double* Jacobian = new double[problem_data->N_parameters*N_terms];

  int data;

  bool keep_going = true;
  while (keep_going) {
    // Wait for proc 0 to send us a message that we should start.
    MPI_Bcast(&data,1,MPI_INT,0,problem_data->mpi_partition->get_comm_group_leaders());
    if (data < 0) {
      if (problem_data->verbose > 0) std::cout << "proc " << problem_data->mpi_partition->get_rank_world() << 
				       " (a group leader) is exiting." << std::endl;
      keep_going = false;
    } else {
      if (problem_data->verbose > 0) std::cout << "proc " << problem_data->mpi_partition->get_rank_world() << 
				       " (a group leader) is starting finite-difference Jacobian calculation." << std::endl;
      finite_difference_Jacobian(state_vector, base_case_residual, Jacobian);
    }
  }

  delete[] state_vector;
  delete[] base_case_residual;
  delete[] Jacobian;
  
}
