#include<iostream>
#include<string>
#include "mango.hpp"

/* Constructor for non-least-squares problems */
mango::problem::problem(int N_parameters_in, double* state_vector_in, objective_function_type objective_function_in, int argc_in, char* argv_in[]) {
  defaults();
  argc = argc_in;
  argv = argv_in;
  N_parameters = N_parameters_in;
  N_terms = -1;
  objective_function = objective_function_in;
  residual_function = NULL;
  least_squares = false;
  state_vector = state_vector_in;
  targets = NULL;
  sigmas = NULL;
}

/* Constructor for least-squares problems */
mango::problem::problem(int N_parameters_in, double* state_vector_in, int N_terms_in, double* targets_in, double* sigmas_in, residual_function_type residual_function_in, int argc_in, char* argv_in[]) {
  defaults();
  argc = argc_in;
  argv = argv_in;
  N_parameters = N_parameters_in;
  N_terms = N_terms_in;
  objective_function = NULL;
  residual_function = residual_function_in;
  least_squares = true;
  state_vector = state_vector_in;
  targets = targets_in;
  sigmas = sigmas_in;
}

/* Destructor */
mango::problem::~problem() {
  std::cout << "Mango problem is being destroyed.\n";
}

void mango::problem::defaults() {
  N_worker_groups = -1;
  set_algorithm(PETSC_NM);
  centered_differences = false;
  finite_difference_step_size = 1.0e-7;
  output_filename = "mango_out.";
  max_function_evaluations = 10000;
}


void mango::problem::set_output_filename(std::string filename) {
  output_filename = filename;
}

bool mango::problem::is_least_squares() {
  return least_squares;
}

int mango::problem::get_N_parameters() {
  return N_parameters;
}

int mango::problem::get_N_terms() {
  return N_terms;
}

MPI_Comm mango::problem::get_mpi_comm_world() {
  return mpi_comm_world;
}

MPI_Comm mango::problem::get_mpi_comm_worker_groups() {
  return mpi_comm_worker_groups;
}

MPI_Comm mango::problem::get_mpi_comm_group_leaders() {
  return mpi_comm_group_leaders;
}

bool mango::problem::is_proc0_world() {
  return proc0_world;
}

bool mango::problem::is_proc0_worker_groups() {
  return proc0_worker_groups;
}

int mango::problem::get_mpi_rank_world() {
  return mpi_rank_world;
}

int mango::problem::get_mpi_rank_worker_groups() {
  return mpi_rank_worker_groups;
}

int mango::problem::get_mpi_rank_group_leaders() {
  return mpi_rank_group_leaders;
}

int mango::problem::get_N_procs_world() {
  return N_procs_world;
}

int mango::problem::get_N_procs_worker_groups() {
  return N_procs_worker_groups;
}

int mango::problem::get_N_procs_group_leaders() {
  return N_procs_group_leaders;
}

int mango::problem::get_worker_group() {
  return worker_group;
}

