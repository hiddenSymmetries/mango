#include <iostream>
#include "Least_squares_solver.hpp"
#include "Package_mango.hpp"
#include "levenberg_marquardt.hpp"

#ifdef MANGO_EIGEN_AVAILABLE
#include <Eigen/Dense>
#endif

// If the next line is uncommented, the least-squares problem will be solved
// a second way, to make sure the 2 methods agree. This check adds some computational cost.
#define CHECK_LEAST_SQUARES_SOLUTION

//void mango::Package_mango::levenberg_marquardt(Least_squares_solver* solver) {
void mango::levenberg_marquardt::solve(Least_squares_solver* solver) {
#ifdef MANGO_EIGEN_AVAILABLE
  if (solver->verbose > 0) std::cout << "Hello from levenberg_marquardt" << std::endl;

  int N_parameters = solver->N_parameters;
  int N_terms = solver->N_terms;
  int verbose = solver->verbose;

  const int STOP_GROUP_LEADERS = -1;
  const int MOBILIZE_GROUP_LEADERS_JACOBIAN = 1; // Must match the value broadcast in finite_difference_Jacobian.cpp
  const int MOBILIZE_GROUP_LEADERS_LINE_SEARCH = 2;

  double lambda = 0.01;
  //double* state_vector = new double[N_parameters];
  //double* base_case_residual = new double[N_terms];
  //double* Jacobian = new double[N_parameters*N_terms];
  // Define Eigen structures 
  
  //Eigen::VectorXd state_vector(N_parameters);
  // Convert some C arrays to Eigen vectors (no copying of memory is performed):
  Eigen::Map<Eigen::VectorXd> state_vector(solver->state_vector, N_parameters);
  Eigen::Map<Eigen::VectorXd> targets(solver->targets, N_terms);
  Eigen::Map<Eigen::VectorXd> sigmas(solver->sigmas, N_terms);
  Eigen::VectorXd state_vector_tentative(N_parameters);
  Eigen::VectorXd residuals(N_terms);
  Eigen::VectorXd shifted_residuals(N_terms);
  Eigen::MatrixXd Jacobian(N_terms,N_parameters);
  //Eigen::Map<MatrixXd> Jacobian_E(Jacobian,
  Eigen::VectorXd residuals_extended(N_terms + N_parameters);
  Eigen::MatrixXd Jacobian_extended(N_terms + N_parameters, N_parameters);
  Eigen::VectorXd delta_x(N_parameters);
#ifdef CHECK_LEAST_SQUARES_SOLUTION
  Eigen::VectorXd delta_x_direct(N_parameters);
  Eigen::MatrixXd alpha(N_parameters, N_parameters);
  Eigen::MatrixXd alpha_prime(N_parameters, N_parameters);
  Eigen::VectorXd beta(N_parameters);
#endif

  residuals_extended.bottomRows(N_parameters) = Eigen::VectorXd::Zero(N_parameters);
  Jacobian_extended.bottomRows(N_parameters) = Eigen::MatrixXd::Zero(N_parameters,N_parameters);

  int data, j, j_line_search, failed_int;
  const int N_line_search = 10;
  double objective_function, tentative_objective_function;
  double difference;
  bool failed = false;
  bool keep_going_outer = true;
  bool line_search_succeeded;

  if (solver->mpi_partition->get_proc0_world()) {
    while (keep_going_outer) {
      // Mobilize other group leaders to start finite-difference Jacobian calculation
      //data = MOBILIZE_GROUP_LEADERS_JACOBIAN;
      //MPI_Bcast(&data,1,MPI_INT,0,solver->mpi_partition->get_comm_group_leaders());
      solver->finite_difference_Jacobian(state_vector.data(), residuals.data(), Jacobian.data());
      
      // Apply the transformation involving sigmas and targets
      shifted_residuals = (residuals - targets).cwiseQuotient(sigmas);
      for (j=0; j<N_parameters; j++) {
	// If it weren't for wanting to compute alpha, we could store results directly in Jacobian_extended.
	Jacobian.col(j) = Jacobian.col(j).cwiseQuotient(sigmas);
      }
      
      objective_function = shifted_residuals.dot(shifted_residuals);
      
      residuals_extended.topRows(N_terms) = shifted_residuals;
      std::cout << "Here comes state_vector from Eigen" << std::endl;
      std::cout << state_vector << std::endl;
      std::cout << "Here comes shifted_residuals from Eigen" << std::endl;
      std::cout << shifted_residuals << std::endl;
      std::cout << "Here comes residuals_extended from Eigen" << std::endl;
      std::cout << residuals_extended << std::endl;
      std::cout << "Here comes Jacobian from Eigen" << std::endl;
      std::cout << Jacobian << std::endl;
      
#ifdef CHECK_LEAST_SQUARES_SOLUTION
      alpha = Jacobian.transpose() * Jacobian;
      alpha_prime = alpha;
      beta = -Jacobian.transpose() * shifted_residuals;
#endif

      line_search_succeeded = false;
      for (j_line_search = 0; j_line_search < N_line_search; j_line_search++) {
	
	Jacobian_extended.topRows(N_terms) = Jacobian;
	double term;
	for (j=0; j<N_parameters; j++) {
	  Jacobian_extended(N_terms+j,j) = sqrt(lambda * Jacobian.col(j).dot(Jacobian.col(j)));
	}
	std::cout << "Here comes Jacobian_extended from Eigen" << std::endl;
	std::cout << Jacobian_extended << std::endl;
	
	// Solve the linear least-squares system
	delta_x = -Jacobian_extended.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(residuals_extended);
	std::cout << "Here comes delta_x from Eigen" << std::endl;
	std::cout << delta_x << std::endl;
	
#ifdef CHECK_LEAST_SQUARES_SOLUTION
	// Direct approach
	for (j=0; j<N_parameters; j++) {
	  alpha_prime(j,j) = alpha(j,j) * (1 + lambda);
	}
	delta_x_direct = alpha_prime.colPivHouseholderQr().solve(beta); // Solve alpha * delta_x = beta for delta_x.
	std::cout << "Here comes delta_x_direct from Eigen" << std::endl;
	std::cout << delta_x_direct << std::endl;
	difference = (delta_x - delta_x_direct).norm();
	std::cout << "(delta_x - delta_x_direct).norm() = " << difference << std::endl;
	if (difference > 1e-10) {
	  std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	  std::cerr << "WARNING!!! delta_x vs delta_x_norm disagree!!" << std::endl;
	  std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
	}
#endif
	
	state_vector_tentative = state_vector + delta_x;
	
	// Evaluate the residuals at the new point
	solver->residual_function(&N_parameters, state_vector_tentative.data(), &N_terms, residuals.data(), &failed_int, solver->problem, solver->user_data);
	failed = false;
	solver->record_function_evaluation_pointer(state_vector_tentative.data(), residuals.data(), failed);
	// Apply the transformation involving sigmas and targets
	shifted_residuals = (residuals - targets).cwiseQuotient(sigmas);
	tentative_objective_function = shifted_residuals.dot(shifted_residuals);
	if (tentative_objective_function < objective_function) {
	  // Success: we reduced the objective function.
	  state_vector = state_vector_tentative;
	  objective_function = tentative_objective_function;
	  lambda = lambda / 10; // So next step will be more like a Newton step.
	  if (verbose>0) std::cout << "Line search succeeded. New lambda = " << lambda << std::endl;
	  line_search_succeeded = true;
	  j_line_search = N_line_search; // Exit "for" loop
	} else {
	  // Objective function did not decrease. Try a step that is more like gradient descent.
	  lambda = lambda * 10;
	  if (verbose>0) std::cout << "Increasing lambda to " << lambda << std::endl;
	}
	if (solver->function_evaluations >= solver->max_function_evaluations) {
	  // Quit due to hitting max_function_evaluations
	  j_line_search = N_line_search; // Exit inner "for" loop
	  keep_going_outer = false;
	  if (verbose>0) std::cout << "Maximum number of function evaluations reached." << std::endl;
	}
      }
      if (!line_search_succeeded) {
	keep_going_outer = false;
	if (verbose>0) std::cout << "Line search failed, so exiting outer loop." << std::endl;
      }
    }

    // Stop the other group leaders
    data = STOP_GROUP_LEADERS;
    MPI_Bcast(&data,1,MPI_INT,0,solver->mpi_partition->get_comm_group_leaders());

  } else {
    // Group leaders other than proc0_world execute the following block.
    bool keep_going = true;
    while (keep_going) {
      // Wait for proc 0 to send us a message that we should start.
      MPI_Bcast(&data,1,MPI_INT,0,solver->mpi_partition->get_comm_group_leaders());
      if (data == STOP_GROUP_LEADERS) {
	if (verbose > 0) std::cout << "proc " << solver->mpi_partition->get_rank_world() <<
			   " (a group leader) is exiting L-M." << std::endl;
	keep_going = false;
      } else if (data == MOBILIZE_GROUP_LEADERS_JACOBIAN) {
	if (verbose > 0) std::cout << "proc " << solver->mpi_partition->get_rank_world() <<
			   " (a group leader) is starting finite-difference Jacobian calculation for L-M." << std::endl;
	solver->finite_difference_Jacobian(state_vector.data(), residuals.data(), Jacobian.data());
      } else if (data == MOBILIZE_GROUP_LEADERS_LINE_SEARCH) {
	// Line search
      } else {
	throw std::runtime_error("Error in mango::levenberg_marquardt::solve(). Should not get here.");
      }
    }
  }
    

  fun1();
#else
  throw std::runtime_error("ERROR: The algorithm mango_levenberg_marquardt was selected. This algorithm requires Eigen, but MANGO was built without Eigen.");
#endif
}

int mango::levenberg_marquardt::fun1() {
  std::cout << "Hello from levenberg_marquardt helper function" << std::endl;
  return 17;
}
