#ifndef MANGO_H
#define MANGO_H

#include<mpi.h>
#include<string>

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
    NLOPT_LN_NELDERMEAD,
    NLOPT_LN_PRAXIS,
    NLOPT_LD_LBFGS,
    HOPSPACK,
    NUM_ALGORITHMS  /* Not an actual algorithm, just counting. */
  } algorithm_type;

  typedef struct {
    bool uses_derivatives;
    bool is_least_squares_algorithm;
    int package;
    std::string name_string;
  } algorithm_properties;

  void get_algorithm_properties(algorithm_type,algorithm_properties*);
  bool string_to_algorithm(std::string, algorithm_type*);

  class problem {
  private:
    MPI_Comm mpi_comm_world;
    MPI_Comm mpi_comm_worker_groups;
    MPI_Comm mpi_comm_group_leaders;
    algorithm_type algorithm;
    int N_procs_world;
    int mpi_rank_world;
    int N_procs_worker_groups;
    int mpi_rank_worker_groups;
    int N_procs_group_leaders;
    int mpi_rank_group_leaders;
    int worker_group;
    bool proc0_world;
    bool proc0_worker_groups;
    int N_worker_groups;
    bool least_squares;
  public:
    /*  problem() : N_worker_groups(987) {}; */
    problem();
    ~problem();
    void set_algorithm(algorithm_type);
    void set_algorithm(std::string);
    void read_input_file(std::string);
    void mpi_init(MPI_Comm);
    void optimize();
  };
}

#endif
