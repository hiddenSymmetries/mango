#include "mango.h"

/* 
C interfaces to C++ subroutines 
See https://modelingguru.nasa.gov/docs/DOC-2642
and http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects
*/
extern "C" {
  mango::problem *mango_problem_create() {
    return new mango::problem;
  }
  void mango_problem_destroy(mango::problem *This) {
    delete This;
  }
  void mango_set_algorithm(mango::problem *This, mango::algorithm_type *algorithm) {
    This->set_algorithm(*algorithm);
  }
  void mango_mpi_init(mango::problem *This, MPI_Comm *comm) {
    This->mpi_init(*comm);
  }
}
