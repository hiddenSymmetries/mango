#ifndef MANGO_PACKAGE_GSL_H
#define MANGO_PACKAGE_GSL_H

#ifdef MANGO_GSL_AVAILABLE
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#endif

namespace mango {

  class Solver;
  class Least_squares_solver;

  class Package_gsl : public Package {
  private:
#ifdef MANGO_GSL_AVAILABLE
    static int gsl_residual_function(const gsl_vector*, void*, gsl_vector*);
    static int gsl_residual_function_and_Jacobian(const gsl_vector*, void*, gsl_matrix*);
    static double gsl_objective_function(const gsl_vector*, void*);
    static void gsl_gradient(const gsl_vector*, void*, gsl_vector*);
    static void gsl_objective_function_and_gradient(const gsl_vector*, void*, double*, gsl_vector*);
#endif

  public:
    void optimize(Solver*);
    void optimize_least_squares(Least_squares_solver*);
  };
}

#endif
