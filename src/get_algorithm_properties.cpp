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

  case NLOPT_GN_DIRECT:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_direct";
    break;
  case NLOPT_GN_DIRECT_L:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_direct_l";
    break;
  case NLOPT_GN_DIRECT_L_RAND:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_direct_l_rand";
    break;
  case NLOPT_GN_DIRECT_NOSCAL:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_direct_noscal";
    break;
  case NLOPT_GN_DIRECT_L_NOSCAL:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_direct_l_noscal";
    break;
  case NLOPT_GN_DIRECT_L_RAND_NOSCAL:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_direct_l_rand_noscal";
    break;
  case NLOPT_GN_ORIG_DIRECT:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_orig_direct";
    break;
  case NLOPT_GN_ORIG_DIRECT_L:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_orig_direct_l";
    break;
  case NLOPT_GN_CRS2_LM:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_gn_crs2_lm";
    break;

  case NLOPT_LN_COBYLA:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ln_cobyla";
    break;
  case NLOPT_LN_BOBYQA:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ln_bobyqa";
    break;
  case NLOPT_LN_PRAXIS:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ln_praxis";
    break;
  case NLOPT_LN_NELDERMEAD:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ln_neldermead";
    break;
  case NLOPT_LN_SBPLX:
    algorithm_uses_derivatives = false;
    least_squares_algorithm = false;
    package = PACKAGE_NLOPT;
    algorithm_name = "nlopt_ln_sbplx";
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
