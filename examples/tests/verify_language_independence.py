#!/usr/bin/env python3

# For each example that has both a C++ and fortran version, this script verifies that the results are independent of the language.

try:
    # This case works when this script is run standalone
    from compare_2_examples import *
except:
    # This case works when this module is imported from another script
    from tests.compare_2_examples import *

def verify_language_independence():

    base_algorithms = [\
            'petsc_nm',\
            'nlopt_ln_bobyqa',\
            'nlopt_ln_neldermead',\
            'nlopt_ld_mma',\
            'nlopt_ld_ccsaq',\
            'nlopt_ld_slsqp',\
            'nlopt_ld_lbfgs',\
            'nlopt_ld_tnewton_precond_restart',\
            'nlopt_ld_tnewton_precond',\
            'nlopt_ld_tnewton_restart',\
            'nlopt_ld_tnewton',\
            'nlopt_ld_var1',\
            'nlopt_ln_sbplx',
            'nlopt_ld_var2']

    # Do not include the 'rand' algorithms, praxis, nlopt_gn_crs2_lm, or petsc_pounders, since they are not deterministic
    extra_algorithms = [\
        'nlopt_gn_direct',\
            'nlopt_gn_direct_l',\
            'nlopt_gn_direct_noscal',\
            'nlopt_gn_direct_l_noscal',\
            'nlopt_gn_orig_direct',\
            'nlopt_gn_orig_direct_l']

    least_squares_algorithms = base_algorithms

    compare_2_examples('quadratic_c','quadratic_f',least_squares_algorithms + extra_algorithms)
    compare_2_examples('rosenbrock_c','rosenbrock_f',least_squares_algorithms)

    algorithms = base_algorithms + ['blorp','foo','glurb']

    compare_2_examples('nondifferentiable_c','nondifferentiable_f',algorithms)

# Allow this module to be run as a standalone script.
if __name__ == '__main__':
    verify_language_independence()
