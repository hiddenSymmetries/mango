#ifdef MANGO_EIGEN_AVAILABLE // Don't bother doing any testing if Eigen is unavailable.

#include <iostream>
#include <iomanip>
#include "catch.hpp"
#include "Levenberg_marquardt.hpp"


TEST_CASE("Levenberg_marquardt","[algorithm][Levenberg_marquardt]") {
  // If N_line_search==1, the normalized lambda grid should be [1.0] for any lambda_step
  int N_line_search = 1;
  auto log_step = GENERATE(range(0,5)); // so log_step = 0, 1, 2, 3, or 4.
  double lambda_step = exp(log_step - 2.0);
  double lambda_grid;
  mango::Levenberg_marquardt::compute_lambda_grid(N_line_search, lambda_step, &lambda_grid);
  CHECK(lambda_grid == Approx(1.0).epsilon(1e-13));
}

//! An example residual function that is used for unit testing
void Levenberg_marquardt_residual_function_1(int* N_parameters, const double* x, int* N_terms, double* f, int* failed_int, mango::Problem* problem, void* user_data) {
  assert(*N_parameters == 2);
  assert(*N_terms == 4);
  for (int j = 0; j < *N_terms; j++) {
    f[j] = exp(j + x[0] * x[0] - exp(x[1]));
  }
  *failed_int = false;
}

// Create a subclass that handles setup and tear-down:
namespace mango {
  class Levenberg_marquardt_tester : public Least_squares_solver {
  public:
    Levenberg_marquardt_tester();
    ~Levenberg_marquardt_tester();
  };
}

mango::Levenberg_marquardt_tester::Levenberg_marquardt_tester() {
  // The Catch2 macros automatically call the mango::problem() constructor (the version with no arguments).
  N_parameters = 2;
  N_terms = 4;
  best_state_vector = new double[N_parameters];
  residuals = new double[N_terms]; // We must allocate this variable since the destructor will delete it.
  //double* base_case_residuals = new double[N_terms];
  state_vector = new double[N_parameters];
  targets = new double[N_terms];
  sigmas = new double[N_terms];
  best_residual_function = new double[N_terms];
  //double base_case_objective_function;
  mpi_partition = new mango::MPI_Partition();
  residual_function = &Levenberg_marquardt_residual_function_1;
  function_evaluations = 0;
  max_function_evaluations = 10000;
  verbose = 0;

  // Set the point about which we will compute the derivatives:
  state_vector[0] = 1.2;
  state_vector[1] = 0.9;

  // Initialize targets and sigmas
  for (int j=0; j<N_terms; j++) {
    targets[j] = 1.5 + 2 * j;
    sigmas[j]  = 0.8 + 1.3 * j;
  }

  centered_differences = true;
  finite_difference_step_size = 1.0e-7;
}

mango::Levenberg_marquardt_tester::~Levenberg_marquardt_tester() {
  delete[] state_vector;
  delete[] targets;
  delete[] sigmas;
  delete[] best_residual_function;
  //delete[] base_case_residuals;
  // best_state_vector and residuals will be deleted by destructor.
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


TEST_CASE_METHOD(mango::Least_squares_solver, "mango::Levenberg_marquardt::process_lambda_grid_results().","[Levenberg_marquardt]") {
  // First, set up some variables that are needed.

  // The Catch2 macros automatically call the mango::problem() constructor (the version with no arguments).
  N_parameters = GENERATE(range(1,6));
  N_terms = GENERATE(range(1,6));
  best_state_vector = new double[N_parameters];
  residuals = new double[N_terms]; // We must allocate this variable since the destructor will delete it.
  //double* base_case_residuals = new double[N_terms];
  state_vector = new double[N_parameters];
  targets = new double[N_terms];
  sigmas = new double[N_terms];
  best_residual_function = new double[N_terms];
  //double base_case_objective_function;
  mpi_partition = new mango::MPI_Partition();
  residual_function = &Levenberg_marquardt_residual_function_1;
  function_evaluations = 0;
  verbose = 0;

  // Initialize targets and sigmas
  for (int j=0; j<N_terms; j++) {
    //targets[j] = 1.5 + 2 * j;
    //sigmas[j]  = 0.8 + 1.3 * j;
    targets[j] = GENERATE(take(1,random(-10.0,10.0)));
    sigmas[j]  = GENERATE(take(1,random(0.1,10.0)));
  }  

  // Set up MPI:
  mpi_partition->set_N_worker_groups(1); // No reason to scan this, since only proc0_world does any work.
  mpi_partition->init(MPI_COMM_WORLD);

  N_line_search = GENERATE(range(1,6));
  CAPTURE(N_parameters, N_terms, N_line_search);

  mango::Levenberg_marquardt lm(this);
  lm.save_lambda_history = false;
				 
  lm.line_search_succeeded = false;
  double original_central_lambda = GENERATE(take(1,random(-10.0,10.0)));
  lm.central_lambda = original_central_lambda;
  lm.lambda_increase_factor = GENERATE(take(1,random(2.0,30.0)));
  double original_objective_function = GENERATE(take(1,random(0.1,10.0)));
  lm.objective_function = original_objective_function;
  
  for (int k=0; k<N_line_search; k++) {
    lm.normalized_lambda_grid[k] = GENERATE(take(1,random(-10.0,10.0)));
    for (int j=0; j<N_terms; j++) lm.lambda_scan_residuals(j,k) = GENERATE(take(1,random(-10.0,10.0)));
    for (int j=0; j<N_parameters; j++) lm.lambda_scan_state_vectors(j,k) = GENERATE(take(1,random(-10.0,10.0)));
  }
    
  // Call the subroutine we want to test:
  if (mpi_partition->get_proc0_worker_groups()) { // Only group leaders should call the subroutine
    lm.process_lambda_grid_results();
  }

  // The subroutine only does computations on proc0_world, so only run the tests on proc0_world:
  if (lm.proc0_world) {

    // Verify that function_evaluations was incremented appropriately:
    CHECK(function_evaluations == N_line_search);

    // Verify that we succeeded in picking out the best objective function from the set:
    double best_objective_function = residuals_to_single_objective(lm.lambda_scan_residuals.col(lm.min_objective_function_index).data());
    double f;
    for (int j=0; j<N_line_search; j++) {
      f = residuals_to_single_objective(lm.lambda_scan_residuals.col(j).data());
      CHECK(f >= best_objective_function);
    }
    
    // Verify that the best-yet objective function in LM is <= the best objective function from the lambda scan:
    if (lm.line_search_succeeded) {
      // If we successfully lowered the objective function,
      // the best-yet objective function in LM should equal the best objective function from the lambda scan:
      CHECK(lm.objective_function == Approx(best_objective_function));
      // and lambda should have been decreased:
      CHECK(lm.central_lambda == Approx(original_central_lambda * lm.normalized_lambda_grid[lm.min_objective_function_index] / lm.lambda_reduction_on_success));
    } else {
      // If we failed to lower the objective function,
      // the best-yet objective function in LM should be smaller than the best objective function from the lambda scan:
      CHECK(lm.objective_function < best_objective_function);
      // and lambda should have been increased:
      CHECK(lm.central_lambda == Approx(original_central_lambda * lm.lambda_increase_factor));
    }
  }

  // Clean up arrays that were allocated:
  delete[] state_vector;
  delete[] targets;
  delete[] sigmas;
  delete[] best_residual_function;
}


TEST_CASE_METHOD(mango::Levenberg_marquardt_tester, "mango::Levenberg_marquardt::evaluate_on_lambda_grid() and line_search()",
		 "[Levenberg_marquardt]") {
  // Validate one piece of the parallelized lambda scan.
  // We won't actually calculate the true Jacobian, just plug in an ad-hoc matrix for testing.
  // This test case is a mini-regression test just for one subroutine.

  // Set up MPI:
  mpi_partition->set_N_worker_groups(GENERATE(range(1,6)));
  mpi_partition->init(MPI_COMM_WORLD);

  N_line_search = 4;
  //centered_differences = true;

  mango::Levenberg_marquardt lm(this);
  lm.check_least_squares_solution = false;
  lm.save_lambda_history = false;

  lm.state_vector << 0.6, -0.8;

  // Here we set central_lambda and normalized_lambda_grid so that if the values for these quantities in
  // normal operation are changed in Levenberg_marquardt.cpp, it won't break this test.
  lm.central_lambda = 0.02;
  lm.normalized_lambda_grid[0] = 0.1;
  lm.normalized_lambda_grid[1] = 0.3;
  lm.normalized_lambda_grid[2] = 1.0;
  lm.normalized_lambda_grid[3] = 3.0;

  // Not the true Jacobian, just some values I made up.
  lm.Jacobian <<  
    1.1,  2.5,
    3.2,  4.0,
    -1.1, -2.5,
    -3.2, -4.0;

  // Not the true residuals, just some values I made up.
  lm.residuals_extended << 0.7, 0.9, -1.2, -1.9, 0.0, 0.0;
 
  //if (mpi_partition->get_proc0_world()) {
  //  std::cout << "Jacobian:" << std::endl << lm.Jacobian << std::endl;
  //  std::cout << "residuals_extended:" << std::endl << lm.residuals_extended << std::endl;
  //}
  
  SECTION("Check mango::Levenberg_marquardt::evaluate_on_lambda_grid()") {

    // Call the subroutine we want to test:
    if (mpi_partition->get_proc0_worker_groups()) { // Only group leaders should call the subroutine
      lm.evaluate_on_lambda_grid();
    }

  }
  SECTION("Verify mango::Levenberg_marquardt::line_search() for a case in which the objective function is reduced already on the 1st lambda_grid") {
    lm.objective_function = 1.0e200; // Set to any value larger than the min objective function for this lambda scan.

    // Call the subroutine we want to test:
    if (mpi_partition->get_proc0_worker_groups()) { // Only group leaders should call the subroutine
      lm.line_search();

      CHECK(lm.line_search_succeeded == true);
      //std::cout << "state_vector after:" << std::endl << std::setprecision(16) << std::scientific << lm.state_vector << std::endl;
      CHECK(lm.state_vector[0] == Approx(4.5835619960968432e-01));
      CHECK(lm.state_vector[1] == Approx(-1.0447504305249349e+00));
    }
    
  }

  if (mpi_partition->get_proc0_world()) {
    // The reference values later in this subroutine were obtained using these next 2 lines:
    //std::cout << "lambda_scan_state_vectors:" << std::endl << std::setprecision(16) << std::scientific << lm.lambda_scan_state_vectors << std::endl;
    //std::cout << "lambda_scan_residuals:" << std::endl << std::setprecision(16) << std::scientific << lm.lambda_scan_residuals << std::endl;
    
    Eigen::MatrixXd lambda_scan_state_vectors(2,4);
    lambda_scan_state_vectors <<
      6.5966619313657804e-01,  6.2136563284770552e-01,  5.4071043403897379e-01,  4.5835619960968432e-01,
      -1.1993276056100737e+00, -1.1711321102947181e+00, -1.1107753800090787e+00, -1.0447504305249349e+00;
    
    Eigen::MatrixXd lambda_scan_residuals(4,4);
    lambda_scan_residuals <<
      1.1431215076343009e+00, 1.0790483326566978e+00, 9.6373805043769645e-01, 8.6789095841964070e-01,
      3.1073264219230281e+00, 2.9331574746897324e+00, 2.6197116298993373e+00, 2.3591722213560140e+00,
      8.4465889478040292e+00, 7.9731486634579225e+00, 7.1211145193581968e+00, 6.4128949795174135e+00,
      2.2960209249278702e+01, 2.1673265127480192e+01, 1.9357196196347253e+01, 1.7432055890638424e+01;
    
    for (int j=0; j<N_line_search; j++) {
      for (int k=0; k<N_parameters; k++) {
	CHECK(lambda_scan_state_vectors(k,j) == Approx(lm.lambda_scan_state_vectors(k,j)));
      }
      for (int k=0; k<N_terms; k++) {
	CHECK(lambda_scan_residuals(k,j) == Approx(lm.lambda_scan_residuals(k,j)));
      }
    }
  }
  
}


TEST_CASE_METHOD(mango::Levenberg_marquardt_tester, "mango::Levenberg_marquardt::solve()",
		 "[Levenberg_marquardt]") {
  // Take one outer step of Levenberg-Marquardt. This test case is a mini regression test.

  // Set up MPI:
  auto N_worker_groups = GENERATE(range(1,6));
  mpi_partition->set_N_worker_groups(N_worker_groups);
  CAPTURE(N_worker_groups);
  mpi_partition->init(MPI_COMM_WORLD);

  N_line_search = 5;

  mango::Levenberg_marquardt lm(this);
  lm.check_least_squares_solution = true;
  lm.save_lambda_history = false; // Should be false eventually
  lm.max_outer_iterations = 1; // Do only a single outer iteration, for simplicity.
  lm.verbose = 0;

  lm.state_vector << 0.0, 0.0;

  // Here we set central_lambda, normalized_lambda_grid, and lambda_reduction_on_success so that if the values for these quantities in
  // normal operation are changed in Levenberg_marquardt.cpp, it won't break this test.
  lm.central_lambda = 1.0e-2;
  lm.normalized_lambda_grid[0] = 0.1;
  lm.normalized_lambda_grid[1] = 0.3;
  lm.normalized_lambda_grid[2] = 1.0;
  lm.normalized_lambda_grid[3] = 3.0;
  lm.normalized_lambda_grid[4] = 10.0;
  lm.lambda_reduction_on_success = 5.0;

  // Call the subroutine we want to test:
  if (mpi_partition->get_proc0_worker_groups()) { // Only group leaders should call the subroutine
    lm.solve();
  }
  
  if (mpi_partition->get_proc0_world()) {
    CHECK(lm.Jacobian(0,0) == Approx(0.0));
    CHECK(lm.Jacobian(1,0) == Approx(0.0));
    CHECK(lm.Jacobian(2,0) == Approx(0.0));
    CHECK(lm.Jacobian(3,0) == Approx(0.0));
    CHECK(lm.Jacobian(0,1) == Approx(-4.5984930134579383e-01));
    CHECK(lm.Jacobian(1,1) == Approx(-4.7619047620416932e-01));
    CHECK(lm.Jacobian(2,1) == Approx(-7.9949465544636245e-01));
    CHECK(lm.Jacobian(3,1) == Approx(-1.5721395948669106e+00));

    CHECK(lm.state_vector(0) == Approx(0.0));
    CHECK(lm.state_vector(1) == Approx(-5.3731855765138470e-01));

    CHECK(lm.lambda_scan_objective_functions(0) == Approx(3.0649073234919824e+00));
    CHECK(lm.lambda_scan_objective_functions(1) == Approx(3.0650616944702622e+00));
    CHECK(lm.lambda_scan_objective_functions(2) == Approx(3.0656473368484627e+00));
    CHECK(lm.lambda_scan_objective_functions(3) == Approx(3.0676889601937392e+00));
    CHECK(lm.lambda_scan_objective_functions(4) == Approx(3.0784332659074516e+00));

    CHECK(lm.central_lambda == Approx(2.0e-4));

  }
}


#endif // MANGO_EIGEN_AVAILABLE
