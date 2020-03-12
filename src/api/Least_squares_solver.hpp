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

#ifndef MANGO_LEAST_SQUARES_SOLVER_H
#define MANGO_LEAST_SQUARES_SOLVER_H

#include "mango.hpp"
#include "Solver.hpp"

namespace mango {

  // This class contains ugly implementation details that are specific to least-squares minimization.
  class Least_squares_solver : public Solver {

  protected:
    Least_squares_solver(); // This version of the constructor, with no arguments, is used only for unit testing. Protected so Catch2 can subclass it.

    // Overrides the routines in Solver:
    void group_leaders_loop();

    // Many of the solver variables and method are public because this information must be used by the concrete Package.
  public:
    void* original_user_data;
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
    void objective_function_wrapper(const double*, double*, bool*); 
    bool record_function_evaluation(const double*, double, bool); // Overrides Solver.

    // Methods that do not exist in the base class Solver:
    double residuals_to_single_objective(double*);
    void residual_function_wrapper(const double*, double*, bool*);
    void finite_difference_Jacobian(const double*, double*, double*);
    static void least_squares_to_single_objective(int*, const double*, double*, int*, mango::Problem*, void*);
    void record_function_evaluation(const double*, double*, bool);// Not an override! Note double* instead of double in 2nd argument, and return void instead of bool.
  };
}

#endif
