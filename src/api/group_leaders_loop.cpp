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
