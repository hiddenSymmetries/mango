#include<iostream>
#include "mango.hpp"

void mango::problem::get_algorithm_properties() {

  switch (algorithm) {
  case PETSC_NM:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_PETSC;
    algorithm_name = "petsc_nm";
    break;
  case PETSC_POUNDERS:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = true;
    package = PACKAGE_PETSC;
    algorithm_name = "petsc_pounders";
    break;
  case NLOPT_LN_NELDERMEAD:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ln_neldermead";
    break;
  case NLOPT_LN_PRAXIS:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ln_praxis";
    break;
  case NLOPT_LD_LBFGS:
    algorithm_uses_derivatives = true;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ld_lbfgs";
    break;
  case HOPSPACK:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_HOPSPACK;
    algorithm_name = "hopspack";
    break;
  default:
    std::cout << "Error! Unrecognized algorithm\n";
    exit(1);
  }

}
