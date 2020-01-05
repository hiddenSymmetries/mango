#include<iostream>
#include<stdexcept>
#include "mango.hpp"

#define mango_interface_string_length 256
 
// C interfaces to C++ subroutines 
// See https://modelingguru.nasa.gov/docs/DOC-2642
// and http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects

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
    if (false) {
      std::cout << "interface.cpp subroutine mango_problem_create: objective_function=" << (long int)objective_function << std::endl;

      std::cout << "From interface.cpp, N_parameters=" << *N_parameters;
      for (int j=0; j<*N_parameters; j++) {
	std::cout << ", state_vector["<<j<<"]=" << state_vector[j];
      }
      std::cout << std::endl << "dummy=" << *dummy;
    }
    /*
    std::cout << "  About to call objective function from interface.cpp\n";
    double f;
    int failed_temp;
    objective_function(N_parameters, state_vector, &f, &failed_temp, this);
    std::cout << "Value of objective function: " << f << "\n";
    */
    return new mango::problem(*N_parameters, state_vector, objective_function, 0, NULL);
  }

  mango::problem *mango_problem_create_least_squares(int* N_parameters, double* state_vector, int* N_terms, double* targets, double* sigmas, 
						     double* best_residual_function, mango::residual_function_type residual_function) {
    return new mango::problem(*N_parameters, state_vector, *N_terms, targets, sigmas, best_residual_function, residual_function, 0, NULL);
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

  /* For converting communicators between Fortran and C, see
     https://www.mcs.anl.gov/research/projects/mpi/mpi-standard/mpi-report-2.0/node59.htm */
  void mango_mpi_init(mango::problem *This, MPI_Fint *comm) {
    This->mpi_init(MPI_Comm_f2c(*comm));
  }

  void mango_mpi_partition_set_custom(mango::problem *This, MPI_Fint *comm_world, MPI_Fint *comm_group_leaders, MPI_Fint *comm_worker_groups) {
    This->mpi_partition.set_custom(MPI_Comm_f2c(*comm_world), MPI_Comm_f2c(*comm_group_leaders), MPI_Comm_f2c(*comm_worker_groups));
  }

  double mango_optimize(mango::problem *This) {
    return This->optimize();
  }

  int mango_get_mpi_rank_world(mango::problem *This) {
    return This->mpi_partition.get_rank_world();
  }

  int mango_get_mpi_rank_worker_groups(mango::problem *This) {
    return This->mpi_partition.get_rank_worker_groups();
  }

  int mango_get_mpi_rank_group_leaders(mango::problem *This) {
    return This->mpi_partition.get_rank_group_leaders();
  }

  int mango_get_N_procs_world(mango::problem *This) {
    return This->mpi_partition.get_N_procs_world();
  }

  int mango_get_N_procs_worker_groups(mango::problem *This) {
    return This->mpi_partition.get_N_procs_worker_groups();
  }

  int mango_get_N_procs_group_leaders(mango::problem *This) {
    return This->mpi_partition.get_N_procs_group_leaders();
  }

  int mango_get_proc0_world(mango::problem *This) {
    return This->mpi_partition.get_proc0_world() ? 1 : 0;
  }

  int mango_get_proc0_worker_groups(mango::problem *This) {
    return This->mpi_partition.get_proc0_worker_groups() ? 1 : 0;
  }

  int mango_get_mpi_comm_world(mango::problem *This) {
    return (int) MPI_Comm_c2f(This->mpi_partition.get_comm_world());
  }

  int mango_get_mpi_comm_worker_groups(mango::problem *This) {
    return (int) MPI_Comm_c2f(This->mpi_partition.get_comm_worker_groups());
  }

  int mango_get_mpi_comm_group_leaders(mango::problem *This) {
    return (int) MPI_Comm_c2f(This->mpi_partition.get_comm_group_leaders());
  }

  int mango_get_N_parameters(mango::problem *This) {
    return (int) This->get_N_parameters();
  }

  int mango_get_N_terms(mango::problem *This) {
    return (int) This->get_N_terms();
  }

  int mango_get_worker_group(mango::problem *This) {
    return (int) This->mpi_partition.get_worker_group();
  }

  int mango_get_best_function_evaluation(mango::problem *This) {
    return (int) This->get_best_function_evaluation();
  }

  int mango_get_function_evaluations(mango::problem *This) {
    return (int) This->get_function_evaluations();
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
      throw std::runtime_error("Error in interface.cpp mango_set_centered_differences");
    }
  }

  int mango_does_algorithm_exist(char algorithm_name[mango_interface_string_length]) {
    bool return_bool = mango::does_algorithm_exist(algorithm_name);
    // Convert bool to integer:
    return(return_bool ? 1 : 0);
  }

  void mango_set_finite_difference_step_size(mango::problem *This, double* step) {
    This->finite_difference_step_size = *step;
  }

  void mango_set_bound_constraints(mango::problem *This, double* lower_bounds, double* upper_bounds) {
    This->set_bound_constraints(lower_bounds, upper_bounds);
  }

  void mango_set_verbose(mango::problem *This, int* verbose) {
    This->verbose = *verbose;
  }

  void mango_set_print_residuals_in_output_file(mango::problem *This, int* print_residuals_in_output_file_int) {
    if (*print_residuals_in_output_file_int==1) {
      This->print_residuals_in_output_file = true;
    } else if (*print_residuals_in_output_file_int==0) {
      This->print_residuals_in_output_file = false;
    } else {
      throw std::runtime_error("Error in interface.cpp mango_set_print_residuals_in_output_file");
    }
  }

  void mango_set_user_data(mango::problem *This, void* user_data) {
    This->user_data = user_data;
  }

  void mango_stop_workers(mango::problem *This) {
    This->mpi_partition.stop_workers();
  }

  void mango_mobilize_workers(mango::problem *This) {
    This->mpi_partition.mobilize_workers();
  }

  int mango_continue_worker_loop(mango::problem *This) {
    int return_bool = This->mpi_partition.continue_worker_loop();
    return (return_bool ? 1 : 0);
  }

  void mango_mpi_partition_write(mango::problem *This, char filename[mango_interface_string_length]) {
    This->mpi_partition.write(filename);
  }

}
