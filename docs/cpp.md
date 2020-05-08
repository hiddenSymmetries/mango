# Calling MANGO from C++ {#cpp}

The use of MANGO from a C++ application is described here. For a demonstration of the concepts here, you are strongly encouraged to look
at the 
[examples in the git repository](https://github.com/hiddenSymmetries/mango/tree/master/examples/src).

# Connecting your code to MANGO

To use MANGO from your C++ application, you must include MANGO's header file from your source code:

~~~~{.cpp}
#include "mango.hpp"
~~~~

When compiling your code, make sure the compiler is able to find `mango.hpp`. This file is located in the `include` subdirectory
of MANGO. Typically the compiler can be instructed to look in this directory with a flag such as `-I <mangodir>/include` where `<mangodir>` is replaced
by the appropriate directory on your system.

To link your code to MANGO, link to the file `libmango.a`, which is located in MANGO's `lib` subdirectory.
Typically the linker can be instructed to link to this file with flags such as `-L <mangodir>/lib -lmango`, where `<mangodir>` is replaced
by the appropriate directory on your system.


# Defining an objective function

For general optimization problems, the objective function is determined by a user-supplied subroutine with the form
mango::objective_function_type:

~~~~{.cpp}
void objective_function(int* N_parameters, const double* state_vector, double* objective_value, int* failed, mango::Problem* problem, void* void_user_data)
~~~~

where `objective_function` can be replaced with any name you like.
The array `state_vector` is an input to your subroutine, indicating the values of the independent variables for which
the objective function is to be evaluated.
`N_parameters` is also provided as an input to the subroutine for convenience, as it gives the size of the `state_vector` array. 
After your subroutine has computed the objective function, the result (a double) should be stored at the location pointed to by `objective_value`.
If your calculation fails for some reason, you can return a value of 1 to `failed`; otherwise you can return 0 to `failed` or simply not set `failed`,
since it defaults to 0.

The mango::Problem object representing the optimization problem is supplied to your subroutine as `problem`. This is often useful
so your subroutine can use information related to MPI, e.g. communicating between group leaders and workers.

The argument `void_user_data` is a pointer to any data structure you supply via the method mango::Problem::set_user_data(). This argument
is useful for passing information to your subroutine for the objective function,
as discussed further below




# Defining the residuals for a least-squares problem

If your problem has least-squares structure, then instead of supplying the objective function as described in the previous section,
you must write a subroutine that computes the vector of residuals \f$R_j\f$ described in @ref concepts.
This subroutine must have the form mango::vector_function_type:

~~~~{.cpp}
void residual_function(int* N_parameters, const double* state_vector, int* N_terms, double* residuals, int* failed, mango::Problem* problem, void* void_user_data)
~~~~

where `residual_function` can be replaced with any name you like.
The array `state_vector` is an input to your subroutine, indicating the values of the independent variables for which
the residuals are to be evaluated.
`N_parameters` is also provided as an input to the subroutine for convenience, as it gives the size of the `state_vector` array. 
Note that your subroutine should compute \f$R\f$, not \f$(R - T)/\sigma\f$ (using the notation from @ref concepts);
the \f$T\f$ and \f$\sigma\f$ arrays are set
separately when you create the mango::Least_squares_problem object.
After your subroutine has computed the vector \f$R\f$, the results should be stored at the array pointed to by `residuals`.
The size of this array, `N_terms`, is provided as an input to the subroutine for convenience.
If your calculation fails for some reason, you can return a value of 1 to `failed`; otherwise you can return 0 to `failed` or simply not set `failed`,
since it defaults to 0.

The mango::Problem object representing the optimization problem (the parent of the mango::Least_squares_problem you create)
is supplied to your subroutine as `problem`. This is often useful
so your subroutine can use information related to MPI, e.g. communicating between group leaders and workers.

The argument `void_user_data` is a pointer to any data structure you supply via the method mango::Problem::set_user_data(). This argument
is useful for passing information to your subroutine for the objective function,
as discussed further below.




# Creating an optimization problem object

An object representing a general optimization problem can be created in your code using the mango::Problem constructor:

~~~~{.cpp}
mango::Problem myprob(N_parameters, state_vector, &objective_function, argc, argv);
~~~~

where `myprob` can be changed to whatever name you wish. 
You should set the integer `N_parameters` to the number of independent variables.
The values of these variables for the initial state should be stored in the user-allocated array `state_vector` (which has type `double*`).
(As a general rule, MANGO gives the user responsibility for allocating memory.)
The values of the `state_vector` array will not be copied or used until you actually invoke mango::Problem::optimize,
so you are free to change the entries in `state_vector` after calling this constructor.
After the optimization is completed, the same `state_vector` array will hold the location of the optimum found.

A pointer to the user-supplied subroutine for computing the objective function is given by the
`&objective_function` argument.

The last two arguments, `argc` (of type `int`) and `argv` (of type `char**`) allow arguments to be passed to
the optimization libraries that MANGO calls. At present, only PETSc uses these arguments. If you do not wish to supply arguments,
you are free to set `argc=0` and `argv=NULL`.

For problems with least-squares structure, then instead of creating a mango::Problem object, you should create
a mango::Least_squares_problem object:

~~~~{.cpp}
mango::Least_squares_problem myprob(N_parameters, state_vector, N_terms, targets, sigmas, best_residual_function, &residual_function, argc, argv);
~~~~

Again, `myprob` can be replaced with whatever name you like.
The arguments `N_parameters`, `state_vector`, `argc`, and `argv` have exactly the same meaning as for a general optimization problem.
The integer `N_terms` provides the number of least-squares terms that are summed in the objective function, i.e. \f$N_t\f$ in @ref concepts.
The double arrays `targets` and `sigmas`, both of size `N_terms`, correspond to the quantities \f$T_j\f$ and \f$\sigma_j\f$ in @ref concepts,
shifting and scaling the terms in the objective function.
The double array `best_residual_function` will store the values of the residuals \f$R_j\f$ at the optimum after the optimization is completed.
Note that you have responsibility for allocating `targets`, `sigmas`, and `best_residual_function`.
The argument `&residual_function` is a pointer to the user-supplied subroutine for computing the residuals \f$R_j\f$,
and `residual_function` can be replaced by whichever subroutine name you use in your code.



# Setting parameters

A number of methods are provided for mango::Problem and mango::Least_squares_problem objects
to set options and parameters. To set the optimization algorithm, see @ref algorithms.

The method mango::Problem::set_max_function_evaluations sets the maximum number of times the objective function will be evaluated, e.g.

~~~~{.cpp}
myprob.set_max_function_evaluations(1000);
~~~~

This limit may be exceeded slightly because concurrent function evaluations will 
always be allowed to complete. For instance, if this limit is set to 1000,
a finite-difference gradient calculation involving 10 function evaluations may be initiated
when the objective function has been evaluated 995 times, resulting in 1005 evaluations in total.

To control whether centered or one-sided finite differences are used for gradient evaluations, use mango::Problem::set_centered_differences, e.g.

~~~~{.cpp}
myprob.set_centered_differences(false);
~~~~

The step size in the independent variables
used for finite difference derivatives is set using mango::Problem::set_finite_difference_step_size:

~~~~{.cpp}
myprob.set_finite_difference_step_size(1.0e-6);
~~~~


MANGO writes an ASCII file containing the history of evaluations of the objective function, and the name of this file can be set using mango::Problem::set_output_filename:

~~~~{.cpp}
myprob.set_output_filename("mango_out.rosenbrock");
~~~~

where the string can be replaced by any filename you like.

For least-squares problems, this output file can be set to include or not include the values of the individual residual terms.
It may be illuminating to view
this information, though for problems with a large number of residual terms, it may be easier to read the output file if this large volume of information is suppressed.
This choice can be controlled using
 mango::Least_squares_problem::set_print_residuals_in_output_file:

~~~~{.cpp}
myprob.set_print_residuals_in_output_file(false);
~~~~

If you wish, a separate output file can be generated containing the information about the MPI partition, e.g. which processors are in which worker group.
This file can be written using mango::MPI_Partition::write, e.g.

~~~~{.cpp}
myprob.mpi_partition.write("mango_mpi.rosenbrock");
~~~~

where the string can be replaced by any filename you like.

If you wish to impose bound constraints (also known as box constraints) on your problem, you can call mango::Problem::set_bound_constraints, e.g.

~~~~{.cpp}
myprob.set_bound_constraints(lower_bounds, upper_bounds);
~~~~

Here, `lower_bounds` and `upper_bounds` are of type `double*` and are user-allocated arrays of size `N_parameters`. Note that not all algorithms
support bound constraints. The elements of these arrays are not copied or used until mango::Problem::optimize is called,
so you can change the lower and upper bounds after the call to mango::Problem::set_bound_constraints.

Alternatively, the related method mango::Problem::set_relative_bound_constraints 
lets you automatically set bound constraints based on a specified multiple of and/or distance
from the initial condition. For instance, to let each parameter vary between 0.5 and 2 times its initial value, you can call

~~~~{.cpp}
myprob.set_relative_bound_constraints(0.5, 2.0, 0.0, true);
~~~~

MANGO's Levenberg-Marquardt algorithm uses a parallelized line search in which, by default, the number of values of the \f$\lambda\f$ parameter
considered between Jacobian evaluations is equal to the number of worker groups. This choice generally makes the best use of parallelization,
but it makes the optimization history depend on the number of worker
groups, and hence possibly on the number of MPI processes. You may occasionally wish for the number of values of  \f$\lambda\f$ examined to be some specified value different from the number of worker groups,
particularly for tests related to parallelization. In this case, you can specify the number of \f$\lambda\f$ values to use with mango::Problem::set_N_line_search, e.g.

~~~~{.cpp}
myprob.set_N_line_search(3);
~~~~

By default, MANGO will not print information to stdout. To turn on the printing of information for debugging you can use mango::Problem::set_verbose, e.g.

~~~~{.cpp}
myprob.set_verbose(1);
~~~~

You can pass any variable or data structure to your subroutine for the objective function or residuals using mango::Problem::set_user_data, e.g.

~~~~{.cpp}
int mydata = 42;
myprob.set_user_data(&mydata);
~~~~

The pointer that you provide will be passed to your subroutine for the objective function or residuals as the `user_data` argument, which has type `void*`.
(See mango::objective_function_type and mango::vector_function_type.)
In this subroutine you must cast the pointer from `void*` to the appropriate type for your data.




# MPI considerations

Your code (not MANGO) is responsible for calling `MPI_Init()` and `MPI_Finalize()`.

The set of MPI processes must be divided up into worker groups. This can either be done by MANGO
or you are welcome to do it yourself.
It is strongly recommended to have MANGO handle the division into worker groups. 
In this case, you can set the number of worker groups using mango::MPI_Partition::set_N_worker_groups.
Then, after calling `MPI_Init`, call mango::Problem::mpi_init as follows:

~~~~{.cpp}
myprob.mpi_partition.set_N_worker_groups(5);
myprob.mpi_init(MPI_COMM_WORLD);
~~~~

where `myprob` is the name of the mango::Problem object you have created.
If you want MANGO to only use a subset of the processes in `MPI_COMM_WORLD`, then use the appropriate
MPI communicator in the code line above. 
If you have selected an optimization algorithm that does not support concurrent function
evaluations, MANGO will automatically change the number of worker groups to 1.

If you prefer to do your own division of
processes into worker groups, you can call mango::MPI_Partition::set_custom instead of mango::Problem::mpi_init.

Once MANGO's MPI information has been initialized by one of these two methods,
methods are available to get the three MPI communicators discussed in @ref concepts,
mango::MPI_Partition::get_comm_world, 
mango::MPI_Partition::get_comm_worker_groups, and
mango::MPI_Partition::get_comm_group_leaders.
The number of processes in each communicator can be queried with
mango::MPI_Partition::get_N_procs_world, 
mango::MPI_Partition::get_N_procs_worker_groups, and
mango::MPI_Partition::get_N_procs_group_leaders.
The rank of a given process within each of the communicators can be obtained with
mango::MPI_Partition::get_rank_world, 
mango::MPI_Partition::get_rank_worker_groups, and
mango::MPI_Partition::get_rank_group_leaders.
To determine if a given process has rank = 0 within a given communicator, you can call
mango::MPI_Partition::get_proc0_world and
mango::MPI_Partition::get_proc0_worker_groups.
The number of worker groups and the worker group that a given process belongs to can
be found using
mango::MPI_Partition::get_N_worker_groups and
mango::MPI_Partition::get_worker_group.



MANGO provides several methods that may be convenient for handling
communication between group leaders and workers:
mango::MPI_Partition::mobilize_workers, mango::MPI_Partition::stop_workers, and
mango::MPI_Partition::continue_worker_loop. Use of these methods is
optional. These methods are demonstrated in all of the
[examples](https://github.com/hiddenSymmetries/mango/tree/master/examples/src).
In typical usage, workers use mango::MPI_Partition::continue_worker_loop
to wait until they are needed to evaluate the objective function,
and exit when the optimization is complete:

~~~~{.cpp}
// This code is run only by workers, not group leaders.
while (myprob->mpi_partition.continue_worker_loop()) {
  // Here, communicate with my group leader via MPI and help it to evaluate the objective function.
}
// We exit the above loop when mango::Problem::optimize() completes.
~~~~

Meanwhile, a group leader causes its workers to 
evaluate the code inside the above `while` loop (i.e. causes
mango::MPI_Partition::continue_worker_loop
to evaluate to `true`) by calling mango::MPI_Partition::mobilize_workers inside the user-supplied subroutine for the objective or residual function, e.g.

~~~~{.cpp}
void objective_function(int* N_parameters, const double* state_vector, double* objective_value, int* failed, mango::Problem* problem, void* void_user_data) {


  problem->mpi_partition.mobilize_workers();
  ...
}
~~~~

When the group leaders are done with the optimization, they should instruct the
workers to exit the `while` loop above:

~~~~{.cpp}
myprob.mpi_partition.stop_workers();
~~~~


# Running the optimization

Once parameters have been set, the optimization is actually performed with the method mango::Problem::optimize. The lowest value of the objective function
is returned, e.g.

~~~~{.cpp}
best_objective_function = myprob.optimize();
~~~~

The method mango::Problem::optimize should be called by all group leaders, but not by workers. 

Once mango::Problem::optimize has completed, the values of the independent variables
at the optimum are available in the `state_vector` array you provided to the mango::Problem constructor.
Further details of the optimization can be obtained using
 mango::Problem::get_function_evaluations and mango::Problem::get_best_function_evaluation.