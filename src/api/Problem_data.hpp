#ifndef MANGO_PROBLEM_DATA_H
#define MANGO_PROBLEM_DATA_H

#include <mpi.h>
#include <string>
#include <fstream>
#include <ctime>
#include "mango.hpp"
#include "Package.hpp"

namespace mango {

  class Problem_data {
    // This class contains the ugly implementation details of the interface for Problem specified in mango.hpp.

  private:
    void init_optimization();
    void group_leaders_loop();
    void load_algorithm_properties();
    void set_package();
    void write_function_evaluation_and_time(clock_t);
    void compose_x_f_string(std::string&, const double*, double);
    void write_file_line(clock_t, const double*, double);
    // 20200127 Temporarily commenting out the next 2 lines, until I get least-squares problems working.
    //void compose_residuals_string(std::string&, double*);
    //void write_least_squares_file_line(clock_t, const double*, double, double*);
    
    // This GSL and HOPSPACK stuff will eventually be moved to the Package_GSL and Package_HOPSPACK classes.
    //#ifdef MANGO_GSL_AVAILABLE
    //static int gsl_residual_function(const gsl_vector*, void*, gsl_vector*);
    //static int gsl_residual_function_and_Jacobian(const gsl_vector*, void*, gsl_matrix*);
    //static double gsl_objective_function(const gsl_vector*, void*);
    //static void gsl_gradient(const gsl_vector*, void*, gsl_vector*);
    //static void gsl_objective_function_and_gradient(const gsl_vector*, void*, double*, gsl_vector*);
    //#endif
    //void write_hopspack_line_to_file(std::string line, double);
    //void finite_difference_Jacobian(const double*, double*, double*);
    //void finite_difference_Jacobian_to_gradient(const double*, double*, double*);

  public:
    // All data in this class is public because this information must be used by the concrete Package.
    // It would not work to make Package a friend of this class because friendship would not extend to the concrete subclasses of Package.
    algorithm_type algorithm;
    int N_parameters;
    objective_function_type objective_function;
    int function_evaluations;
    std::ofstream output_file;
    int argc;
    char** argv;
    int max_function_and_gradient_evaluations;
    bool at_least_one_success;
    double* best_state_vector;
    double best_objective_function;
    int best_function_evaluation;
    bool bound_constraints_set;
    double* lower_bounds;
    double* upper_bounds;
    clock_t start_time, best_time;
    Package* package;
    double* state_vector;
    bool centered_differences;
    double finite_difference_step_size;
    std::string output_filename;
    int max_function_evaluations;
    int verbose;
    void* user_data;
    MPI_Partition* mpi_partition;

    Problem_data(int);
    ~Problem_data();

    double optimize(MPI_Partition*);
    void objective_function_wrapper(const double*, double*, bool*); 
    void finite_difference_gradient(const double*, double*, double*);
  };

}

#endif
