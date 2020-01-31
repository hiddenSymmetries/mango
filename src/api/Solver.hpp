#ifndef MANGO_SOLVER_H
#define MANGO_SOLVER_H

#include <mpi.h>
#include <string>
#include <ctime>
#include "mango.hpp"
#include "Package.hpp"
#include "Recorder.hpp"

namespace mango {

  class Solver {
    // This class contains the ugly implementation details of the interface for Problem specified in mango.hpp.
    // All methods are "virtual" so they can be over-ridden in subclasses of Solver that replace the solver in Problem.
  protected:
    virtual void group_leaders_loop();
    virtual void set_package();
    virtual bool record_function_evaluation(const double*, double, bool);

    //virtual void load_algorithm_properties();
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
    Problem* problem;
    Recorder* recorder;

    Solver(Problem*, int);
    ~Solver();

    virtual double optimize(MPI_Partition*);
    virtual void init_optimization();
    virtual void objective_function_wrapper(const double*, double*, bool*); 
    virtual void finite_difference_gradient(const double*, double*, double*);
    //virtual void compose_x_f_string(std::string&, const double*, double);
    //virtual void write_file_line(clock_t, const double*, double);
    //virtual void write_function_evaluation_and_time(clock_t);
  };

}

#endif
