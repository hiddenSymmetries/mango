#include "catch.hpp"
#include "mango.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test finite-difference gradient, for a non-least-squares problem.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void objective_function_1(int* N_parameters, const double* x, double* f, int* failed_int, mango::problem* problem) {
  assert(*N_parameters == 3);
  *f = exp(x[0] * x[0] - exp(x[1]) + sin(x[2])); // A 3-D model function, for testing.
  *failed_int = false;
}


TEST_CASE_METHOD(mango::problem, "problem::finite_difference_gradient()","[problem][finite difference]") {
  // The Catch2 macros automatically call the mango::problem() constructor (the version with no arguments).
  N_parameters = 3;
  delete[] best_state_vector; // The constructor problem() allocated best_state_vector of size 1, so reallocate it.
  best_state_vector = new double[N_parameters];
  state_vector = new double[N_parameters];
  double* gradient = new double[N_parameters];
  least_squares = false;
  objective_function = &objective_function_1;
  function_evaluations = 0;
  verbose = 0;
  double base_case_objective_function;

  // Set up MPI:
  auto N_worker_groups_requested = GENERATE(range(1,5)); // Scan over N_worker_groups
  mpi_partition.set_N_worker_groups(N_worker_groups_requested);
  mpi_partition.init(MPI_COMM_WORLD);

  // We need to open an output file, since the finite-differencing routine will write to it.
  if (mpi_partition.get_proc0_world()) {
    output_file.open("mango_out.temp");
    if (!output_file.is_open()) throw std::runtime_error("Error! Unable to open output file.");
  }

  // Set the point about which we will compute the gradient:
  state_vector[0] =  1.2;
  state_vector[1] =  0.9;
  state_vector[2] = -0.4;

  finite_difference_step_size = 1.0e-7;
  double correct_objective_function = 2.443823056453063e-01;

  SECTION("1-sided differences") {
    centered_differences = false;

    if (mpi_partition.get_proc0_world()) {
      // Case of proc0_world
      finite_difference_gradient(state_vector, &base_case_objective_function, gradient);
      // Tell group leaders to exit.
      int data = -1;
      MPI_Bcast(&data,1,MPI_INT,0,mpi_partition.get_comm_group_leaders());
    } else {
      // Case for group leaders:
      if (mpi_partition.get_proc0_worker_groups()) {
	group_leaders_loop();
      } else {
	// Everybody else, i.e. workers. Nothing to do here.
      }
    }
    
    if (mpi_partition.get_proc0_world()) {
      //std::cout << "base_case_objective_function: " << std::scientific << std::setw(24) << std::setprecision(15) << base_case_objective_function << ",  1-sided gradient: " << gradient[0] << ", " << gradient[1] << ", " << gradient[2] << std::endl;
      // Finally, see if the results are correct:
      CHECK(        function_evaluations == 4);
      CHECK(base_case_objective_function == Approx(correct_objective_function).epsilon(1e-14));
      CHECK(                 gradient[0] == Approx( 5.865176283537110e-01).epsilon(1e-13));
      CHECK(                 gradient[1] == Approx(-6.010834349701177e-01).epsilon(1e-13));
      CHECK(                 gradient[2] == Approx( 2.250910244305793e-01).epsilon(1e-13));
      // Also check best_state_vector, best_objective_function and best_function_evaluation?
    }
  }
  SECTION("centered differences") {
    centered_differences = true;

    if (mpi_partition.get_proc0_world()) {
      // Case of proc0_world
      finite_difference_gradient(state_vector, &base_case_objective_function, gradient);
      // Tell group leaders to exit.
      int data = -1;
      MPI_Bcast(&data,1,MPI_INT,0,mpi_partition.get_comm_group_leaders());
    } else {
      // Case for group leaders:
      if (mpi_partition.get_proc0_worker_groups()) {
	group_leaders_loop();
      } else {
	// Everybody else, i.e. workers. Nothing to do here.
      }
    }
    
    if (mpi_partition.get_proc0_world()) {
      //std::cout << "base_case_objective_function: " << std::scientific << std::setw(24) << std::setprecision(15) << base_case_objective_function << ",  centered gradient: " << gradient[0] << ", " << gradient[1] << ", " << gradient[2] << std::endl;
      // Finally, see if the results are correct:
      CHECK(        function_evaluations == 7);
      CHECK(base_case_objective_function == Approx(correct_objective_function).epsilon(1e-14));
      CHECK(                 gradient[0] == Approx( 5.865175337071982e-01).epsilon(1e-13));
      CHECK(                 gradient[1] == Approx(-6.010834789627051e-01).epsilon(1e-13));
      CHECK(                 gradient[2] == Approx( 2.250910093037906e-01).epsilon(1e-13));
    }
  }

  if (mpi_partition.get_proc0_world()) output_file.close();
  delete[] state_vector;
  delete[] gradient;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Now test finite-difference Jacobian, for a least-squares problem.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void residual_function_1(int* N_parameters, const double* x, int* N_terms, double* f, int* failed_int, mango::problem* problem) {
  assert(*N_parameters == 2);
  assert(*N_terms == 4);
  for (int j = 0; j < *N_terms; j++) {
    f[j] = exp(j + x[0] * x[0] - exp(x[1]));
  }
  *failed_int = false;
}


TEST_CASE_METHOD(mango::problem, "problem::finite_difference_Jacobian()","[problem][finite difference]") {
  // The Catch2 macros automatically call the mango::problem() constructor (the version with no arguments).
  N_parameters = 2;
  N_terms = 4;
  delete[] best_state_vector; // The constructor problem() allocated best_state_vector of size 1, so reallocate it.
  best_state_vector = new double[N_parameters];
  residuals = new double[N_terms]; // We must allocate this variable since the destructor will delete it.
  double* base_case_residuals = new double[N_terms];
  state_vector = new double[N_parameters];
  double* Jacobian = new double[N_parameters * N_terms];
  targets = new double[N_terms];
  sigmas = new double[N_terms];
  best_residual_function = new double[N_terms];
  least_squares = true;
  residual_function = &residual_function_1;
  function_evaluations = 0;
  verbose = 0;

  // Set up MPI:
  auto N_worker_groups_requested = GENERATE(range(1,5)); // Scan over N_worker_groups
  //int N_worker_groups_requested = 1;
  mpi_partition.set_N_worker_groups(N_worker_groups_requested);
  mpi_partition.init(MPI_COMM_WORLD);

  // We need to open an output file, since the finite-differencing routine will write to it.
  if (mpi_partition.get_proc0_world()) {
    output_file.open("mango_out.temp");
    if (!output_file.is_open()) throw std::runtime_error("Error! Unable to open output file.");
  }

  // Set the point about which we will compute the gradient:
  state_vector[0] = 1.2;
  state_vector[1] = 0.9;

  // Initialize targets and sigmas
  for (int j=0; j<N_terms; j++) {
    targets[j] = 1.5 + 2 * j;
    sigmas[j]  = 0.8 + 1.3 * j;
  }

  finite_difference_step_size = 1.0e-7;
  double correct_residuals[] = {3.607380846860443e-01, 9.805877804351946e-01, 2.665513944765978e+00, 7.245618119561541e+00};

  SECTION("1-sided differences") {
    centered_differences = false;

    if (mpi_partition.get_proc0_world()) {
      // Case of proc0_world
      finite_difference_Jacobian(state_vector, base_case_residuals, Jacobian);
      // Tell group leaders to exit.
      int data = -1;
      MPI_Bcast(&data,1,MPI_INT,0,mpi_partition.get_comm_group_leaders());
    } else {
      // Case for group leaders:
      if (mpi_partition.get_proc0_worker_groups()) {
	group_leaders_least_squares_loop();
      } else {
	// Everybody else, i.e. workers. Nothing to do here.
      }
    }
    
    if (mpi_partition.get_proc0_world()) {
      // Finally, see if the results are correct:
      
      //std::cout << "residuals:" << std::scientific << std::setprecision(15);
      //for (int k=0; k<N_terms; k++) std::cout << " " << std::setw(24) << base_case_residuals[k];
      //std::cout << std::endl;
      //std::cout << "d(residuals)/d(x[0]):";
      //for (int k=0; k<N_terms; k++) std::cout << " " << std::setw(24) << Jacobian[k];
      //std::cout << std::endl;
      //std::cout << "d(residuals)/d(x[1]):";
      //for (int k=0; k<N_terms; k++) std::cout << " " << std::setw(24) << Jacobian[k+N_terms];
      //std::cout << std::endl;
      
      CHECK(function_evaluations == 3);
      
      //for (int k=0; k<N_terms; k++) CHECK(residuals[k] == Approx(correct_residuals[k]).epsilon(1e-14));
      CHECK(base_case_residuals[0] == Approx(correct_residuals[0]).epsilon(1e-14));
      CHECK(base_case_residuals[1] == Approx(correct_residuals[1]).epsilon(1e-14));
      CHECK(base_case_residuals[2] == Approx(correct_residuals[2]).epsilon(1e-14));
      CHECK(base_case_residuals[3] == Approx(correct_residuals[3]).epsilon(1e-14));

      // Order of Jacobian entries is [j_parameter*N_terms + j_term], so term is least significant, parameter is most significant.
      CHECK(Jacobian[0] == Approx(8.657715439008840e-01).epsilon(1e-13));
      CHECK(Jacobian[1] == Approx(2.353411054922816e+00).epsilon(1e-13));
      CHECK(Jacobian[2] == Approx(6.397234502131255e+00).epsilon(1e-13));
      CHECK(Jacobian[3] == Approx(1.738948636642590e+01).epsilon(1e-13));

      CHECK(Jacobian[4] == Approx(-8.872724499564555e-01).epsilon(1e-13));
      CHECK(Jacobian[5] == Approx(-2.411856577788640e+00).epsilon(1e-13));
      CHECK(Jacobian[6] == Approx(-6.556105911492693e+00).epsilon(1e-13));
      CHECK(Jacobian[7] == Approx(-1.782134355643450e+01).epsilon(1e-13));
      
    }
  }
  SECTION("Centered differences") {
    centered_differences = true;

    if (mpi_partition.get_proc0_world()) {
      // Case of proc0_world
      finite_difference_Jacobian(state_vector, base_case_residuals, Jacobian);
      // Tell group leaders to exit.
      int data = -1;
      MPI_Bcast(&data,1,MPI_INT,0,mpi_partition.get_comm_group_leaders());
    } else {
      // Case for group leaders:
      if (mpi_partition.get_proc0_worker_groups()) {
	group_leaders_least_squares_loop();
      } else {
	// Everybody else, i.e. workers. Nothing to do here.
      }
    }
    
    if (mpi_partition.get_proc0_world()) {
      // Finally, see if the results are correct:
      
      //std::cout << "residuals:" << std::scientific << std::setprecision(15);
      //for (int k=0; k<N_terms; k++) std::cout << " " << std::setw(24) << base_case_residuals[k];
      //std::cout << std::endl;
      //std::cout << "d(residuals)/d(x[0]):";
      //for (int k=0; k<N_terms; k++) std::cout << " " << std::setw(24) << Jacobian[k];
      //std::cout << std::endl;
      //std::cout << "d(residuals)/d(x[1]):";
      //for (int k=0; k<N_terms; k++) std::cout << " " << std::setw(24) << Jacobian[k+N_terms];
      //std::cout << std::endl;
      
      CHECK(function_evaluations == 5);
      
      //for (int k=0; k<N_terms; k++) CHECK(residuals[k] == Approx(correct_residuals[k]).epsilon(1e-14));
      CHECK(base_case_residuals[0] == Approx(correct_residuals[0]).epsilon(1e-14));
      CHECK(base_case_residuals[1] == Approx(correct_residuals[1]).epsilon(1e-14));
      CHECK(base_case_residuals[2] == Approx(correct_residuals[2]).epsilon(1e-14));
      CHECK(base_case_residuals[3] == Approx(correct_residuals[3]).epsilon(1e-14));

      // Order of Jacobian entries is [j_parameter*N_terms + j_term], so term is least significant, parameter is most significant.
      CHECK(Jacobian[0] == Approx(8.657714037352271e-01).epsilon(1e-13));
      CHECK(Jacobian[1] == Approx(2.353410674116319e+00).epsilon(1e-13));
      CHECK(Jacobian[2] == Approx(6.397233469623842e+00).epsilon(1e-13));
      CHECK(Jacobian[3] == Approx(1.738948351093228e+01).epsilon(1e-13));

      CHECK(Jacobian[4] == Approx(-8.872725151820582e-01).epsilon(1e-13));
      CHECK(Jacobian[5] == Approx(-2.411856754314101e+00).epsilon(1e-13));
      CHECK(Jacobian[6] == Approx(-6.556106388888594e+00).epsilon(1e-13));
      CHECK(Jacobian[7] == Approx(-1.782134486205678e+01).epsilon(1e-13));
      
    }
  }

  if (mpi_partition.get_proc0_world()) output_file.close();
  delete[] state_vector;
  delete[] Jacobian;
  delete[] targets;
  delete[] sigmas;
  delete[] best_residual_function;
  delete[] base_case_residuals;
  // best_state_vector and residuals will be deleted by destructor.
}


