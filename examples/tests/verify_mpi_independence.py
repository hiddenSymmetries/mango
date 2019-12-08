#!/usr/bin/env python3

# This module checks a single example's short_summary_mpi file to verify
# that results for a given algorithm are exactly independent of the number of
# MPI processes and N_worker_groups. Only algorithms that are deterministic
# are checked, i.e. algorithms that involve randomness are ignored.

try:
    # This case works when this script is run standalone
    from nondeterministic_algorithms import *
except:
    # This case works when this module is imported from another script
    from tests.nondeterministic_algorithms import *

def compare_entries(firstline, thisline, i, j, k):
    if firstline[k] != thisline[k]:
        print('Error! firstline (line '+str(i)+') and thisline (line '+str(j)+') differ in element '+str(k))
        print('firstline:',firstline)
        print('thisline: ',thisline)
        exit(1)

def verify_mpi_independence(example_name):
    print('Hello from '+__file__+'. Verifying results for each deterministic algorithm in '+example_name+' are independent of N_procs and N_worker_groups.')

    # First, open the short_summary_mpi.* file
    big_filename = 'output/short_summary_mpi.'+example_name
    bigger_filename = big_filename
    import os
    if not os.path.isfile(big_filename):
        # Try another possible path to the file
        bigger_filename = '../'+big_filename
    try:
        f = open(bigger_filename)
    except:
        print('Error! Unable to open example output file '+big_filename)
        raise
    lines = f.readlines()
    f.close()

    index = 5 # First line with regular content after the header lines.
    while index < len(lines)-1:
        firstline = lines[index].split(',')
        print('Found first line with a new algorithm:',lines[index].strip())
        index0 = index
        algorithm = firstline[0].strip()
        # Check whether this algorithm is deterministic
        random_algorithm = False
        for j in range(len(nondeterministic_algorithms)):
            if algorithm == nondeterministic_algorithms[j]:
                random_algorithm = True
                break
        # Loop over all the other lines in the summary file with the same algorithm:
        for j in range(index+1, len(lines)):
            index = j
            thisline = lines[j].split(',')
            if thisline[0] != firstline[0]:
                # j is the first line of a new algorithm, so exit the j loop.
                break
            # If we made it this far, then thisline and firstline use the same algorithm.
            if random_algorithm:
                print('Skipping line due to nondeterministic algorithm:',lines[j].strip())
                continue
            if len(firstline) != len(thisline):
                print('Error! firstline (line '+str(index0)+') and thisline (line '+str(j)+') have different number of elements')
                print('firstline:',firstline)
                print('thisline: ',thisline)
                exit(1)
            if firstline[3].strip() == 'FAILED' and thisline[3].strip() == 'FAILED':
                # Both runs failed. Treat this case as a match.
                print('This line and firstline both failed, which is a match:',lines[j].strip())
                continue
            compare_entries(firstline, thisline, index0, j, 3) # last_function_evaluation
            compare_entries(firstline, thisline, index0, j, 5) # best_function_evaluation
            for k in range(7, len(firstline)):
                compare_entries(firstline, thisline, index0, j, k)
            print('Confirmed this line matches firstline:',lines[j].strip())
    print('Finished confirming that all deterministic algorithms in '+big_filename+' give results independent of N_procs and N_worker_groups.')

# Allow this module to be run as a standalone script.
if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print('Error! You must specify 1 argument: the name of the example to check.')
        exit(1)
    verify_mpi_independence(sys.argv[1])
    
