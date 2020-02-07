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
#include <cassert>
#include <stdexcept>
#include "mango.hpp"
#include "Least_squares_solver.hpp"
#include "Package_gsl.hpp"

#ifdef MANGO_GSL_AVAILABLE
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlinear.h>
#endif

void mango::Package_gsl::optimize_least_squares(Least_squares_solver* solver) {
#ifdef MANGO_GSL_AVAILABLE
  if (solver->verbose>0) std::cout << "Hello from optimize_least_squares_gsl" << std::endl;

  gsl_vector *gsl_residual = gsl_vector_alloc(solver->N_terms);
  gsl_vector *gsl_state_vector = gsl_vector_alloc(solver->N_parameters);
  gsl_multifit_nlinear_fdf gsl_optimizer;
  gsl_multifit_nlinear_parameters gsl_optimizer_params = gsl_multifit_nlinear_default_parameters();

  gsl_optimizer.f = gsl_residual_function;
  gsl_optimizer.df = gsl_residual_function_and_Jacobian;
  //gsl_optimizer.fvv = func_fvv;
  gsl_optimizer.n = solver->N_terms;
  gsl_optimizer.p = solver->N_parameters;
  gsl_optimizer.params = (void*)solver;

  // Set initial condition
  for (int j=0; j<solver->N_parameters; j++) gsl_vector_set(gsl_state_vector, j, solver->state_vector[j]);

  switch (solver->algorithm) {
  case GSL_LM:
    gsl_optimizer_params.trs = gsl_multifit_nlinear_trs_lm;
    break;
  case GSL_DOGLEG:
    gsl_optimizer_params.trs = gsl_multifit_nlinear_trs_dogleg;
    break;
  case GSL_DDOGLEG:
    gsl_optimizer_params.trs = gsl_multifit_nlinear_trs_ddogleg;
    break;
  case GSL_SUBSPACE2D:
    gsl_optimizer_params.trs = gsl_multifit_nlinear_trs_subspace2D;
    break;
  default:
    throw std::runtime_error("Error! in optimize_least_squares_gsl.cpp switch! Should not get here!");
  }

  // Set other optimizer parameters
  gsl_optimizer_params.solver = gsl_multifit_nlinear_solver_svd; // This option is described in the documentation as the slowest but most robust for ill-conditioned problems. Since speed is not a major concern outside of the objective function, I'll opt for the extra robustness.
  const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
  const size_t max_iter = (solver->max_function_evaluations + solver->max_function_and_gradient_evaluations) / 2;
  const double xtol = 1.0e-8;
  const double gtol = 1.0e-8;
  const double ftol = 1.0e-8;
  gsl_multifit_nlinear_workspace *work = gsl_multifit_nlinear_alloc(T, &gsl_optimizer_params, solver->N_terms, solver->N_parameters);
  gsl_vector * f = gsl_multifit_nlinear_residual(work);
  gsl_vector * x = gsl_multifit_nlinear_position(work);
  int info;

  // Run the optimization
  gsl_multifit_nlinear_init(gsl_state_vector, &gsl_optimizer, work);
  gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, NULL, NULL, &info, work); // NULLs correspond to optional callback function

  gsl_multifit_nlinear_free(work);
  gsl_vector_free(gsl_residual);
  gsl_vector_free(gsl_state_vector);
  if (solver->verbose>0) std::cout << "Goodbye from optimize_least_squares_gsl" << std::endl;
#else
  throw std::runtime_error("Error! A GSL algorithm was requested, but Mango was compiled without GSL support.");
#endif
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifdef MANGO_GSL_AVAILABLE

int mango::Package_gsl::gsl_residual_function(const gsl_vector * x, void *params, gsl_vector * f) {
  Least_squares_solver* solver = (Least_squares_solver*)params;

  if (solver->verbose > 0) std::cout << "Hello from gsl_residual_function. f stride = " << f->stride << std::endl << std::flush;

  assert(solver->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride for f may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);

  bool failed;
  solver->residual_function_wrapper(x->data, solver->residuals, &failed);

  // GSL's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately:
  for (int j=0; j<solver->N_terms; j++) {
    gsl_vector_set(f, j, (solver->residuals[j] - solver->targets[j]) / solver->sigmas[j]);
    // Should handle the "failed" case
  }

  if (solver->verbose > 0) std::cout << "Goodbye from gsl_residual_function" << std::endl << std::flush;
  return GSL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int mango::Package_gsl::gsl_residual_function_and_Jacobian (const gsl_vector * x, void *params, gsl_matrix * J) {
  Least_squares_solver* solver = (Least_squares_solver*)params;

  if (solver->verbose > 0) std::cout << "Hello from gsl_residual_function_and_Jacobian" << std::endl << std::flush;

  assert(solver->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride for f may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);

  // For now, I'll allocate and de-allocate memory for the Jacobian on every call to this subroutine. It might be worth modifying things so this allocation is done only once.
  double* mango_Jacobian = new double[solver->N_parameters * solver->N_terms];
  solver->finite_difference_Jacobian(x->data, solver->residuals, mango_Jacobian);

  // It does not seem possible in GSL to get both the residual vector and Jacobian in a single subroutine call!
  // This is a major inefficiency!!!

  // Introduce shorthand:
  int N_parameters = solver->N_parameters;
  int N_terms = solver->N_terms;

  // GSL's definition of the residual function does not include sigmas or targets, so scale mango's Jacobian appropriately.
  // There is probably a faster approach than these explicit loops, but I'll worry about optimizing this later.
  for (int j_parameter = 0; j_parameter < N_parameters; j_parameter++) {
    for (int j_term = 0; j_term < N_terms; j_term++) {
      // row index is before column index in gsl_matrix_set
      // row index = term, column index = parameter
      gsl_matrix_set(J, j_term, j_parameter, mango_Jacobian[j_parameter*N_terms+j_term] / solver->sigmas[j_term]);
    }
  }

  delete[] mango_Jacobian;

  if (solver->verbose > 0) std::cout << "Goodbye from gsl_residual_function_and_Jacobian" << std::endl << std::flush;
  return GSL_SUCCESS;
}

#endif
