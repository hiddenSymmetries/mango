#include <iostream>
#include <string>
#include <stdexcept>
#include "mango.hpp"
#include "Problem_data.hpp"

// Constructor for non-least-squares problems
mango::Problem::Problem(int N_parameters_in, double* state_vector_in, objective_function_type objective_function_in, int argc_in, char* argv_in[]) {
  data = new Problem_data(this,N_parameters_in);
  data->argc = argc_in;
  data->argv = argv_in;
  data->N_parameters = N_parameters_in;
  data->objective_function = objective_function_in;
  data->state_vector = state_vector_in;
}

// Destructor
mango::Problem::~Problem() {
  if (data->verbose > 0) std::cout << "Mango problem is being destroyed." << std::endl;
  delete data;
}

int mango::Problem::get_N_parameters() {
  return data->N_parameters;
}

int mango::Problem::get_best_function_evaluation() {
  return data->best_function_evaluation;
}

double* mango::Problem::get_state_vector() {
  return data->state_vector;
}

int mango::Problem::get_function_evaluations() {
  return data->function_evaluations;
}

void mango::Problem::set_bound_constraints(double* lb, double* ub) {
  data->lower_bounds = lb;
  data->upper_bounds = ub;
  data->bound_constraints_set = true;
}

void mango::Problem::set_centered_differences(bool new_bool) {
  data->centered_differences = new_bool;
}

void mango::Problem::set_finite_difference_step_size(double delta) {
  data->finite_difference_step_size = delta;
}

void mango::Problem::set_max_function_evaluations(int n) {
  data->max_function_evaluations = n;
}

void mango::Problem::set_verbose(int v) {
  data->verbose = v;
}

void mango::Problem::set_output_filename(std::string filename) {
  data->output_filename = filename;
}

void mango::Problem::set_user_data(void* user_data) {
  data->user_data = user_data;
}

#define bold_line "****************************************************************************************"
void mango::Problem::mpi_init(MPI_Comm mpi_comm_world) {
  // This method basically just calls MPI_Partition::init, but first checks to see if the algorithm
  // chosen can support parallel function evaluations. If not, N_worker_groups is set to 1.

  if (data->algorithm < 0) throw std::runtime_error("Error in mango::Problem::mpi_init. Algorithm cannot be negative.");
  if (data->algorithm >= NUM_ALGORITHMS) throw std::runtime_error("Error in mango::Problem::mpi_init. Algorithm is too large.");

  mpi_partition.verbose = data->verbose;

  if (algorithms[data->algorithm].parallel) {
    int mpi_rank_world, N_procs_world;
    MPI_Comm_size(mpi_comm_world, &N_procs_world);
    MPI_Comm_rank(mpi_comm_world, &mpi_rank_world);

    if ((N_procs_world > 1) && (mpi_partition.get_N_worker_groups() == 1) && (mpi_rank_world==0)) {
      std::cerr << bold_line << std::endl;
      std::cerr << "WARNING!!! You have chosen an algorithm that can exploit concurrent function evaluations" << std::endl;
      std::cerr << "but you have set N_worker_groups=1. You probably want a larger value." << std::endl;
      std::cerr << bold_line << std::endl;
    }
  } else {
    // There is no point having >1 worker groups with these algorithms.
    mpi_partition.set_N_worker_groups(1);
  }  

  mpi_partition.init(mpi_comm_world);
}


double mango::Problem::optimize() {
  // Delegate this work to Problem_data so we don't need to put "data->" in front of all the variables.
  return data->optimize(&mpi_partition);
}
