#!/usr/bin/env python

import numpy as np
import os

def test_float_equality(x1,x2,tol):
    # Return true if the _relative_ difference |x1 - x2|/mean(x1,x2) is < tol,
    # except when x1==x2==0, in which case return true.
    avg = (np.abs(x1) + np.abs(x2)) / 2
    if (x1 == 0) and (x2 == 0):
        return True
    return np.abs(x2-x1) / avg < tol

def compare_2_examples(filename1, filename2, algorithms_to_exclude):
    solution_vector_tolerance = 1.0e-17
    objective_function_tolerance = 1.0e-17
    verbose = False

    # Read output from example 1:
    big_filename1 = 'output/short_summary_mpi.'+filename1
    bigger_filename1 = big_filename1
    if not os.path.isfile(big_filename1):
        # Try another possible path to the file
        bigger_filename1 = '../'+big_filename1
    try:
        f = open(bigger_filename1)
    except:
        print('Error! Unable to open example output file '+big_filename1)
        raise
    lines1 = f.readlines()
    f.close()

    # Read output from example 2:
    big_filename2 = 'output/short_summary_mpi.'+filename2
    bigger_filename2 = big_filename2
    if not os.path.isfile(big_filename2):
        # Try another possible path to the file
        bigger_filename2 = '../'+big_filename2
    try:
        f = open(bigger_filename2)
    except:
        print('Error! Unable to open example output file '+big_filename2)
        raise
    lines2 = f.readlines()
    f.close()

    try:
        N_parameters = int(lines1[3])
    except:
        print('Error! Unable to convert line 4 of '+big_filename1+' to an integer.')
        raise

    if len(lines1) != len(lines2):
        print('Error! Number of lines in '+big_filename1+' does not equal number of lines in '+big_filename2)
        exit(1)

    # Ignore header lines:
    for j_line in range(5,len(lines1)):
        results1 = lines1[j_line].split(',')
        results2 = lines2[j_line].split(',')

        # Algorithm, N_procs, and N_worker_groups should always match:
        if (results1[0] != results2[0]) or (results1[1] != results2[1]) or (results1[2] != results2[2]):
            print('Error! Lines '+str(j_line)+' of '+big_filename1+' and '+big_filename2+' to not match all of the first 3 elements.')
            print(big_filename1+': ',results1)
            print(big_filename2+': ',results2)
            exit(1)

        # See if this is an algorithm to skip:
        algorithm1 = results1[0].strip()
        exclude = False
        for algorithm in algorithms_to_exclude:
            if algorithm == algorithm1:
                exclude = True
                continue
        if exclude:
            if verbose:
                print('Skipping excluded algorithm:',results1)
            continue
        else:
            if verbose:
                print('Processing line:',results1)

        # If both examples failed for these parameters, that is fine, and we can consider it a match.
        index = 3
        if results1[index].strip() == 'FAILED' and results2[index].strip() == 'FAILED':
            if verbose:
                print('2 failures count as a match')
            continue

        # Make sure the number of function evaluations was the same:
        index = 3
        function_evaluations1 = int(results1[index])
        function_evaluations2 = int(results2[index])
        if function_evaluations1 != function_evaluations2:
            print('Error! On line '+str(j_line)+', function_evaluations does not match between '+big_filename1+' and '+big_filename2)
            print('function_evaluations1 = ',results1[index],',  function_evaluations2 = ',results2[index])
            exit(1)

        # Make sure the solution vector was the same:
        for j_parameter in range(N_parameters):
            index = 4+j_parameter
            x1 = float(results1[index])
            x2 = float(results2[index])
            if not test_float_equality(x1,x2,solution_vector_tolerance):
                print('Error! On line '+str(j_line)+', x['+str(j_parameter)+'] does not match between '+big_filename1+' and '+big_filename2)
                print('x1 = ',results1[index].strip(),',  x2 = ',results2[index].strip())
                exit(1)

        # Make sure the final objective function was the same:
        index = 4+N_parameters
        f1 = float(results1[index])
        f2 = float(results2[index])
        if not test_float_equality(f1,f2,objective_function_tolerance):
            print('Error! On line '+str(j_line)+', the objective function does not match between '+big_filename1+' and '+big_filename2)
            print('f1 = ',results1[index].strip(),',  f2 = ',results2[index].strip())
            exit(1)


    print('All results compared in '+big_filename1+' and '+big_filename2+' are consistent.')
