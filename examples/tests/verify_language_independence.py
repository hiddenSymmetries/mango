#!/usr/bin/env python3

# For each example that has both a C++ and fortran version, this script verifies that the results are independent of the language.

try:
    # This case works when this script is run standalone
    from compare_2_examples import *
except:
    # This case works when this module is imported from another script
    from tests.compare_2_examples import *

def verify_language_independence(examples_run=0):
    compare_2_examples('chwirut_c','chwirut_f',examples_run=examples_run)
    compare_2_examples('quadratic_c','quadratic_f',examples_run=examples_run)
    compare_2_examples('rosenbrock_c','rosenbrock_f',examples_run=examples_run)
    compare_2_examples('nondifferentiable_c','nondifferentiable_f',examples_run=examples_run)

# Allow this module to be run as a standalone script.
if __name__ == '__main__':
    verify_language_independence()
