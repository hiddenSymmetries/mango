// Results from a line search should be independent of # of MPI procs.

// Verify that the minimum from the line search is chosen for the next point.

// Do 1 step of line search and make sure the results match reference values.

#ifdef MANGO_EIGEN_AVAILABLE // Don't bother doing any testing otherwise.

//#include <iostream>
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
  verbose = 0;

  // Set the point about which we will compute the derivatives:
  state_vector[0] = 1.2;
  state_vector[1] = 0.9;

  // Initialize targets and sigmas
  for (int j=0; j<N_terms; j++) {
    targets[j] = 1.5 + 2 * j;
    sigmas[j]  = 0.8 + 1.3 * j;
  }

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

/*
TEST_CASE_METHOD(mango::Least_squares_solver, "mango::Levenberg_marquardt::process_lambda_grid_results().","[problem][Levenberg_marquardt]") {
  // The Catch2 macros automatically call the mango::problem() constructor (the version with no arguments).
  N_parameters = GENERATE(range(1,5));
  N_terms = GENERATE(range(1,5));
  best_state_vector = new double[N_parameters];
  residuals = new double[N_terms]; // We must allocate this variable since the destructor will delete it.
  //double* base_case_residuals = new double[N_terms];
  state_vector = new double[N_parameters];
  targets = new double[N_terms];
  sigmas = new double[N_terms];
  best_residual_function = new double[N_terms];
  //double base_case_objective_function;
  mpi_partition = new mango::MPI_Partition();
  //residual_function = &Levenberg_marquardt_residual_function_1;
  function_evaluations = 0;
  verbose = 0;

  // Initialize targets and sigmas
  for (int j=0; j<N_terms; j++) {
    targets[j] = 1.5 + 2 * j;
    sigmas[j]  = 0.8 + 1.3 * j;
  }

  // Set up MPI:
  mpi_partition->set_N_worker_groups(1); // No reason to scan this, since only proc0_world does any work.
  mpi_partition->init(MPI_COMM_WORLD);

  N_line_search = GENERATE(range(1,5));
    
  mango::Levenberg_marquardt levenberg_marquardt(this);
				 
  levenberg_marquardt.central_lambda = 0.02;
  levenberg_marquardt.lambda_increase_factor = 25;

  //  for (int j=0; j<N_terms; j++) {
  //  for (int k=0; k<N_line_search; k++) {

  levenberg_marquardt.process_lambda_grid_results();

  CHECK(levenberg_marquardt.max_line_search_iterations == 4);

  delete[] state_vector;
  delete[] targets;
  delete[] sigmas;
  delete[] best_residual_function;
}
*/


TEST_CASE_METHOD(mango::Levenberg_marquardt_tester, "mango::Levenberg_marquardt::process_lambda_grid_results().","[problem][Levenberg_marquardt][foo]") {
  // Set up MPI:
  mpi_partition->set_N_worker_groups(1); // No reason to scan this, since only proc0_world does any work.
  mpi_partition->init(MPI_COMM_WORLD);

  N_line_search = GENERATE(range(1,5));
    
  mango::Levenberg_marquardt levenberg_marquardt(this);
				 
  //levenberg_marquardt.central_lambda = 0.02;
  //levenberg_marquardt.lambda_increase_factor = 25;
  //double temp = GENERATE(random(-10.0, 10.0));
  //std::cout << "temp=" << temp << std::endl;
  CHECK(levenberg_marquardt.max_line_search_iterations == 4);
}




#endif // MANGO_EIGEN_AVAILABLE
