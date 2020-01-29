#ifndef MANGO_LEAST_SQUARES_DATA_H
#define MANGO_LEAST_SQUARES_DATA_H

#include "mango.hpp"

namespace mango {

  class Problem_data;

  // This class contains ugly implementation details that are specific to least-squares minimization
  class Least_squares_data {

  private:
    void* original_user_data;

    void group_leaders_least_squares_loop();
    //void compose_residuals_string(std::string&, double*);
    //void write_least_squares_file_line(clock_t, const double*, double, double*);
    void record_function_evaluation(const double*, double*, bool);

    // Much of the data are public because this information must be used by the concrete Package.
  public:
    int N_terms;
    double* targets;
    double* sigmas;
    residual_function_type residual_function;
    double* best_residual_function;
    double* residuals;
    bool print_residuals_in_output_file;
    Problem_data* problem_data;
    double* current_residuals;

    Least_squares_data(Problem_data*, int);
    ~Least_squares_data();

    double optimize(MPI_Partition*);
    double residuals_to_single_objective(double*);
    void residual_function_wrapper(const double*, double*, bool*);
    void finite_difference_Jacobian(const double*, double*, double*);
    void finite_difference_Jacobian_to_gradient(const double*, double*, double*);
    static void least_squares_to_single_objective(int*, const double*, double*, int*, mango::Problem*, void*);
  };
}

#endif
