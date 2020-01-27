#ifndef MANGO_PACKAGE_PETSC_H
#define MANGO_PACKAGE_PETSC_H

#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

namespace mango {

  class Problem_data;
  class Least_squares_data;

  class Package_petsc : public Package {
  private:
#ifdef MANGO_PETSC_AVAILABLE
    static PetscErrorCode mango_petsc_objective_function(Tao, Vec, PetscReal*, void*);
    static PetscErrorCode mango_petsc_residual_function(Tao, Vec, Vec, void*);
    static PetscErrorCode mango_petsc_Jacobian_function(Tao, Vec, Mat, Mat, void*);
#endif

  public:
    void optimize(Problem_data*);
    void optimize_least_squares(Problem_data*, Least_squares_data*);
  };
}

#endif
