#include <iostream>
#include <iomanip>
#include "Least_squares_solver.hpp"
#include "Package_mango.hpp"
#include "Levenberg_marquardt.hpp"

#ifdef MANGO_EIGEN_AVAILABLE
#include <Eigen/Dense>
#endif

// If the next line is uncommented, the least-squares problem will be solved
// a second way, to make sure the 2 methods agree. This check adds some computational cost.
#define CHECK_LEAST_SQUARES_SOLUTION

#ifndef MANGO_EIGEN_AVAILABLE
// Eigen is NOT available.

mango::Levenberg_marquardt::Levenberg_marquardt(Least_squares_solver* solver_in) {
  throw std::runtime_error("ERROR: The algorithm mango_levenberg_marquardt was selected. This algorithm requires Eigen, but MANGO was built without Eigen.");
}

void mango::Levenberg_marquardt::solve() {}

#else
// The rest of this file is used when Eigen IS available.

//! Constructor
mango::Levenberg_marquardt::Levenberg_marquardt(Least_squares_solver* solver_in) 
  // Call constructors for members
  : state_vector(solver_in->state_vector, solver_in->N_parameters),
    targets(solver_in->targets, solver_in->N_terms),
    sigmas(solver_in->sigmas, solver_in->N_terms)
{
  solver = solver_in;

  // Initial value for the Levenberg-Marquardt parameter:
  central_lambda = 0.01;

  max_line_search_iterations = 4;

  // Define shorthand variable names:
  N_parameters = solver->N_parameters;
  N_terms = solver->N_terms;
  verbose = solver->verbose;
  N_line_search = solver->N_line_search;
  proc0_world = solver->mpi_partition->get_proc0_world();
  comm_group_leaders = solver->mpi_partition->get_comm_group_leaders();

  if (solver->verbose > 0) std::cout << "Hello from levenberg_marquardt. N_line_search=" << N_line_search << std::endl;

  if (N_line_search < 1) throw std::runtime_error("N_line_search must be >= 1.");

  // Set sizes for Eigen vectors and matrices:
  state_vector_tentative.resize(N_parameters);
  residuals.resize(N_terms);
  shifted_residuals.resize(N_terms);
  Jacobian.resize(N_terms,N_parameters);
  residuals_extended.resize(N_terms + N_parameters);
  Jacobian_extended.resize(N_terms + N_parameters, N_parameters);
  delta_x.resize(N_parameters);
  lambda_scan_residuals.resize(N_terms, N_line_search);
  lambda_scan_state_vectors.resize(N_parameters, N_line_search);
  lambdas.resize(N_line_search);
  lambda_scan_objective_functions.resize(N_line_search);
#ifdef CHECK_LEAST_SQUARES_SOLUTION
  delta_x_direct.resize(N_parameters);
  alpha.resize(N_parameters, N_parameters);
  alpha_prime.resize(N_parameters, N_parameters);
  beta.resize(N_parameters);
#endif

  residuals_extended.bottomRows(N_parameters) = Eigen::VectorXd::Zero(N_parameters);
  Jacobian_extended.bottomRows(N_parameters) = Eigen::MatrixXd::Zero(N_parameters,N_parameters);

  failed = false;
  lambda_increase_factor = compute_lambda_increase_factor(N_line_search);
  normalized_lambda_grid = new double[N_line_search];
  compute_lambda_grid(N_line_search, lambda_increase_factor, normalized_lambda_grid);
  if (verbose>0 && proc0_world) {
    std::cout << "lambda_increase_factor: " << lambda_increase_factor << std::endl;
    std::cout << "normalized_lambda_grid:";
    for (j_lambda_grid=0; j_lambda_grid<N_line_search; j_lambda_grid++) std::cout << " " << normalized_lambda_grid[j_lambda_grid];
    std::cout << std::endl;
  }

  // Open output file:
  if (proc0_world) {
    std::string filename = solver->output_filename + "_levenberg_marquardt";
    lambda_file.open(filename.c_str());
    if (!lambda_file.is_open()) {
      std::cerr << "Levenberg-Marquardt output file: " << filename << std::endl;
      throw std::runtime_error("Error! Unable to open Levenberg-Marquardt output file.");
    }
    lambda_file << "outer_iteration,j_line_search,lambdas(1:N_line_search),objective_functions(1:N_line_search),min_objective_function_index,line_search_succeeded" << std::endl;
  }
}

//! The main driver for the Levenberg-Marquardt solver
/**
 *
 */
void mango::Levenberg_marquardt::solve() {
  keep_going_outer = true;
  outer_iteration = 0;
  //  if (solver->mpi_partition->get_proc0_world()) {
  while (keep_going_outer) {
    outer_iteration++;
    // In finite_difference_Jacobian, proc0 will bcast, so other procs need a corresponding bcast here:
    if (! proc0_world) MPI_Bcast(&data,1,MPI_INT,0,comm_group_leaders);
    // Evaluate the Jacobian:
    solver->finite_difference_Jacobian(state_vector.data(), residuals.data(), Jacobian.data());
      
    // Apply the transformation involving sigmas and targets.
    // Do this only on proc0, since only proc0 has the Jacobian, and possibly only proc0 will have targets & sigmas.
    if (proc0_world) {
      shifted_residuals = (residuals - targets).cwiseQuotient(sigmas);
      for (j=0; j<N_parameters; j++) {
	// If it weren't for wanting to compute alpha, we could store results directly in Jacobian_extended.
	Jacobian.col(j) = Jacobian.col(j).cwiseQuotient(sigmas);
      }
    }
    // Broadcast the Jacobian and shifted_residuals to all group leaders:
    MPI_Bcast(Jacobian.data(), N_terms*N_parameters, MPI_DOUBLE, 0, comm_group_leaders);
    MPI_Bcast(shifted_residuals.data(), N_terms, MPI_DOUBLE, 0, comm_group_leaders);
    // At this point, all group leaders have the correct Jacobian and shifted_residuals.
  
    objective_function = shifted_residuals.dot(shifted_residuals);
      
    residuals_extended.topRows(N_terms) = shifted_residuals;
    if (verbose>0 && proc0_world) {
      std::cout << "Here comes state_vector from Eigen" << std::endl;
      std::cout << state_vector << std::endl;
      std::cout << "Here comes shifted_residuals from Eigen" << std::endl;
      std::cout << shifted_residuals << std::endl;
      std::cout << "Here comes residuals_extended from Eigen" << std::endl;
      std::cout << residuals_extended << std::endl;
      std::cout << "Here comes Jacobian from Eigen" << std::endl;
      std::cout << Jacobian << std::endl;
    }

#ifdef CHECK_LEAST_SQUARES_SOLUTION
    alpha = Jacobian.transpose() * Jacobian;
    alpha_prime = alpha;
    beta = -Jacobian.transpose() * shifted_residuals;
#endif

    line_search();
    if (!line_search_succeeded) {
      keep_going_outer = false;
      if (verbose>0) std::cout << "Line search failed, so exiting outer loop on proc" << solver->mpi_partition->get_rank_world() << std::endl;
    }
  } // while (keep_going_outer)

  // Finalize output file:
  if (proc0_world) lambda_file.close();

  delete[] normalized_lambda_grid;

}

//! Given a Jacobian, search over values of lambda to find a step that yields a decreased objective function
/**
 *
 */
void mango::Levenberg_marquardt::line_search() {
  line_search_succeeded = false;
  // Loop over values of central_lambda:
  for (j_line_search = 0; j_line_search < max_line_search_iterations; j_line_search++) {
    evaluate_on_lambda_grid(); // This is the expensive parallelized evaluation of the residuals.
    process_lambda_grid_results(); // This is fast post-processing on proc0_world to determine which evaluation was best, & updating lambda.
  }
  MPI_Bcast(&line_search_succeeded, 1, MPI_C_BOOL, 0, comm_group_leaders);
  MPI_Bcast(&keep_going_outer, 1, MPI_C_BOOL, 0, comm_group_leaders);
  MPI_Bcast(state_vector.data(), N_parameters, MPI_DOUBLE, 0, comm_group_leaders); // Need to broadcast the state vector here; finite_difference_Jacobian only broadcasts a copy of the state vector, not the original one.
}


//! Evaluate the residuals for a set of trial steps corresponding to a set of values for lambda
/**
 *
 */
void mango::Levenberg_marquardt::evaluate_on_lambda_grid() {
  lambda_scan_residuals = Eigen::MatrixXd::Zero(N_terms, N_line_search); // Initialize residuals to 0.
  lambda_scan_state_vectors = Eigen::MatrixXd::Zero(N_parameters, N_line_search); // Initialize to 0.
  // Perform concurrent function evaluations for several values of lambda: 
  for (j_lambda_grid = 0; j_lambda_grid < N_line_search; j_lambda_grid++) {
    // Check if this MPI proc owns this point in the lambda grid:
    if ((j_lambda_grid % solver->mpi_partition->get_N_worker_groups()) == solver->mpi_partition->get_rank_group_leaders()) {
      lambda = central_lambda * normalized_lambda_grid[j_lambda_grid];
      lambdas(j_lambda_grid) = lambda;
      if (verbose>0) std::cout << "Proc " << solver->mpi_partition->get_rank_world() << " is handling j_lambda_grid=" << j_lambda_grid 
			       << ", lambda=" << lambda << std::endl;
      Jacobian_extended.topRows(N_terms) = Jacobian;
      for (j=0; j<N_parameters; j++) {
	Jacobian_extended(N_terms+j,j) = sqrt(lambda * Jacobian.col(j).dot(Jacobian.col(j)));
      }
      if (verbose>0 && proc0_world) std::cout << "Here comes Jacobian_extended from Eigen" << std::endl << Jacobian_extended << std::endl;
      
      // Solve the linear least-squares system to compute the step in parameter space
      delta_x = -Jacobian_extended.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(residuals_extended);
      if (verbose>0 && proc0_world) std::cout << "Here comes delta_x from Eigen" << std::endl << delta_x << std::endl;
      
#ifdef CHECK_LEAST_SQUARES_SOLUTION
      // Direct approach
      for (j=0; j<N_parameters; j++) {
	alpha_prime(j,j) = alpha(j,j) * (1 + lambda);
      }
      delta_x_direct = alpha_prime.colPivHouseholderQr().solve(beta); // Solve alpha * delta_x = beta for delta_x.
      if (verbose>0 && proc0_world) std::cout << "Here comes delta_x_direct from Eigen" << std::endl << delta_x_direct << std::endl;
      difference = (delta_x - delta_x_direct).norm();
      if (verbose>0 && proc0_world) std::cout << "(delta_x - delta_x_direct).norm() = " << difference << std::endl;
      if (difference > 1e-10) {
	std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	std::cerr << "WARNING!!! delta_x vs delta_x_norm disagree!!" << std::endl;
	std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
      }
#endif
      
      state_vector_tentative = state_vector + delta_x;
      lambda_scan_state_vectors.col(j_lambda_grid) = state_vector_tentative;
      
      // Evaluate the residuals at the new point
      solver->residual_function(&N_parameters, state_vector_tentative.data(), &N_terms, residuals.data(), &failed_int, solver->problem, solver->user_data);
      lambda_scan_residuals.col(j_lambda_grid) = residuals;
    } // if this MPI proc owns this point in the lambda grid
  } // End of loop over lambda grid.
  
  // Send the computed state vectors and residuals back to proc0_world.
  // The approach requiring least communication would be to have each proc only send the residuals it evaluated.
  // However, for simpicity, I'll do a MPI_SUM reduction. This could be changed to the faster approach if needed.
  if (proc0_world) {
    MPI_Reduce(MPI_IN_PLACE,                     lambda_scan_state_vectors.data(), N_line_search * N_parameters, MPI_DOUBLE, MPI_SUM, 0, comm_group_leaders);
    MPI_Reduce(MPI_IN_PLACE,                         lambda_scan_residuals.data(), N_line_search * N_terms,      MPI_DOUBLE, MPI_SUM, 0, comm_group_leaders);
  } else {
    MPI_Reduce(lambda_scan_state_vectors.data(), lambda_scan_state_vectors.data(), N_line_search * N_parameters, MPI_DOUBLE, MPI_SUM, 0, comm_group_leaders);
    MPI_Reduce(lambda_scan_residuals.data(),         lambda_scan_residuals.data(), N_line_search * N_terms,      MPI_DOUBLE, MPI_SUM, 0, comm_group_leaders);
  }
}

//! Given a set of residual evaluations on a grid of values of lambda, determine which was the best, and determine the next value of lambda to use.
/**
 *
 */
void mango::Levenberg_marquardt::process_lambda_grid_results() { 
  double original_central_lambda = central_lambda;
  int original_j_line_search = j_line_search;

  if (proc0_world) {
    // proc0 is responsible for finding the best function evaluation and deciding how to proceed.
    failed = false;
    for (j_lambda_grid = 0; j_lambda_grid < N_line_search; j_lambda_grid++) {
      // Record the function evaluations from the lambda scan. This line also increments the counter for function evaluations.
      solver->record_function_evaluation_pointer(lambda_scan_state_vectors.col(j_lambda_grid).data(), lambda_scan_residuals.col(j_lambda_grid).data(), failed);
      // Apply the transformation involving sigmas and targets:
      shifted_residuals = (lambda_scan_residuals.col(j_lambda_grid) - targets).cwiseQuotient(sigmas);
      tentative_objective_function = shifted_residuals.dot(shifted_residuals);
      if (verbose>0) std::cout<< "For j_lambda_grid=" << j_lambda_grid << ", objective function=" << tentative_objective_function << std::endl;
      lambda_scan_objective_functions(j_lambda_grid) = tentative_objective_function;
      // Find the index in the lambda grid with smallest value of the objective function:
      if (j_lambda_grid==0) {
	min_objective_function = tentative_objective_function;
	min_objective_function_index = 0;
      } else if (tentative_objective_function < min_objective_function) {
	min_objective_function = tentative_objective_function;
	min_objective_function_index = j_lambda_grid;
      }
    }
    if (verbose>0 && proc0_world) std::cout << "Best j_lambda_grid=" << min_objective_function_index << 
				    ", lambda=" << central_lambda * normalized_lambda_grid[min_objective_function_index] << std::endl;
    if (min_objective_function < objective_function) {
      // Success: we reduced the objective function.
      state_vector = lambda_scan_state_vectors.col(min_objective_function_index);
      objective_function = min_objective_function;
      // Take the optimal lambda from the previous step, and try reducing it a bit so the next step will be more like a Newton step.
      central_lambda = central_lambda * normalized_lambda_grid[min_objective_function_index] / 10; 
      if (verbose>0) std::cout << "Line search succeeded. New central lambda = " << central_lambda << std::endl;
      line_search_succeeded = true;
      j_line_search = max_line_search_iterations; // Exit "for" loop
    } else {
      // Objective function did not decrease. Try a step that is more like gradient descent.
      central_lambda = central_lambda * lambda_increase_factor;
      if (verbose>0) std::cout << "Increasing central lambda to " << central_lambda << std::endl;
    }
    if (solver->function_evaluations >= solver->max_function_evaluations) {
      // Quit due to hitting max_function_evaluations
      j_line_search = max_line_search_iterations; // Exit inner "for" loop
      keep_going_outer = false;
      if (verbose>0) std::cout << "Maximum number of function evaluations reached." << std::endl;
    }

    // Record results in the _levenberg_marquardt output file:
    lambda_file << std::setw(6) << outer_iteration << "," << std::setw(3) << original_j_line_search << ",";
    for (int j=0; j<N_line_search; j++) lambda_file << std::setw(24) << std::setprecision(16) << std::scientific << lambdas(j) << ",";
    for (int j=0; j<N_line_search; j++) lambda_file << std::setw(24) << std::setprecision(16) << std::scientific << lambda_scan_objective_functions(j) << ",";
    lambda_file << std::setw(3) << min_objective_function_index << ", " << std::setw(1) << line_search_succeeded << std::endl << std::flush;

  } // if proc0_world
  // Broadcast results from proc0 to all group leaders.
  MPI_Bcast(&central_lambda, 1, MPI_DOUBLE, 0, comm_group_leaders);
  MPI_Bcast(&j_line_search, 1, MPI_INT, 0, comm_group_leaders);
}

//! Compute the factor by which the Levenberg-Marquardt lambda parameter is reduced when a step causes the objective function to increase.
/**
 * @param[in] N_line_search The number of points considered simultaneously in a set of trial steps.
 * @return The factor by which the Levenberg-Marquardt lambda parameter will be reduced when a step causes the objective function to increase.
*/
double mango::Levenberg_marquardt::compute_lambda_increase_factor(const int N_line_search) {
  // Step size when N_line_search -> infinity:
  double max_lambda_step = 1000.0;

  // Step size when N_line_search = 1:
  double min_lambda_step = 10.0;

  // When factor = 0, lambda_step increases rapidly with N_line_search.
  // When factor >> 1, lambda_step increases slowly with N_line_search.
  double factor = 2.0;

  return exp(log(max_lambda_step) - (log(max_lambda_step) - log(min_lambda_step)) * (1+factor)/(N_line_search+factor));
}

//! Compute a grid of values for the Levenberg-Marquardt parameter lambda that will be used for each set of concurrent function evaluations.
/**
 * @param[in] N_line_search The number of points considered simultaneously in a set of trial steps.
 * @param[in] lambda_step The factor by which the Levenberg-Marquardt lambda parameter will be reduced when a step causes the objective function to increase.
 * @param[out] lambdas The computed grid of values. It will be logarithmically centered about 1.0.
 */
void mango::Levenberg_marquardt::compute_lambda_grid(const int N_line_search, const double lambda_step, double* lambdas) {
  double log_lambda_step = log(lambda_step);
  for (int j = 0; j < N_line_search; j++) {
    lambdas[j] = exp(((j+0.5)/N_line_search - 0.5) * log_lambda_step);
  }
}

#endif // MANGO_EIGEN_AVAILABLE
