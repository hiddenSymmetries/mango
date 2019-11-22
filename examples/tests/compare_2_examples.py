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

def compare_2_examples(filename1, filename2, algorithms):
    solution_vector_tolerance = 1.0e-17
    objective_function_tolerance = 1.0e-17

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

    # Ignore header lines:
    lines1 = lines1[5:]
    lines2 = lines2[5:]

    for j_algorithm in range(len(algorithms)):
        # Extract the lines from the output file associated with a given algorithm
        algorithm = algorithms[j_algorithm]
        results1 = []
        results2 = []
        for line in lines1:
            splitline = line.split(',')
            if splitline[0] == algorithm:
                results1.append(splitline)
        for line in lines2:
            splitline = line.split(',')
            if splitline[0] == algorithm:
                results2.append(splitline)
        if len(results1) == 0:
            print('Error! No results for algorithm '+algorithm+' in file '+big_filename1)
            exit(1)
        if len(results2) == 0:
            print('Error! No results for algorithm '+algorithm+' in file '+big_filename2)
            exit(1)
        if len(results1) != len(results2):
            print('Error! For algorithm '+algorithm+', the number of MPI options in '+big_filename1+' does not equal the corresponding number in '+big_filename2)
            exit(1)

        for j_mpi in range(len(results1)):
            # Make sure the same value of N_procs was used:
            index = 1
            N_procs1 = int(results1[j_mpi][index])
            N_procs2 = int(results2[j_mpi][index])
            if N_procs1 != N_procs2:
                print('Error! For algorithm '+algorithm+' line '+str(j_mpi)+', N_procs does not match between '+big_filename1+' and '+big_filename2)
                exit(1)

            # Make sure the same value of N_worker_groups was used:
            index = 2
            N_worker_groups1 = int(results1[j_mpi][index])
            N_worker_groups2 = int(results2[j_mpi][index])
            if N_worker_groups1 != N_worker_groups2:
                print('Error! For algorithm '+algorithm+' line '+str(j_mpi)+', N_worker_groups does not match between '+big_filename1+' and '+big_filename2)
                exit(1)

            # If both examples failed for these parameters, that is fine, and we can consider it a match.
            index = 3
            if results1[j_mpi][index].strip() == 'FAILED' and results2[j_mpi][index].strip() == 'FAILED':
                continue

            # Make sure the number of function evaluations was the same:
            index = 3
            function_evaluations1 = int(results1[j_mpi][index])
            function_evaluations2 = int(results2[j_mpi][index])
            if function_evaluations1 != function_evaluations2:
                print('Error! For algorithm '+algorithm+' line '+str(j_mpi)+', function_evaluations does not match between '+big_filename1+' and '+big_filename2)
                print('function_evaluations1 = ',results1[j_mpi][index],',  function_evaluations2 = ',results2[j_mpi][index])
                exit(1)

            # Make sure the solution vector was the same:
            for j_parameter in range(N_parameters):
                index = 4+j_parameter
                x1 = float(results1[j_mpi][index])
                x2 = float(results2[j_mpi][index])
                if not test_float_equality(x1,x2,solution_vector_tolerance):
                    print('Error! For algorithm '+algorithm+' line '+str(j_mpi)+', x['+str(j_parameter)+'] does not match between '+big_filename1+' and '+big_filename2)
                    print('x1 = ',results1[j_mpi][index].strip(),',  x2 = ',results2[j_mpi][index].strip())
                    exit(1)

            # Make sure the final objective function was the same:
            index = 4+N_parameters
            f1 = float(results1[j_mpi][index])
            f2 = float(results2[j_mpi][index])
            if not test_float_equality(f1,f2,objective_function_tolerance):
                print('Error! For algorithm '+algorithm+' line '+str(j_mpi)+', the objective function does not match between '+big_filename1+' and '+big_filename2)
                print('f1 = ',results1[j_mpi][index].strip(),',  f2 = ',results2[j_mpi][index].strip())
                exit(1)


    print('All results compared in '+big_filename1+' and '+big_filename2+' are consistent.')
