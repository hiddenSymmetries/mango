#ifndef MANGO_PROBLEM_DATA_H
#define MANGO_PROBLEM_DATA_H

#include <mpi.h>
#include <string>
#include <fstream>
#include <ctime>
#include "mango.hpp"
#include "Least_squares_data.hpp"

namespace mango {
  void finite_difference_gradient(objective_function_type, const MPI_Partition*, const double*, double*, double*);
  void finite_difference_Jacobian(residual_function_type, const MPI_Partition*, const double*, double*, double*);
  void finite_difference_Jacobian_to_gradient(Least_squares_data, const MPI_Partition&, const double*, double*, double*);
}

#endif
