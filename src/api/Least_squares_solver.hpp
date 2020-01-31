#ifndef MANGO_LEAST_SQUARES_SOLVER_H
#define MANGO_LEAST_SQUARES_SOLVER_H

#include "mango.hpp"
#include "Solver.hpp"

namespace mango {

  // This class contains ugly implementation details that are specific to least-squares minimization
  class Least_squares_solver : public Solver {

  private:
    void* original_user_data;

    // Overrides the routines in Solver:
    void group_leaders_loop();
    bool record_function_evaluation(const double*, double, bool); // Overrides Solver.
    void record_function_evaluation(const double*, double*, bool);// Not an override! Note double* instead of double in 2nd argument.

    //void compose_residuals_string(std::string&, double*);
    //void write_least_squares_file_line(clock_t, const double*, double, double*);

    // Much of the solver are public because this information must be used by the concrete Package.
  public:
    int N_terms;
    double* targets;
    double* sigmas;
    residual_function_type residual_function;
    double* best_residual_function;
    double* residuals;
    bool print_residuals_in_output_file;
    double* current_residuals;
    Least_squares_problem* least_squares_problem;

    Least_squares_solver(Least_squares_problem*, int, int);
    ~Least_squares_solver();

    // Overrides of routines in Solver:
    double optimize(MPI_Partition*);
    void finite_difference_gradient(const double*, double*, double*);
    //bool record_function_evaluation(const double*, double, bool);

    // Methods that do not exist in Solver:
    double residuals_to_single_objective(double*);
    void residual_function_wrapper(const double*, double*, bool*);
    void finite_difference_Jacobian(const double*, double*, double*);
    static void least_squares_to_single_objective(int*, const double*, double*, int*, mango::Problem*, void*);
  };
}

#endif
