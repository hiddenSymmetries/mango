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
    Solver(); // This version of the constructor, with no arguments, is used only for unit testing.
    virtual void group_leaders_loop();
    virtual void set_package();

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
    virtual bool record_function_evaluation(const double*, double, bool);
  };

}

#endif
