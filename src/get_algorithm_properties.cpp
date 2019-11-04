#include<iostream>
#include "mango.hpp"

void mango::get_algorithm_properties(int algorithm, bool* algorithm_uses_derivatives, bool* least_squares_algorithm, package_type* package, std::string* algorithm_name, bool* bound_constraints_required) {

  switch (algorithm) {
  case PETSC_NM:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_PETSC;
    *algorithm_name = "petsc_nm";
    *bound_constraints_required = false;
    break;
  case PETSC_POUNDERS:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = true;
    *package = PACKAGE_PETSC;
    *algorithm_name = "petsc_pounders";
    *bound_constraints_required = false;
    break;

  case NLOPT_GN_DIRECT:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_DIRECT_L:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_DIRECT_L_RAND:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l_rand";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_DIRECT_NOSCAL:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_noscal";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_DIRECT_L_NOSCAL:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l_noscal";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_DIRECT_L_RAND_NOSCAL:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_direct_l_rand_noscal";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_ORIG_DIRECT:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_orig_direct";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_ORIG_DIRECT_L:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_orig_direct_l";
    *bound_constraints_required = true;
    break;
  case NLOPT_GN_CRS2_LM:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_gn_crs2_lm";
    *bound_constraints_required = true;
    break;

  case NLOPT_LN_COBYLA:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_cobyla";
    *bound_constraints_required = false;
    break;
  case NLOPT_LN_BOBYQA:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_bobyqa";
    *bound_constraints_required = false;
    break;
  case NLOPT_LN_PRAXIS:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_praxis";
    *bound_constraints_required = false;
    break;
  case NLOPT_LN_NELDERMEAD:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_neldermead";
    *bound_constraints_required = false;
    break;
  case NLOPT_LN_SBPLX:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ln_sbplx";
    *bound_constraints_required = false;
    break;

  case NLOPT_LD_MMA:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_mma";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_CCSAQ:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_ccsaq";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_SLSQP:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_slsqp";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_LBFGS:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_lbfgs";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_TNEWTON_PRECOND_RESTART:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton_precond_restart";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_TNEWTON_PRECOND:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton_precond";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_TNEWTON_RESTART:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton_restart";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_TNEWTON:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_tnewton";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_VAR1:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_var1";
    *bound_constraints_required = false;
    break;
  case NLOPT_LD_VAR2:
    *algorithm_uses_derivatives = true;
    *least_squares_algorithm = false;
    *package = PACKAGE_NLOPT;
    *algorithm_name = "nlopt_ld_var2";
    *bound_constraints_required = false;
    break;

  case HOPSPACK:
    *algorithm_uses_derivatives = false;
    *least_squares_algorithm = false;
    *package = PACKAGE_HOPSPACK;
    *algorithm_name = "hopspack";
    *bound_constraints_required = true;
    break;
  default:
    throw std::runtime_error("Error! Unrecognized algorithm");
  }

}

//////////////////////////////////////////////////////////////////

void mango::problem::load_algorithm_properties() {
  get_algorithm_properties(algorithm, &algorithm_uses_derivatives, &least_squares_algorithm, &package, &algorithm_name, &algorithm_requires_bound_constraints);
}
