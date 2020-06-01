# Output files and plotting {#plotting}

# Convergence history

When an optimization is carried out with MANGO, an output file is created
which records the history of the objective function evaluations.
Usually this file is named `mango_out.<something>`, but any name you like
can be set using mango::Problem::set_output_filename.
The first 5 lines of this file are header lines, with lines 1, 3, and 5 just a description of what follows.
Line 2 is either `standard` or `least_squares`, depending on whether the optimization problem
has least-squares form. Line 4 gives `N_parameters`, the dimension of the search space.
In the main data section that begins on line 6, the following data are recorded.
The objective function evaluation count, the time in seconds since the optimization began, the values of the independent variables, and
the value of the objective function. For least-squares problems, additional columns may be included that give the value of each residual term
in the objective function. These additional columns can be included or excluded using mango::Least_squares_problem::set_print_residuals_in_output_file.

If the optimization completes gracefully, a final row will be appended to the output file, which is a copy of the previous line corresponding
to the minimum objective function.

The data in the output file can be plotted using the python script `plotting/mangoPlot`. This script requires `numpy` and `matplotlib`. 
As command-line arguments to this script, you can supply the names of one or more MANGO output files. The wildcard character `*` can be used
to select multiple files, just as for commands like `ls`. 
The `mangoPlot` script will display two plots. In both figures, the vertical axis is the objective function minus the
minimum value of the objective function found among all the output files being plotted. In the top figure, the horizontal
axis is the number of evaluations of the objective function. In the second figure, the horizontal axis is the wallclock time in seconds.

The `mangoPlot` script is useful for comparing the performance of different optimization algorithms. 
For example, here is typical output of `mangoPlot`, comparing 3 PETSc algorithms for the `chwirut_c` example, 
each using 5 MPI processors and 5 worker groups:

\image html mangoPlot.png width=100%


# Levenberg-Marquardt history

