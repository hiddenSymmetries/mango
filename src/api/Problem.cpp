#include <iostream>
#include <string>
#include <stdexcept>
#include <cassert>
#include "mango.hpp"
#include "Solver.hpp"

// Constructor for non-least-squares problems
mango::Problem::Problem(int N_parameters_in, double* state_vector_in, objective_function_type objective_function_in, int argc_in, char* argv_in[]) {
  solver = new Solver(this,N_parameters_in);
  solver->argc = argc_in;
  solver->argv = argv_in;
  solver->N_parameters = N_parameters_in;
  solver->objective_function = objective_function_in;
  solver->state_vector = state_vector_in;
}

// Destructor
mango::Problem::~Problem() {
  if (solver->verbose > 0) std::cout << "Mango problem is being destroyed." << std::endl;
  delete solver;
}

int mango::Problem::get_N_parameters() {
  return solver->N_parameters;
}

int mango::Problem::get_best_function_evaluation() {
  return solver->best_function_evaluation;
}

double* mango::Problem::get_state_vector() {
  return solver->state_vector;
}

int mango::Problem::get_function_evaluations() {
  return solver->function_evaluations;
}

void mango::Problem::set_bound_constraints(double* lb, double* ub) {
  solver->lower_bounds = lb;
  solver->upper_bounds = ub;
  solver->bound_constraints_set = true;
}

void mango::Problem::set_centered_differences(bool new_bool) {
  solver->centered_differences = new_bool;
}

void mango::Problem::set_finite_difference_step_size(double delta) {
  solver->finite_difference_step_size = delta;
}

void mango::Problem::set_max_function_evaluations(int n) {
  solver->max_function_evaluations = n;
}

void mango::Problem::set_verbose(int v) {
  solver->verbose = v;
}

void mango::Problem::set_output_filename(std::string filename) {
  solver->output_filename = filename;
}

void mango::Problem::set_user_data(void* user_data) {
  solver->user_data = user_data;
}

#define bold_line "****************************************************************************************"
void mango::Problem::mpi_init(MPI_Comm mpi_comm_world) {
  // This method basically just calls MPI_Partition::init, but first checks to see if the algorithm
  // chosen can support parallel function evaluations. If not, N_worker_groups is set to 1.

  if (solver->algorithm < 0) throw std::runtime_error("Error in mango::Problem::mpi_init. Algorithm cannot be negative.");
  if (solver->algorithm >= NUM_ALGORITHMS) throw std::runtime_error("Error in mango::Problem::mpi_init. Algorithm is too large.");

  mpi_partition.verbose = solver->verbose;

  if (algorithms[solver->algorithm].parallel) {
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
  // Delegate this work to Solver so we don't need to put "solver->" in front of all the variables, and so we can replace solver with derived classes.
  return solver->optimize(&mpi_partition);
}


void mango::Problem::set_relative_bound_constraints(double min_factor, double max_factor, double min_radius, bool preserve_sign) {
  if (min_factor < 0) throw std::runtime_error("mango::Problem::set_relative_bound_constraints: min_factor must be >= 0.");
  if (min_factor > 1) throw std::runtime_error("mango::Problem::set_relative_bound_constraints: min_factor must be <= 1.");

  if (max_factor < 1) throw std::runtime_error("mango::Problem::set_relative_bound_constraints: max_factor must be >= 1.");

  if (min_radius < 0) throw std::runtime_error("mango::Problem::set_relative_bound_constraints: min_radius must be >= 0.");

  if (! solver->bound_constraints_set) throw std::runtime_error("mango::Problem::set_relative_bound_constraints can only be called after bound constraints are set.");

  int j;
  double temp;
  if (preserve_sign) {
    // Approach in which the sign of the variable is preserved.
    for (j=0; j < solver->N_parameters; j++) {
      if (solver->state_vector[j] > 0) {
	// Initial value and both bounds are positive
	solver->lower_bounds[j] = min_factor * solver->state_vector[j];
	solver->upper_bounds[j] = max_factor * solver->state_vector[j];
	if (solver->upper_bounds[j] - solver->state_vector[j] < min_radius) solver->upper_bounds[j] = solver->state_vector[j] + min_radius;
	if (solver->state_vector[j] - solver->lower_bounds[j] < min_radius) solver->lower_bounds[j] = solver->state_vector[j] - min_radius;
	if (solver->lower_bounds[j] < 0) solver->lower_bounds[j] = 0;
      } else if (solver->state_vector[j] < 0) {
	// Initial value and both bounds are negative
	solver->lower_bounds[j] = max_factor * solver->state_vector[j];
	solver->upper_bounds[j] = min_factor * solver->state_vector[j];
	if (solver->upper_bounds[j] - solver->state_vector[j] < min_radius) solver->upper_bounds[j] = solver->state_vector[j] + min_radius;
	if (solver->state_vector[j] - solver->lower_bounds[j] < min_radius) solver->lower_bounds[j] = solver->state_vector[j] - min_radius;
	if (solver->upper_bounds[j] > 0) solver->upper_bounds[j] = 0;
      } else {
	// Initial value is 0, so there is a bound of each sign
	assert(solver->state_vector[j] == 0);
	solver->lower_bounds[j] = -min_radius;
	solver->upper_bounds[j] =  min_radius;
      }
      if (! (solver->upper_bounds[j] >= solver->lower_bounds[j])) std::cout << "ub:" << solver->upper_bounds[j] << " lb:" << solver->lower_bounds[j] << "\n";
      assert(solver->upper_bounds[j] >= solver->lower_bounds[j]);
    }
  } else {
    // Approach in which the sign of the variable is not preserved.
    for (j=0; j < solver->N_parameters; j++) {
      temp = max_factor * abs(solver->state_vector[j]);
      if (temp < min_radius) temp = min_radius;
      solver->lower_bounds[j] = -temp;
      solver->upper_bounds[j] =  temp;
      assert(solver->upper_bounds[j] >= solver->lower_bounds[j]);
    }
  }
}
