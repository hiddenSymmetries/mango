#ifndef MANGO_H
#define MANGO_H

#include<mpi.h>
#include<string>
#include<fstream>

#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

namespace mango {

  typedef enum {
    PACKAGE_MANGO,
    PACKAGE_PETSC,
    PACKAGE_NLOPT,
    PACKAGE_HOPSPACK,
    PACKAGE_GSL,
    NUM_PACKAGES /* Not an actual package, just counting. */
  } package_type;

  typedef enum {
    PETSC_NM,
    PETSC_POUNDERS,
    NLOPT_GN_DIRECT,
    NLOPT_GN_DIRECT_L,
    NLOPT_GN_DIRECT_L_RAND,
    NLOPT_GN_DIRECT_NOSCAL,
    NLOPT_GN_DIRECT_L_NOSCAL,
    NLOPT_GN_DIRECT_L_RAND_NOSCAL,
    NLOPT_GN_ORIG_DIRECT,
    NLOPT_GN_ORIG_DIRECT_L,
    NLOPT_GN_CRS2_LM,
    NLOPT_LN_COBYLA,
    NLOPT_LN_BOBYQA,
    NLOPT_LN_PRAXIS,
    NLOPT_LN_NELDERMEAD,
    NLOPT_LN_SBPLX,
    NLOPT_LD_MMA,
    NLOPT_LD_CCSAQ,
    NLOPT_LD_SLSQP,
    NLOPT_LD_LBFGS,
    NLOPT_LD_TNEWTON_PRECOND_RESTART,
    NLOPT_LD_TNEWTON_PRECOND,
    NLOPT_LD_TNEWTON_RESTART,
    NLOPT_LD_TNEWTON,
    NLOPT_LD_VAR1,
    NLOPT_LD_VAR2,
    HOPSPACK,
    NUM_ALGORITHMS  /* Not an actual algorithm, just counting. */
  } algorithm_type;

  const double NUMBER_FOR_FAILED = 1.0e+12;

  class problem;

  typedef void (*objective_function_type)(int*, const double*, double*, int*, mango::problem*);
  typedef void (*residual_function_type)(int*, const double*, int*, double*, int*, mango::problem*);

  bool does_algorithm_exist(std::string);
  void get_algorithm_properties(int, bool*, bool*, package_type*, std::string*, bool*);

  //////////////////////////////////////////////////////////////////////////////////////

  class MPI_Partition {
  private:
    MPI_Comm comm_world;
    MPI_Comm comm_worker_groups;
    MPI_Comm comm_group_leaders;
    int N_procs_world;
    int rank_world;
    int N_procs_worker_groups;
    int rank_worker_groups;
    int N_procs_group_leaders;
    int rank_group_leaders;
    int worker_group;
    bool proc0_world;
    bool proc0_worker_groups;
    int N_worker_groups;
    bool initialized;

    void verify_initialized();
    void print();

  public:
    MPI_Partition();
    ~MPI_Partition();
    void init(MPI_Comm);
    void set_custom(MPI_Comm, MPI_Comm, MPI_Comm);
    MPI_Comm get_comm_world();
    MPI_Comm get_comm_worker_groups();
    MPI_Comm get_comm_group_leaders();
    bool get_proc0_world();
    bool get_proc0_worker_groups();
    int get_rank_world();
    int get_rank_worker_groups();
    int get_rank_group_leaders();
    int get_N_procs_world();
    int get_N_procs_worker_groups();
    int get_N_procs_group_leaders();
    int get_worker_group();
    int get_N_worker_groups();
    void set_N_worker_groups(int);
  };

  //////////////////////////////////////////////////////////////////////////////////////

  class problem {
  private:
    algorithm_type algorithm;
    bool algorithm_uses_derivatives;
    bool algorithm_requires_bound_constraints;
    bool least_squares_algorithm;
    bool least_squares;
    package_type package;
    std::string algorithm_name;
    int N_parameters;
    int N_terms;
    objective_function_type objective_function;
    residual_function_type residual_function;
    int function_evaluations;
    std::ofstream output_file;
    int argc;
    char** argv;
    int max_function_and_gradient_evaluations;
    bool at_least_one_success;
    double* best_state_vector;
    double best_objective_function;
    double* best_residual_function;
    int best_function_evaluation;
    bool bound_constraints_set;
    double* lower_bounds;
    double* upper_bounds;

    void group_leaders_loop();
    void group_leaders_least_squares_loop();
    void optimize_least_squares();
    void defaults();
    void load_algorithm_properties();
    void optimize_petsc();
    void optimize_nlopt();
    void optimize_hopspack();
    void optimize_gsl();
    void optimize_least_squares_petsc();
    void optimize_least_squares_gsl();
    void write_file_line(const double*, double);
    double write_least_squares_file_line(const double*, double*);
    static double nlopt_objective_function(unsigned, const double*, double*, void*); 
#ifdef MANGO_PETSC_AVAILABLE
    static PetscErrorCode mango_petsc_objective_function(Tao, Vec, PetscReal*, void*);
    static PetscErrorCode mango_petsc_residual_function(Tao, Vec, Vec, void*);
#endif
    /*void objective_function_wrapper(const double*, double*, bool*); */
    void objective_function_wrapper(const double*, double*, bool*); 
    void residual_function_wrapper(const double*, double*, bool*); 
    static void least_squares_to_single_objective(int*, const double*, double*, int*, mango::problem*);
    void finite_difference_gradient(const double*, double*, double*);
    void finite_difference_Jacobian(const double*, double*, double*);
    void finite_difference_Jacobian_to_gradient(const double*, double*, double*);

  public:
    double* state_vector;
    double* targets;
    double* sigmas;
    bool centered_differences;
    double finite_difference_step_size;
    std::string output_filename;
    int max_function_evaluations;
    MPI_Partition mpi_partition;

    problem(int, double*, objective_function_type, int, char**); /* For non-least-squares problems */
    problem(int, double*, int, double*, double*, double*, residual_function_type, int, char**); /* For least-squares problems */
    ~problem();
    void mpi_init(MPI_Comm);
    void set_algorithm(algorithm_type);
    void set_algorithm(std::string);
    void read_input_file(std::string);
    void set_output_filename(std::string);
    void set_bound_constraints(double*, double*);
    double optimize();
    bool is_least_squares();
    int get_N_parameters();
    int get_N_terms();
    int get_best_function_evaluation();
    int get_function_evaluations();
  };
}

#endif
