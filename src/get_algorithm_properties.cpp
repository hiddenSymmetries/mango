#include<iostream>
#include "mango.h"

void mango::get_algorithm_properties(algorithm_type algorithm, algorithm_properties* properties) {

  switch (algorithm) {
  case PETSC_NM:
    properties->uses_derivatives = false;
    properties->is_least_squares_algorithm = false;
    properties->package = PACKAGE_PETSC;
    properties->name_string = "petsc_nm";
    break;
  case PETSC_POUNDERS:
    properties->uses_derivatives = false;
    properties->is_least_squares_algorithm = true;
    properties->package = PACKAGE_PETSC;
    properties->name_string = "petsc_pounders";
    break;
  case NLOPT_LN_NELDERMEAD:
    properties->uses_derivatives = false;
    properties->is_least_squares_algorithm = false;
    properties->package = PACKAGE_NLOPT;
    properties->name_string = "nlopt_ln_neldermead";
    break;
  case NLOPT_LN_PRAXIS:
    properties->uses_derivatives = false;
    properties->is_least_squares_algorithm = false;
    properties->package = PACKAGE_NLOPT;
    properties->name_string = "nlopt_ln_praxis";
    break;
  case NLOPT_LD_LBFGS:
    properties->uses_derivatives = true;
    properties->is_least_squares_algorithm = false;
    properties->package = PACKAGE_NLOPT;
    properties->name_string = "nlopt_ld_lbfgs";
    break;
  case HOPSPACK:
    properties->uses_derivatives = false;
    properties->is_least_squares_algorithm = false;
    properties->package = PACKAGE_HOPSPACK;
    properties->name_string = "hopspack";
    break;
  default:
    std::cout << "Error! Unrecognized algorithm\n";
    exit(1);
  }

}
