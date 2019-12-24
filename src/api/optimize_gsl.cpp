#include <iostream>
#include <cassert>
#include <stdexcept>
#include "mango.hpp"

#ifdef MANGO_GSL_AVAILABLE
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multimin.h>
#endif

void mango::problem::optimize_gsl() {
#ifdef MANGO_GSL_AVAILABLE
  if (verbose>0) std::cout << "Hello from optimize_gsl" << std::endl;

  // Set initial condition
  gsl_vector *gsl_state_vector = gsl_vector_alloc(N_parameters);
  for (int j=0; j<N_parameters; j++) gsl_vector_set(gsl_state_vector, j, state_vector[j]); // Could be faster

  int iterations = 0;

  if (algorithms[algorithm].uses_derivatives) {
    ///////////////////////////////////////////////////////////////////////////////
    // Derivative-based algorithms:
    ///////////////////////////////////////////////////////////////////////////////

    const gsl_multimin_fdfminimizer_type *Tfdf;
    switch (algorithm) {
    case GSL_CONJUGATE_FR:
      Tfdf = gsl_multimin_fdfminimizer_conjugate_fr;
      break;
    case GSL_CONJUGATE_PR:
      Tfdf = gsl_multimin_fdfminimizer_conjugate_pr;
      break;
    case GSL_BFGS:
      // The GSL documentation indicates that gsl_multimin_fdfminimizer_bfgs2 supercedes gsl_multimin_fdfminimizer_bfgs.
      Tfdf = gsl_multimin_fdfminimizer_vector_bfgs2;
      break;
    default:
      throw std::runtime_error("Error in optimize_gsl.cpp switch 1! Should not get here!");
    }

    gsl_multimin_fdfminimizer* fdfminimizer = gsl_multimin_fdfminimizer_alloc(Tfdf, N_parameters);
    gsl_multimin_function_fdf fdf_parameters;
    fdf_parameters.f = &mango::problem::gsl_objective_function;
    fdf_parameters.df = &mango::problem::gsl_gradient;
    fdf_parameters.fdf = &mango::problem::gsl_objective_function_and_gradient;
    fdf_parameters.n = N_parameters;
    fdf_parameters.params = (void*)this;

    double step_size = 0.01;
    double line_search_tolerance = 0.1;
    gsl_multimin_fdfminimizer_set(fdfminimizer, &fdf_parameters, gsl_state_vector, step_size, line_search_tolerance);

    int status;
    do {
      iterations++;
      status = gsl_multimin_fdfminimizer_iterate(fdfminimizer); // Take a step.
      if (status) break;
      status = gsl_multimin_test_gradient (fdfminimizer->gradient, 1e-5); // Need to make this tolerance a variable
    } while (status == GSL_CONTINUE && function_evaluations < max_function_evaluations);

    gsl_multimin_fdfminimizer_free(fdfminimizer);

  } else {
    ///////////////////////////////////////////////////////////////////////////////
    // Derivative-free algorithms
    ///////////////////////////////////////////////////////////////////////////////

    const gsl_multimin_fminimizer_type *Tf;
    switch (algorithm) {
    case GSL_NM:
      Tf = gsl_multimin_fminimizer_nmsimplex2;
      break;
    default:
      throw std::runtime_error("Error in optimize_gsl.cpp switch 2! Should not get here!");
    }
    gsl_multimin_fminimizer* fminimizer = gsl_multimin_fminimizer_alloc(Tf, N_parameters);
    gsl_multimin_function f_parameters;
    f_parameters.f = &gsl_objective_function;
    f_parameters.n = N_parameters;
    f_parameters.params = (void*)this;

    // Set initial step sizes
    gsl_vector* step_sizes;
    step_sizes = gsl_vector_alloc(N_parameters);
    gsl_vector_set_all(step_sizes, 0.1); // I should make a smarter choice for the value here.

    gsl_multimin_fminimizer_set(fminimizer, &f_parameters, gsl_state_vector, step_sizes);

    int status;
    double size;
    do {
      iterations++;
      status = gsl_multimin_fminimizer_iterate(fminimizer);
      if (status) break;
      size = gsl_multimin_fminimizer_size (fminimizer);
      status = gsl_multimin_test_size (size, 1e-6); // This tolerance should be changed into a variable.
    } while (status == GSL_CONTINUE && function_evaluations < max_function_evaluations);

    gsl_vector_free(step_sizes);
    gsl_multimin_fminimizer_free(fminimizer);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // End of separate blocks for derivative-based vs derivative-free algorithms.

  gsl_vector_free(gsl_state_vector);
  if (verbose>0) std::cout << "Goodbye from optimize_gsl" << std::endl;
#else
  throw std::runtime_error("Error! A GSL algorithm was requested, but Mango was compiled without GSL support.");
#endif
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#ifdef MANGO_GSL_AVAILABLE

double mango::problem::gsl_objective_function(const gsl_vector * x, void *params) {
  mango::problem* this_problem = (mango::problem*) params;

  if (this_problem->verbose > 0) std::cout << "Hello from gsl_objective_function." << std::endl << std::flush;

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride for f may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);

  double f;
  bool failed;
  this_problem->objective_function_wrapper(x->data, &f, &failed);

  if (this_problem->verbose > 0) std::cout << "Goodbye from gsl_objective_function" << std::endl << std::flush;
  if (failed) {
    return GSL_NAN;
  } else {
    return f;
  }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void mango::problem::gsl_objective_function_and_gradient(const gsl_vector * x, void *params, double* f, gsl_vector* gradient) {
  mango::problem* this_problem = (mango::problem*) params;
  if (this_problem->verbose > 0) std::cout << "Hello from gsl_objective_function_and_gradient" << std::endl << std::flush;

  // gsl vectors have a 'stride'. Only if the stride is 1 does the layout of a gsl vector correspond to a standard double array.
  // Curran pointed out that the stride may not be 1!
  // See https://github.com/PrincetonUniversity/STELLOPT/commit/5820c453283785ffd97e40aec261ca97f76e9071
  assert(x->stride == 1);
  assert(gradient->stride == 1);

  this_problem->finite_difference_gradient(x->data, f, gradient->data);

  if (this_problem->verbose > 0) std::cout << "Goodbye from gsl_objective_function_and_gradient" << std::endl << std::flush;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void mango::problem::gsl_gradient(const gsl_vector * x, void *params, gsl_vector* gradient) {
  mango::problem* this_problem = (mango::problem*) params;
  if (this_problem->verbose > 0) std::cout << "Hello from gsl_gradient" << std::endl << std::flush;
  double f;
  // Just call objective_function_and_gradient and throw away the objective function.
  mango::problem::gsl_objective_function_and_gradient(x, params, &f, gradient);
}

#endif
