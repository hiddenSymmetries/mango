#include "catch.hpp"
#include "mango.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>

void objective_function_1(int* N_parameters, const double* x, double* f, int* failed_int, mango::problem* problem) {
  assert(*N_parameters == 2);
  *f = exp(x[0] * x[0] - exp(x[1])); // A 2-D model function, for testing.
  *failed_int = false;
}


TEST_CASE_METHOD(mango::problem, "problem::finite_difference_Jacobian","[problem]") {
  // The Catch2 macros automatically call the mango::problem() constructor (the version with no arguments).
  N_parameters = 2;
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
  state_vector[0] = 1.2;
  state_vector[1] = 0.9;

  finite_difference_step_size = 1.0e-7;
  double correct_objective_function = 3.607380846860443e-01;

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
      //std::cout << "base_case_objective_function: " << std::scientific << std::setw(24) << std::setprecision(15) << base_case_objective_function;
      //std::cout << ",  gradient: " << std::setw(24) << gradient[0] << ", " << gradient[1] << std::endl;
      // Finally, see if the results are correct:
      CHECK(        function_evaluations == 3);
      CHECK(base_case_objective_function == Approx(correct_objective_function).epsilon(1e-14));
      CHECK(                 gradient[0] == Approx( 8.657715439008840e-01).epsilon(1e-13));
      CHECK(                 gradient[1] == Approx(-8.872724499564555e-01).epsilon(1e-13));
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
      //std::cout << "base_case_objective_function: " << std::scientific << std::setw(24) << std::setprecision(15) << base_case_objective_function;
      //std::cout << ",  gradient: " << std::setw(24) << gradient[0] << ", " << gradient[1] << std::endl;
      // Finally, see if the results are correct:
      CHECK(        function_evaluations == 5);
      CHECK(base_case_objective_function == Approx(correct_objective_function).epsilon(1e-14));
      CHECK(                 gradient[0] == Approx( 8.657714037352271e-01).epsilon(1e-13));
      CHECK(                 gradient[1] == Approx(-8.872725151820582e-01).epsilon(1e-13));
    }
  }

  if (mpi_partition.get_proc0_world()) output_file.close();
  delete[] state_vector;
  delete[] gradient;
}
