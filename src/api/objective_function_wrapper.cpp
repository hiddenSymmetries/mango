// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctime>
#include <sstream>
#include "mango.hpp"
#include "Solver.hpp"

void mango::Solver::objective_function_wrapper(const double* x, double* f, bool* failed) {
  if (verbose > 0) std::cout << "Hello from objective_function_wrapper" << std::endl;

  int failed_int = 123;
  std::cout << "In mango::Solver::objective_function_wrapper, problem = " << problem << std::endl;
  objective_function(&N_parameters, x, f, &failed_int, problem, user_data);
  *failed = (failed_int != 0);

  if (verbose > 0) std::cout << " objective_function_wrapper: *failed=" << *failed << " at_least_one_success=" << at_least_one_success 
			     << ", *f < best_objective_function=" << (*f < best_objective_function) << std::endl;

  record_function_evaluation(x, *f, *failed);
}


bool mango::Solver::record_function_evaluation(const double* x, double f, bool failed) {
  if (verbose > 0) std::cout << "Hello from Solver::record_function_evaluation" << std::endl;

  function_evaluations++;

  clock_t now = clock();

  bool new_optimum = false;
  if (!failed && (!at_least_one_success || f < best_objective_function)) {
    new_optimum = true;
    at_least_one_success = true;
    best_objective_function = f;
    best_function_evaluation = function_evaluations;
    memcpy(best_state_vector, x, N_parameters * sizeof(double));
    best_time = now;
  }

  if (mpi_partition->get_proc0_world()) recorder->record_function_evaluation(function_evaluations, now, x, f);

  return new_optimum;
}


