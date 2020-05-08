# Calling MANGO from Fortran {#fortran}

The use of MANGO from a Fortran application is described here. For a demonstration of the concepts here, you are strongly encouraged to look
at the 
[examples in the git repository](https://github.com/hiddenSymmetries/mango/tree/master/examples/src).

# Connecting your code to MANGO

To use MANGO from your Fortran application, you must include use the `mango_mod` module from your source code:

~~~~{.f90}
use mango_mod
~~~~

MANGO also makes use of the standard Fortran module `iso_C_binding` in two ways. First, integers and doubles passed to and from MANGO should have type `C_int` and `C_double` respectively.
These types are defined in the `iso_C_binding` module. Normally these types are the
same as the Fortran types `integer` and `double precision`, but if the C and Fortran types differ
due to compiler flags, MANGO will use the C types, since it uses C++ internally.
Second, the `user_data` argument to 
your user-supplied objective function or residual function has the type `C_ptr` defined
in `iso_C_binding`.
If you pass data to your objective function using the @ref mango_set_user_data subroutine,
you need to use the `C_loc` function from `iso_C_binding` to generate a pointer
to your data of this type, and use the function `c_f_pointer` subroutine from
`iso_C_binding` to convert the C pointer to a Fortran pointer.
Therefore your objective/residual function and any routine that calls @ref mango_set_user_data must include
~~~~{.f90}
use iso_C_binding
~~~~

When compiling your code, make sure the compiler is able to find the `mango_mod` module file,
which typically has a filename like `mango_mod.mod` or `mango_mod.MOD`, depending on the compiler.
This file will be located in the `include` subdirectory
of MANGO. Typically the compiler can be instructed to look in this directory with a flag such as `-I <mangodir>/include` where `<mangodir>` is replaced
by the appropriate directory on your system.

To link your code to MANGO, link to the file `libmango.a`, which is located in MANGO's `lib` subdirectory.
Typically the linker can be instructed to link to this file with flags such as `-L <mangodir>/lib -lmango`, where `<mangodir>` is replaced
by the appropriate directory on your system.


# Defining an objective function

For general optimization problems, the objective function is determined by a user-supplied subroutine with the form
mango_mod::objective_function_interface:

~~~~{.f90}
subroutine objective_function(N_parameters, state_vector, objective_value, failed, problem, user_data)
~~~~

where `objective_function` can be replaced with any name you like.
The array `state_vector` is an input to your subroutine, indicating the values of the independent variables for which
the objective function is to be evaluated.
`N_parameters` is also provided as an input to the subroutine for convenience, as it gives the size of the `state_vector` array. 
After your subroutine has computed the objective function, the result (a numberof type `C_double`) should be stored in `objective_value`.
If your calculation fails for some reason, you can return a value of 1 to `failed`; otherwise you can return 0 to `failed` or simply not set `failed`,
since it defaults to 0.

The mango_mod::mango_problem object representing the optimization problem is supplied to your subroutine as `problem`. This is often useful
so your subroutine can use information related to MPI, e.g. communicating between group leaders and workers.

The argument `user_data` is a pointer to any data structure you supply via the subroutine @ref mango_set_user_data. This argument
is useful for passing information to your subroutine for the objective function,
as discussed further below
Note that `user_data` has `type(C_ptr)`, defined in the standard `iso_C_binding` module.




# Defining the residuals for a least-squares problem

If your problem has least-squares structure, then instead of supplying the objective function as described in the previous section,
you must write a subroutine that computes the vector of residuals \f$R_j\f$ described in @ref concepts.
This subroutine must have the form mango_mod::vector_function_interface:

~~~~{.f90}
subroutine residual_function(N_parameters, state_vector, N_terms, residuals, failed, problem, user_data)
~~~~

where `residual_function` can be replaced with any name you like.
The array `state_vector` is an input to your subroutine, indicating the values of the independent variables for which
the residuals are to be evaluated.
`N_parameters` is also provided as an input to the subroutine for convenience, as it gives the size of the `state_vector` array. 
Note that your subroutine should compute \f$R\f$, not \f$(R - T)/\sigma\f$ (using the notation from @ref concepts);
the \f$T\f$ and \f$\sigma\f$ arrays are set
separately when you create the mango_mod::mango_problem object.
After your subroutine has computed the vector \f$R\f$, the results should be stored in the `residuals` array.
The size of this array, `N_terms`, is provided as an input to the subroutine for convenience.
If your calculation fails for some reason, you can return a value of 1 to `failed`; otherwise you can return 0 to `failed` or simply not set `failed`,
since it defaults to 0.

The mango_mod::mango_problem object representing the optimization problem
is supplied to your subroutine as `problem`. This is often useful
so your subroutine can use information related to MPI, e.g. communicating between group leaders and workers.

The argument `user_data` is a pointer to any data structure you supply via the method @ref mango_set_user_data(). This argument
is useful for passing information to your subroutine for the objective function,
as discussed further below. 
Note that `user_data` has `type(C_ptr)`, defined in the standard `iso_C_binding` module.




# Creating an optimization problem object

MANGO optimization problems (both general and least-squares) are represented in Fortran by the type `mango_problem` defined in the `mango_mod` module:

~~~~{.f90}
type(mango_problem) :: myprob
~~~~

After such an object is declared, a general optimization problem can be created by calling @ref mango_problem_create :

~~~~{.f90}
call mango_problem_create(myprob, N_parameters, state_vector, objective_function)
~~~~

where `myprob` can be changed to whatever name you wish. 
You should set the integer `N_parameters` to the number of independent variables.
The values of these variables for the initial state should be stored in the user-allocated array `state_vector` (which has type `C_double`).
(As a general rule, MANGO gives the user responsibility for allocating memory.)
The values of the `state_vector` array will not be copied or used until you actually invoke @ref mango_optimize,
so you are free to change the entries in `state_vector` after calling @ref mango_problem_create.
After the optimization is completed, the same `state_vector` array will hold the location of the optimum found.

A pointer to the user-supplied subroutine for computing the objective function is given by the
`objective_function` argument.

For problems with least-squares structure, then instead of calling @ref mango_problem_create, you should call
@ref mango_problem_create_least_squares :

~~~~{.f90}
call mango_problem_create_least_squares(myprob, N_parameters, state_vector, N_terms, targets, sigmas, best_residual_function, residual_function)
~~~~

Again, `myprob` can be replaced with whatever name you like.
The arguments `N_parameters` and `state_vector` have exactly the same meaning as for a general optimization problem.
The integer `N_terms` provides the number of least-squares terms that are summed in the objective function, i.e. \f$N_t\f$ in @ref concepts.
The double precision arrays `targets` and `sigmas`, both of size `N_terms`, correspond to the quantities \f$T_j\f$ and \f$\sigma_j\f$ in @ref concepts,
shifting and scaling the terms in the objective function.
The double precision array `best_residual_function` will store the values of the residuals \f$R_j\f$ at the optimum after the optimization is completed.
Note that you have responsibility for allocating `targets`, `sigmas`, and `best_residual_function`.
The argument `residual_function` is a pointer to the user-supplied subroutine for computing the residuals \f$R_j\f$,
and `residual_function` can be replaced by whichever subroutine name you use in your code.



# Setting parameters

A number of subroutines are provided 
to set options and parameters. To set the optimization algorithm, see @ref algorithms.

In general, the Fortran API is similar to the C++ API, except that Fortran subroutines
take the optimization problem object as an extra first argument, and Fortran
subroutine names begin with `mango_`.

The subroutine @ref mango_set_max_function_evaluations sets the maximum number of times the objective function will be evaluated, e.g.

~~~~{.f90}
call mango_set_max_function_evaluations(myprob, 1000)
~~~~

where `myprob` is the optimization problem object (of type mango_mod::mango_problem).
The limit you set may be exceeded slightly because concurrent function evaluations will 
always be allowed to complete. For instance, if this limit is set to 1000,
a finite-difference gradient calculation involving 10 function evaluations may be initiated
when the objective function has been evaluated 995 times, resulting in 1005 evaluations in total.

To control whether centered or one-sided finite differences are used for gradient evaluations, use @ref mango_set_centered_differences, e.g.

~~~~{.f90}
call mango_set_centered_differences(myprob, .false.)
~~~~

The step size in the independent variables
used for finite difference derivatives is set using @ref mango_set_finite_difference_step_size :

~~~~{.f90}
call mango_set_finite_difference_step_size(myprob, 1.0e-6)
~~~~


MANGO writes an ASCII file containing the history of evaluations of the objective function, and the name of this file can be set using @ref mango_set_output_filename :

~~~~{.f90}
call mango_set_output_filename(myprob, "mango_out.rosenbrock")
~~~~

where the string can be replaced by any filename you like.

For least-squares problems, this output file can be set to include or not include the values of the individual residual terms.
It may be illuminating to view
this information, though for problems with a large number of residual terms, it may be easier to read the output file if this large volume of information is suppressed.
This choice can be controlled using
 @ref mango_set_print_residuals_in_output_file :

~~~~{.f90}
call mango_set_print_residuals_in_output_file(myprob, .false.)
~~~~

If you wish, a separate output file can be generated containing the information about the MPI partition, e.g. which processors are in which worker group.
This file can be written using @ref mango_mpi_partition_write, e.g.

~~~~{.f90}
call mango_mpi_partition_write(myprob, "mango_mpi.rosenbrock")
~~~~

where the string can be replaced by any filename you like.

If you wish to impose bound constraints (also known as box constraints) on your problem, you can call @ref mango_set_bound_constraints, e.g.

~~~~{.f90}
call mango_set_bound_constraints(myprob, lower_bounds, upper_bounds)
~~~~

Here, `lower_bounds` and `upper_bounds` are arrays of type `double precision` and are user-allocated arrays of size `N_parameters`. Note that not all algorithms
support bound constraints. The elements of these arrays are not copied or used until @ref mango_optimize is called,
so you can change the lower and upper bounds after the call to @ref mango_set_bound_constraints.

Alternatively, the related subroutine @ref mango_set_relative_bound_constraints 
lets you automatically set bound constraints based on a specified multiple of and/or distance
from the initial condition. For instance, to let each parameter vary between 0.5 and 2 times its initial value, you can call

~~~~{.f90}
call mango_set_relative_bound_constraints(myprob, 0.5, 2.0, 0.0, .true.)
~~~~

MANGO's Levenberg-Marquardt algorithm uses a parallelized line search in which, by default, the number of values of the \f$\lambda\f$ parameter
considered between Jacobian evaluations is equal to the number of worker groups. This choice generally makes the best use of parallelization,
but it makes the optimization history depend on the number of worker
groups, and hence possibly on the number of MPI processes. You may occasionally wish for the number of values of  \f$\lambda\f$ examined to be some specified value different from the number of worker groups,
particularly for tests related to parallelization. In this case, you can specify the number of \f$\lambda\f$ values to use with @ref mango_set_N_line_search, e.g.

~~~~{.f90}
call mango_set_N_line_search(myprob, 3)
~~~~

By default, MANGO will not print information to stdout. To turn on the printing of information for debugging you can use @ref mango_set_verbose, e.g.

~~~~{.f90}
call mango_set_verbose(myprob, 1)
~~~~

You can pass any variable or data structure to your subroutine for the objective function or residuals using @ref mango_set_user_data, e.g.

~~~~{.f90}
integer :: mydata = 42
...
call mango_set_user_data(myprob, C_loc(mydata))
~~~~
The `C_loc` function used here is in the standard Fortran module `iso_C_binding`.


The pointer that you provide will be passed to your subroutine for the objective function or residuals as the `user_data` argument, which has `type(C_ptr)`.
(See mango_mod::objective_function_interface and mango_mod::vector_function_interface)
To use the data in your subroutine, you must convert the pointer from a C to Fortran pointer using the `c_f_pointer` subroutine in `iso_C_binding`:
~~~~{.f90}
subroutine objective_function(N_parameters, state_vector, objective_value, failed, myprob, user_data)
  use iso_C_binding
  ...
  integer, pointer :: f_user_data
  ...
  call c_f_pointer(user_data, f_user_data)
  ! Now f_user_data == 42.
~~~~




# MPI considerations

Your code (not MANGO) is responsible for calling `MPI_Init()` and `MPI_Finalize()`.

The set of MPI processes must be divided up into worker groups. This can either be done by MANGO
or you are welcome to do it yourself.
It is strongly recommended to have MANGO handle the division into worker groups. 
In this case, 
you can set the number of worker groups using @ref mango_set_N_worker_groups.
Then, after calling `MPI_Init`, call @ref mango_mpi_init as follows:

~~~~{.f90}
call mango_set_N_worker_groups(myprob, 5)
call mango_mpi_init(myprob, MPI_COMM_WORLD)
~~~~

where `myprob` is the name of the mango_mod::mango_problem object you have created.
If you want MANGO to only use a subset of the processes in `MPI_COMM_WORLD`, then use the appropriate
MPI communicator in the code line above.
If you have selected an optimization algorithm that does not support concurrent function
evaluations, MANGO will automatically change the number of worker groups to 1.

If you prefer to do your own division of
processes into worker groups, you can call @ref mango_mpi_partition_set_custom instead of @ref mango_mpi_init.

Once MANGO's MPI information has been initialized by one of these two methods,
methods are available to get the three MPI communicators discussed in @ref concepts,
@ref mango_get_mpi_comm_world,
@ref mango_get_mpi_comm_worker_groups, and
@ref mango_get_mpi_comm_group_leaders.
The number of processes in each communicator can be queried with
@ref mango_get_N_procs_world,
@ref mango_get_N_procs_worker_groups, and
@ref mango_get_N_procs_group_leaders.
The rank of a given process within each of the communicators can be obtained with
@ref mango_get_mpi_rank_world,
@ref mango_get_mpi_rank_worker_groups, and
@ref mango_get_mpi_rank_group_leaders.
To determine if a given process has rank = 0 within a given communicator, you can call
@ref mango_get_proc0_world and
@ref mango_get_proc0_worker_groups.
The number of worker groups and the worker group that a given process belongs to can
be found using
@ref mango_get_N_worker_groups and
@ref mango_get_worker_group.


MANGO provides several subroutines that may be convenient for handling
communication between group leaders and workers:
@ref mango_mobilize_workers, @ref mango_stop_workers, and
@ref mango_continue_worker_loop. Use of these subroutines is
optional. These subroutines are demonstrated in all of the
[examples](https://github.com/hiddenSymmetries/mango/tree/master/examples/src).
In typical usage, workers use @ref mango_continue_worker_loop
to wait until they are needed to evaluate the objective function,
and exit when the optimization is complete:

~~~~{.f90}
! This code is run only by workers, not group leaders.
do while (mango_continue_worker_loop(myprob))
  ! Here, communicate with my group leader via MPI and help it to evaluate the objective function.
end do
! We exit the above loop when mango_optimize() completes.
~~~~

Meanwhile, a group leader causes its workers to 
evaluate the code inside the above `do while` loop (i.e. causes
@ref mango_continue_worker_loop
to evaluate to `.true.`) by calling @ref mango_mobilize_workers inside the user-supplied subroutine for the objective or residual function, e.g.

~~~~{.f90}
subroutine objective_function(N_parameters, state_vector, objective_value, failed, myprob, user_data)


  call mango_mobilize_workers(myprob)
  ...
}
~~~~

When the group leaders are done with the optimization, they should instruct the
workers to exit the `do while` loop above:

~~~~{.f90}
call mango_stop_workers(myprob)
~~~~


# Running the optimization

Once parameters have been set, the optimization is actually performed with the function @ref mango_optimize. The lowest value of the objective function
is returned, e.g.

~~~~{.f90}
best_objective_function = mango_optimize(myprob)
~~~~

The subroutine @ref mango_optimize should be called by all group leaders, but not by workers. 

Once @ref mango_optimize has completed, the values of the independent variables
at the optimum are available in the `state_vector` array you provided to the mango::Problem constructor.
Further details of the optimization can be obtained using
 @ref mango_get_function_evaluations and @ref mango_get_best_function_evaluation.