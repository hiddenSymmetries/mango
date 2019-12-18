#include<iostream>
#include<string>
#include<stdexcept>
#include "mango.hpp"

// Constructor for non-least-squares problems
mango::problem::problem(int N_parameters_in, double* state_vector_in, objective_function_type objective_function_in, int argc_in, char* argv_in[]) {
  defaults();
  argc = argc_in;
  argv = argv_in;
  N_parameters = N_parameters_in;
  N_terms = -1;
  best_state_vector = new double[N_parameters];
  objective_function = objective_function_in;
  residual_function = NULL;
  least_squares = false;
  state_vector = state_vector_in;
  targets = NULL;
  sigmas = NULL;
}

// Constructor for least-squares problems
mango::problem::problem(int N_parameters_in, double* state_vector_in, int N_terms_in, double* targets_in, double* sigmas_in, 
			double* best_residual_function_in, residual_function_type residual_function_in, int argc_in, char* argv_in[]) {
  defaults();
  argc = argc_in;
  argv = argv_in;
  N_parameters = N_parameters_in;
  N_terms = N_terms_in;
  best_state_vector = new double[N_parameters];
  residuals = new double[N_terms];
  objective_function = NULL;
  residual_function = residual_function_in;
  least_squares = true;
  state_vector = state_vector_in;
  targets = targets_in;
  sigmas = sigmas_in;
  best_residual_function = best_residual_function_in;
}

// Destructor
mango::problem::~problem() {
  if (verbose > 0) std::cout << "Mango problem is being destroyed." << std::endl;
  delete[] best_state_vector;
  if (least_squares) delete[] residuals;
}

void mango::problem::defaults() {
  verbose = 0;
  mpi_partition.set_N_worker_groups(-1);
  set_algorithm(PETSC_NM);
  centered_differences = false;
  finite_difference_step_size = 1.0e-7;
  output_filename = "mango_out.";
  max_function_evaluations = 10000;
  best_function_evaluation = -1;
  bound_constraints_set = false;
  print_residuals_in_output_file = true;
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

int mango::problem::get_best_function_evaluation() {
  return best_function_evaluation;
}

int mango::problem::get_function_evaluations() {
  return function_evaluations;
}

void mango::problem::set_bound_constraints(double* lb, double* ub) {
  lower_bounds = lb;
  upper_bounds = ub;
  bound_constraints_set = true;
}

#define bold_line "****************************************************************************************"
void mango::problem::mpi_init(MPI_Comm mpi_comm_world) {
  // This method basically just calls MPI_Partition::init, but first checks to see if the algorithm
  // chosen can support parallel function evaluations. If not, N_worker_groups is set to 1.

  if (algorithm < 0) throw std::runtime_error("Error in mango::problem::mpi_init. Algorithm cannot be negative.");
  if (algorithm >= NUM_ALGORITHMS) throw std::runtime_error("Error in mango::problem::mpi_init. Algorithm is too large.");

  mpi_partition.verbose = verbose;

  if (algorithms[algorithm].parallel) {
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
