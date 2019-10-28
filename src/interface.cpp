#include<iostream>
#include "mango.hpp"

#define mango_interface_string_length 256

/* 
C interfaces to C++ subroutines 
See https://modelingguru.nasa.gov/docs/DOC-2642
and http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects
*/
extern "C" {
  /*  mango::problem *mango_problem_create() {
    return new mango::problem;
    } */
  /*
  mango::problem *mango_problem_create(int N_parameters) {
    return new mango::problem(N_parameters);
  }
  mango::problem *mango_problem_create_least_squares(int N_parameters, int N_terms) {
    return new mango::problem(N_parameters,N_terms);
    } */
  mango::problem *mango_problem_create(int* N_parameters, double* state_vector, int* dummy, mango::objective_function_type objective_function) {
    std::cout << "interface.cpp subroutine mango_problem_create: objective_function=" << (long int)objective_function << "\n";

    std::cout << "From interface.cpp, N_parameters=" << *N_parameters;
    for (int j=0; j<*N_parameters; j++) {
      std::cout << ", state_vector["<<j<<"]=" << state_vector[j];
    }
    std::cout << "\ndummy=" << *dummy;
    /*
    std::cout << "  About to call objective function from interface.cpp\n";
    double f;
    int failed_temp;
    objective_function(N_parameters, state_vector, &f, &failed_temp, this);
    std::cout << "Value of objective function: " << f << "\n";
    */
    return new mango::problem(*N_parameters, state_vector, objective_function, 0, NULL);
  }

  mango::problem *mango_problem_create_least_squares(int* N_parameters, double* state_vector, int* N_terms, double* targets, double* sigmas, mango::residual_function_type residual_function) {
    return new mango::problem(*N_parameters, state_vector, *N_terms, targets, sigmas, residual_function, 0, NULL);
  }

  void mango_problem_destroy(mango::problem *This) {
    delete This;
  }

  void mango_set_algorithm(mango::problem *This, mango::algorithm_type *algorithm) {
    This->set_algorithm(*algorithm);
  }

  void mango_set_algorithm_from_string(mango::problem *This, char algorithm_name[mango_interface_string_length]) {
    This->set_algorithm(algorithm_name);
  }

  void mango_read_input_file(mango::problem *This, char filename[mango_interface_string_length]) {
    This->read_input_file(filename);
  }

  void mango_set_output_filename(mango::problem *This, char filename[mango_interface_string_length]) {
    This->set_output_filename(filename);
  }

  void mango_mpi_init(mango::problem *This, MPI_Comm *comm) {
    This->mpi_init(*comm);
  }

  void mango_optimize(mango::problem *This) {
    This->optimize();
  }

  int mango_get_mpi_rank_world(mango::problem *This) {
    return This->get_mpi_rank_world();
  }

  int mango_get_mpi_rank_worker_groups(mango::problem *This) {
    return This->get_mpi_rank_worker_groups();
  }

  int mango_get_mpi_rank_group_leaders(mango::problem *This) {
    return This->get_mpi_rank_group_leaders();
  }

  int mango_get_N_procs_world(mango::problem *This) {
    return This->get_N_procs_world();
  }

  int mango_get_N_procs_worker_groups(mango::problem *This) {
    return This->get_N_procs_worker_groups();
  }

  int mango_get_N_procs_group_leaders(mango::problem *This) {
    return This->get_N_procs_group_leaders();
  }

  int mango_is_proc0_world(mango::problem *This) {
    return This->is_proc0_world() ? 1 : 0;
  }

  int mango_is_proc0_worker_groups(mango::problem *This) {
    return This->is_proc0_worker_groups() ? 1 : 0;
  }

  int mango_get_mpi_comm_world(mango::problem *This) {
    return (int) This->get_mpi_comm_world();
  }

  int mango_get_mpi_comm_worker_groups(mango::problem *This) {
    return (int) This->get_mpi_comm_worker_groups();
  }

  int mango_get_mpi_comm_group_leaders(mango::problem *This) {
    return (int) This->get_mpi_comm_group_leaders();
  }

  int mango_get_N_parameters(mango::problem *This) {
    return (int) This->get_N_parameters();
  }

  int mango_get_N_terms(mango::problem *This) {
    return (int) This->get_N_terms();
  }

  void mango_set_max_function_evaluations(mango::problem *This, int *N) {
    This->max_function_evaluations = *N;
  }

  void mango_set_centered_differences(mango::problem *This, int* centered_differences_int) {
    if (*centered_differences_int==1) {
      This->centered_differences = true;
    } else if (*centered_differences_int==0) {
      This->centered_differences = false;
    } else {
      std::cout << "Error in interface.cpp mango_set_centered_differences: centered_difference_int = " << centered_differences_int << "\n";
      exit(1);
    }
  }

}
