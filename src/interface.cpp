#include "mango.h"

#define mango_interface_string_length 128

/* 
C interfaces to C++ subroutines 
See https://modelingguru.nasa.gov/docs/DOC-2642
and http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects
*/
extern "C" {
  /*  mango::problem *mango_problem_create() {
    return new mango::problem;
    } */
  /*
  mango::problem *mango_problem_create(int N_parameters) {
    return new mango::problem(N_parameters);
  }
  mango::problem *mango_problem_create_least_squares(int N_parameters, int N_terms) {
    return new mango::problem(N_parameters,N_terms);
    } */
  mango::problem *mango_problem_create(int N_parameters, double* state_vector) {
    return new mango::problem(N_parameters, state_vector);
  }
  mango::problem *mango_problem_create_least_squares(int N_parameters, double* state_vector, int N_terms, double* targets, double* sigmas) {
    return new mango::problem(N_parameters, state_vector, N_terms, targets, sigmas);
  }
  void mango_problem_destroy(mango::problem *This) {
    delete This;
  }
  void mango_set_algorithm(mango::problem *This, mango::algorithm_type *algorithm) {
    This->set_algorithm(*algorithm);
  }
  //  void mango_set_algorithm_from_string(mango::problem *This, char algorithm_name[mango_interface_string_length], int string_length) {
    /* string_length is added as an argument when passing strings between fortran and C */
  void mango_set_algorithm_from_string(mango::problem *This, char algorithm_name[mango_interface_string_length]) {
    This->set_algorithm(algorithm_name);
  }
  void mango_read_input_file(mango::problem *This, char filename[mango_interface_string_length]) {
    This->read_input_file(filename);
  }
  void mango_mpi_init(mango::problem *This, MPI_Comm *comm) {
    This->mpi_init(*comm);
  }
}
