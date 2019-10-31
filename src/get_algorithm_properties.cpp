#include<iostream>
#include<stdlib.h>
#include "mango.hpp"

void mango::get_algorithm_properties(int algorithm, bool* algorithm_uses_derivatives, bool* least_squares_algorithm, package_type* package, std::string* algorithm_name) {

  switch (algorithm) {
  case PETSC_NM:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_PETSC;
    *algorithm_name = "petsc_nm";
    break;
  case PETSC_POUNDERS:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = true;
    *package = PACKAGE_PETSC;
    *algorithm_name = "petsc_pounders";
    break;

  case NLOPT_GN_DIRECT:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct";
    break;
  case NLOPT_GN_DIRECT_L:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l";
    break;
  case NLOPT_GN_DIRECT_L_RAND:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l_rand";
    break;
  case NLOPT_GN_DIRECT_NOSCAL:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_noscal";
    break;
  case NLOPT_GN_DIRECT_L_NOSCAL:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l_noscal";
    break;
  case NLOPT_GN_DIRECT_L_RAND_NOSCAL:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l_rand_noscal";
    break;
  case NLOPT_GN_ORIG_DIRECT:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_orig_direct";
    break;
  case NLOPT_GN_ORIG_DIRECT_L:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_orig_direct_l";
    break;
  case NLOPT_GN_CRS2_LM:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_crs2_lm";
    break;

  case NLOPT_LN_COBYLA:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_cobyla";
    break;
  case NLOPT_LN_BOBYQA:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_bobyqa";
    break;
  case NLOPT_LN_PRAXIS:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_praxis";
    break;
  case NLOPT_LN_NELDERMEAD:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_neldermead";
    break;
  case NLOPT_LN_SBPLX:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_sbplx";
    break;

  case NLOPT_LD_MMA:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_mma";
    break;
  case NLOPT_LD_CCSAQ:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_ccsaq";
    break;
  case NLOPT_LD_SLSQP:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_slsqp";
    break;
  case NLOPT_LD_LBFGS:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_lbfgs";
    break;
  case NLOPT_LD_TNEWTON_PRECOND_RESTART:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton_precond_restart";
    break;
  case NLOPT_LD_TNEWTON_PRECOND:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton_precond";
    break;
  case NLOPT_LD_TNEWTON_RESTART:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton_restart";
    break;
  case NLOPT_LD_TNEWTON:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton";
    break;
  case NLOPT_LD_VAR1:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_var1";
    break;
  case NLOPT_LD_VAR2:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_var2";
    break;

  case HOPSPACK:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_HOPSPACK;
    *algorithm_name = "hopspack";
    break;
  default:
    std::cout << "Error! Unrecognized algorithm\n";
    exit(1);
  }

}

//////////////////////////////////////////////////////////////////

void mango::problem::load_algorithm_properties() {
  get_algorithm_properties(algorithm, &algorithm_uses_derivatives, &least_squares_algorithm, &package, &algorithm_name);
}
