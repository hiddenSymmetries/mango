#include <iostream>
#include <cassert>
#include <stdexcept>
#include "mango.hpp"

#ifdef MANGO_GSL_AVAILABLE
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlinear.h>
#endif

void mango::problem::optimize_least_squares_gsl() {
#ifdef MANGO_GSL_AVAILABLE
  if (verbose>0) std::cout << "Hello from optimize_least_squares_gsl" << std::endl;

  gsl_vector *gsl_residual = gsl_vector_alloc(N_terms);
  gsl_vector *gsl_state_vector = gsl_vector_alloc(N_parameters);
  gsl_multifit_nlinear_fdf gsl_optimizer;
  gsl_multifit_nlinear_parameters gsl_optimizer_params = gsl_multifit_nlinear_default_parameters();

  gsl_optimizer.f = gsl_residual_function;
  gsl_optimizer.df = gsl_residual_function_and_Jacobian;
  //gsl_optimizer.fvv = func_fvv;
  gsl_optimizer.n = N_terms;
  gsl_optimizer.p = N_parameters;
  gsl_optimizer.params = (void*)this;

  // Set initial condition
  for (int j=0; j<N_parameters; j++) gsl_vector_set(gsl_state_vector, j, state_vector[j]);

  switch (algorithm) {
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
  const size_t max_iter = (max_function_evaluations + max_function_and_gradient_evaluations) / 2;
  const double xtol = 1.0e-8;
  const double gtol = 1.0e-8;
  const double ftol = 1.0e-8;
  gsl_multifit_nlinear_workspace *work = gsl_multifit_nlinear_alloc(T, &gsl_optimizer_params, N_terms, N_parameters);
  gsl_vector * f = gsl_multifit_nlinear_residual(work);
  gsl_vector * x = gsl_multifit_nlinear_position(work);
  int info;

  // Run the optimization
  gsl_multifit_nlinear_init(gsl_state_vector, &gsl_optimizer, work);
  gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, NULL, NULL, &info, work); // NULLs correspond to optional callback function

  gsl_multifit_nlinear_free(work);
  gsl_vector_free(gsl_residual);
  gsl_vector_free(gsl_state_vector);
  if (verbose>0) std::cout << "Goodbye from optimize_least_squares_gsl" << std::endl;
#else
  throw std::runtime_error("Error! A GSL algorithm was requested, but Mango was compiled without GSL support.");
#endif
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifdef MANGO_GSL_AVAILABLE

int mango::problem::gsl_residual_function(const gsl_vector * x, void *params, gsl_vector * f) {
  mango::problem* this_problem = (mango::problem*) params;

  if (this_problem->verbose > 0) std::cout << "Hello from gsl_residual_function. f stride = " << f->stride << std::endl << std::flush;

  assert(this_problem->mpi_partition.get_proc0_world()); // This subroutine should only ever be called by proc 0.

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride for f may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);

  bool failed;
  this_problem->residual_function_wrapper(x->data, this_problem->residuals, &failed);

  // GSL's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately:
  for (int j=0; j<this_problem->N_terms; j++) {
    gsl_vector_set(f, j, (this_problem->residuals[j] - this_problem->targets[j]) / this_problem->sigmas[j]);
    // Should handle the "failed" case
  }

  if (this_problem->verbose > 0) std::cout << "Goodbye from gsl_residual_function" << std::endl << std::flush;
  return GSL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int mango::problem::gsl_residual_function_and_Jacobian (const gsl_vector * x, void *params, gsl_matrix * J) {
  mango::problem* this_problem = (mango::problem*) params;
  if (this_problem->verbose > 0) std::cout << "Hello from gsl_residual_function_and_Jacobian" << std::endl << std::flush;

  assert(this_problem->mpi_partition.get_proc0_world()); // This subroutine should only ever be called by proc 0.

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride for f may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);

  // For now, I'll allocate and de-allocate memory for the Jacobian on every call to this subroutine. It might be worth modifying things so this allocation is done only once.
  double* mango_Jacobian = new double[this_problem->N_parameters * this_problem->N_terms];
  this_problem->finite_difference_Jacobian(x->data, this_problem->residuals, mango_Jacobian);

  // It does not seem possible in GSL to get both the residual vector and Jacobian in a single subroutine call!
  // This is a major inefficiency!!!

  // Introduce shorthand:
  int N_parameters = this_problem->N_parameters;
  int N_terms = this_problem->N_terms;

  // GSL's definition of the residual function does not include sigmas or targets, so scale mango's Jacobian appropriately.
  // There is probably a faster approach than these explicit loops, but I'll worry about optimizing this later.
  for (int j_parameter = 0; j_parameter < N_parameters; j_parameter++) {
    for (int j_term = 0; j_term < N_terms; j_term++) {
      // row index is before column index in gsl_matrix_set
      // row index = term, column index = parameter
      gsl_matrix_set(J, j_term, j_parameter, mango_Jacobian[j_parameter*N_terms+j_term] / this_problem->sigmas[j_term]);
    }
  }

  delete[] mango_Jacobian;

  if (this_problem->verbose > 0) std::cout << "Goodbye from gsl_residual_function_and_Jacobian" << std::endl << std::flush;
  return GSL_SUCCESS;
}

#endif
