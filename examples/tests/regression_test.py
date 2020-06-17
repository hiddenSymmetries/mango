#!/usr/bin/env python3

# This module checks a single example's short_summary file to verify
# that results match those in the corresponding short_summary.*.reference file.
# The algorithms in the two files do not need to be identical, and they can be
# in different orders. The one requirement is that every algorithm in the reference file
# must appear somewhere in the new short_summary file.

try:
    # This case works when this script is run standalone
    from nondeterministic_algorithms import *
except:
    # This case works when this module is imported from another script
    from tests.nondeterministic_algorithms import *

def regression_test(example_name, packages_available = ['hopspack','nlopt','mango','petsc','gsl']):
    print('Hello from '+__file__+'. Comparing short_summary.'+example_name+' with short_summary.'+example_name+'.reference.')
    import numpy as np

    # First, open the short_summary.* file
    big_filename = 'output/short_summary.'+example_name
    bigger_filename = big_filename
    import os
    if not os.path.isfile(big_filename):
        # Try another possible path to the file
        bigger_filename = '../'+big_filename
    try:
        f = open(bigger_filename)
    except:
        print('Error! Unable to open file '+big_filename)
        raise
    lines = f.readlines()
    f.close()

    # Now open the short_summary.*.reference file
    reference_filename = bigger_filename + '.reference'
    try:
        f = open(reference_filename)
    except:
        print('Error! Unable to open file '+reference_filename)
        raise
    reference_lines = f.readlines()
    f.close()

    try:
        N_parameters = int(reference_lines[3])
    except:
        print('Error reading N_parameters on line 4 of the reference file. The format of this file is probably wrong.')
        print(reference_lines[3])
        raise

    def algorithm_available(algorithm):
        # This subroutine takes an algorithm name and checks it against the list of available packages.
        for j in range(len(packages_available)):
            package = packages_available[j]
            if package == algorithm[:len(package)]:
                return True
        return False

    for index in range(5, len(reference_lines)):
        # Skip empty lines, as might occur at the end of the file
        if len(reference_lines[index].strip()) == 0:
            continue
        # Skip any comment lines
        firstchar = reference_lines[index].strip()[0]
        if firstchar=='#' or firstchar == '!':
            print('  Skipping line of reference file since firstchar=',firstchar)
            continue
        splitline_reference = reference_lines[index].split(',')
        algorithm = splitline_reference[0].strip()
        if not algorithm_available(algorithm):
            print('  Skipping algorithm '+algorithm+' since we are ignoring its package')
            continue
        print('  Examining algorithm '+algorithm)

        # Now find the corresponding line in the new short_summary file
        found_match = False
        for index1 in range(5,len(lines)):
            splitline = lines[index1].split(',')
            if len(splitline) > 0 and splitline[0].strip() == algorithm:
                found_match = True
                break
        if not found_match:
            print('Error! Algorithm '+algorithm+' is present in the reference file but not in the new short_summary file')
            exit(1)

        # If there is a failure for this algorithm in both the reference file and in the new short_summary file, count that as a match
        temp = splitline[-1].strip()
        if temp == 'FAILED' and splitline_reference[-1].strip() == temp:
            print('    Failures in both the new and reference files count as a match')
            continue

        expected_entries = N_parameters + 8
        if len(splitline_reference) != expected_entries:
            print('Number of elements is wrong on line '+str(index+1)+' of reference file. Expected '+str(expected_entries)+', found '+str(len(splitline_reference)))
            print(splitline_reference)
            exit(1)

        # If algorithm is deterministic, check that the number of function evaluations matches
        deterministic = True
        for algorithm1 in nondeterministic_algorithms:
            if algorithm == algorithm1:
                deterministic = False
                continue
        if deterministic:
            # Compare last_function_evaluation, which is entry 1
            j=1
            print('    Comparing last_function evaluation: '+splitline[j].strip()+' vs '+splitline_reference[j].strip()) 
            if splitline[j].strip() != splitline_reference[j].strip():
                print('WARNING!! For algorithm '+algorithm+', last_function_evaluation has changed!')
                print('Reference line:')
                print(splitline_reference)
                print('Corresponding line in new short_summary file:')
                print(splitline)
                #exit(1) 
            # Compare best_function_evaluation, which is entry 3
            j=3
            print('    Comparing best_function evaluation: '+splitline[j].strip()+' vs '+splitline_reference[j].strip()) 
            if splitline[j].strip() != splitline_reference[j].strip():
                print('WARNING!! For algorithm '+algorithm+', best_function_evaluation has changed!')
                print('Reference line:')
                print(splitline_reference)
                print('Corresponding line in new short_summary file:')
                print(splitline)
                #exit(1)

        # Now compare the optimum value of the objective function
        temp = splitline_reference[-1]
        try:
            abs_tolerance_f = float(temp)
        except:
            print('Error converting absolute_tolerance_f from char to float: '+temp)
            raise
        temp = splitline_reference[-3]
        try:
            oldval = float(temp)
        except:
            print('Error converting reference f from char to float: '+temp)
            raise
        temp = splitline[-1]
        try:
            newval = float(temp)
        except:
            print('Error converting reference f from char to float: '+temp)
            raise
        diff = np.abs(oldval-newval)
        if diff > abs_tolerance_f:
            print('Significant difference in objective function! Reference val={:.15e}, new val={:.15e}, diff={:.3e}, abs_tol={:.3e}'.format(oldval,newval,diff,abs_tolerance_f))
            exit(1)
        print('    f matches:    Reference val={:.15e}, new val={:.15e}, diff={:.3e}, abs_tol={:.3e}'.format(oldval,newval,diff,abs_tolerance_f))

        # Now compare the optimum values of the input parameters
        temp = splitline_reference[-2]
        try:
            abs_tolerance_x = float(temp)
        except:
            print('Error converting absolute_tolerance_x from char to float: '+temp)
            raise
        for j in range(N_parameters):
            temp = splitline_reference[5+j]
            try:
                oldval = float(temp)
            except:
                print('Error converting reference f from char to float: '+temp)
                raise
            temp = splitline[5+j]
            try:
                newval = float(temp)
            except:
                print('Error converting reference f from char to float: '+temp)
                raise
            diff = np.abs(oldval-newval)
            if diff > abs_tolerance_x:
                print('Significant difference in x('+str(j+1)+')! Reference val={:.15e}, new val={:.15e}, diff={:.3e}, abs_tol={:.3e}'.format(oldval,newval,diff,abs_tolerance_x))
                exit(1)
            print('    x('+str(j+1)+') matches: Reference val={:.15e}, new val={:.15e}, diff={:.3e}, abs_tol={:.3e}'.format(oldval,newval,diff,abs_tolerance_x))

    print('  Regression tests were successful for '+example_name+'.')

# Allow this module to be run as a standalone script.
if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print('Error! You must specify 1 argument: the name of the example to check.')
        exit(1)
    regression_test(sys.argv[1])
    
