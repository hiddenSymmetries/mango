#include "Package_mango.hpp"
#include "Least_squares_solver.hpp"

#ifdef MANGO_EIGEN_AVAILABLE
#include <Eigen/Dense>
#endif

namespace mango {
  class Levenberg_marquardt : public LeastSquaresAlgorithm {
  public:
#ifdef MANGO_EIGEN_AVAILABLE
    Least_squares_solver* solver;

    double central_lambda;
    int max_line_search_iterations;

    // Define shorthand variable names:
    int N_parameters;
    int N_terms;
    int verbose;
    int N_line_search;
    bool proc0_world;
    MPI_Comm comm_group_leaders;

    // Convert some C arrays to Eigen vectors (no copying of memory is performed):
    Eigen::Map<Eigen::VectorXd> state_vector;
    Eigen::Map<Eigen::VectorXd> targets;
    Eigen::Map<Eigen::VectorXd> sigmas;

    Eigen::VectorXd state_vector_tentative;
    Eigen::VectorXd residuals;
    Eigen::VectorXd shifted_residuals;
    Eigen::MatrixXd Jacobian;
    Eigen::VectorXd residuals_extended;
    Eigen::MatrixXd Jacobian_extended;
    Eigen::VectorXd delta_x;
    Eigen::MatrixXd lambda_scan_residuals;
    Eigen::MatrixXd lambda_scan_state_vectors;
    Eigen::VectorXd delta_x_direct;
    Eigen::MatrixXd alpha;
    Eigen::MatrixXd alpha_prime;
    Eigen::VectorXd beta;

    int data, j, j_line_search, failed_int, j_lambda_grid;
    double lambda, objective_function, tentative_objective_function, min_objective_function;
    int min_objective_function_index;
    double difference;
    bool failed;
    bool keep_going_outer;
    bool line_search_succeeded;
    double lambda_increase_factor;
    double* normalized_lambda_grid;

    void evaluate_on_lambda_grid();
    void process_lambda_grid_results();
    void line_search();

    static double compute_lambda_increase_factor(const int);
    static void compute_lambda_grid(const int, const double, double*);

#endif
    Levenberg_marquardt(Least_squares_solver*);
    void solve();

  };

}

