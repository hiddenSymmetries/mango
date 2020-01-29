#include <iostream>
#include <cassert>
#include <stdexcept>
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Least_squares_data.hpp"
#include "Package_gsl.hpp"

#ifdef MANGO_GSL_AVAILABLE
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlinear.h>
#endif

void mango::Package_gsl::optimize_least_squares(Problem_data* problem_data, Least_squares_data* least_squares_data) {
#ifdef MANGO_GSL_AVAILABLE
  if (problem_data->verbose>0) std::cout << "Hello from optimize_least_squares_gsl" << std::endl;

  gsl_vector *gsl_residual = gsl_vector_alloc(least_squares_data->N_terms);
  gsl_vector *gsl_state_vector = gsl_vector_alloc(problem_data->N_parameters);
  gsl_multifit_nlinear_fdf gsl_optimizer;
  gsl_multifit_nlinear_parameters gsl_optimizer_params = gsl_multifit_nlinear_default_parameters();

  gsl_optimizer.f = gsl_residual_function;
  gsl_optimizer.df = gsl_residual_function_and_Jacobian;
  //gsl_optimizer.fvv = func_fvv;
  gsl_optimizer.n = least_squares_data->N_terms;
  gsl_optimizer.p = problem_data->N_parameters;
  gsl_optimizer.params = (void*)least_squares_data;

  // Set initial condition
  for (int j=0; j<problem_data->N_parameters; j++) gsl_vector_set(gsl_state_vector, j, problem_data->state_vector[j]);

  switch (problem_data->algorithm) {
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
  const size_t max_iter = (problem_data->max_function_evaluations + problem_data->max_function_and_gradient_evaluations) / 2;
  const double xtol = 1.0e-8;
  const double gtol = 1.0e-8;
  const double ftol = 1.0e-8;
  gsl_multifit_nlinear_workspace *work = gsl_multifit_nlinear_alloc(T, &gsl_optimizer_params, least_squares_data->N_terms, problem_data->N_parameters);
  gsl_vector * f = gsl_multifit_nlinear_residual(work);
  gsl_vector * x = gsl_multifit_nlinear_position(work);
  int info;

  // Run the optimization
  gsl_multifit_nlinear_init(gsl_state_vector, &gsl_optimizer, work);
  gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, NULL, NULL, &info, work); // NULLs correspond to optional callback function

  gsl_multifit_nlinear_free(work);
  gsl_vector_free(gsl_residual);
  gsl_vector_free(gsl_state_vector);
  if (problem_data->verbose>0) std::cout << "Goodbye from optimize_least_squares_gsl" << std::endl;
#else
  throw std::runtime_error("Error! A GSL algorithm was requested, but Mango was compiled without GSL support.");
#endif
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifdef MANGO_GSL_AVAILABLE

int mango::Package_gsl::gsl_residual_function(const gsl_vector * x, void *params, gsl_vector * f) {
  Least_squares_data* least_squares_data = (Least_squares_data*)params;
  Problem_data* problem_data = least_squares_data->problem_data;

  if (problem_data->verbose > 0) std::cout << "Hello from gsl_residual_function. f stride = " << f->stride << std::endl << std::flush;

  assert(problem_data->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride for f may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);

  bool failed;
  least_squares_data->residual_function_wrapper(x->data, least_squares_data->residuals, &failed);

  // GSL's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately:
  for (int j=0; j<least_squares_data->N_terms; j++) {
    gsl_vector_set(f, j, (least_squares_data->residuals[j] - least_squares_data->targets[j]) / least_squares_data->sigmas[j]);
    // Should handle the "failed" case
  }

  if (problem_data->verbose > 0) std::cout << "Goodbye from gsl_residual_function" << std::endl << std::flush;
  return GSL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int mango::Package_gsl::gsl_residual_function_and_Jacobian (const gsl_vector * x, void *params, gsl_matrix * J) {
  Least_squares_data* least_squares_data = (Least_squares_data*)params;
  Problem_data* problem_data = least_squares_data->problem_data;

  if (problem_data->verbose > 0) std::cout << "Hello from gsl_residual_function_and_Jacobian" << std::endl << std::flush;

  assert(problem_data->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride for f may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);

  // For now, I'll allocate and de-allocate memory for the Jacobian on every call to this subroutine. It might be worth modifying things so this allocation is done only once.
  double* mango_Jacobian = new double[problem_data->N_parameters * least_squares_data->N_terms];
  least_squares_data->finite_difference_Jacobian(x->data, least_squares_data->residuals, mango_Jacobian);

  // It does not seem possible in GSL to get both the residual vector and Jacobian in a single subroutine call!
  // This is a major inefficiency!!!

  // Introduce shorthand:
  int N_parameters = problem_data->N_parameters;
  int N_terms = least_squares_data->N_terms;

  // GSL's definition of the residual function does not include sigmas or targets, so scale mango's Jacobian appropriately.
  // There is probably a faster approach than these explicit loops, but I'll worry about optimizing this later.
  for (int j_parameter = 0; j_parameter < N_parameters; j_parameter++) {
    for (int j_term = 0; j_term < N_terms; j_term++) {
      // row index is before column index in gsl_matrix_set
      // row index = term, column index = parameter
      gsl_matrix_set(J, j_term, j_parameter, mango_Jacobian[j_parameter*N_terms+j_term] / least_squares_data->sigmas[j_term]);
    }
  }

  delete[] mango_Jacobian;

  if (problem_data->verbose > 0) std::cout << "Goodbye from gsl_residual_function_and_Jacobian" << std::endl << std::flush;
  return GSL_SUCCESS;
}

#endif
