# Core Concepts {#concepts}

## Worker Groups and MPI Partitions

Given a set of \f$M\f$ MPI processes, an optimization problem can be parallelized in different ways.
At one extreme, all \f$M\f$ processes could work together on each evaluation of the objective function,
with the optimization algorithm itself only initiating a single evaluation of the objective function \f$f\f$ at a time.
At the other extreme, the optimization algorithm could request \f$M\f$ evaluations of \f$f\f$
all at once, with each evaluation performed by a single processor. 
This type of parallelization can be useful for instance when finite differences are used to evaluate gradients.
Or, both types of parallelization
could be used at the same time. To allow all of these types of parallelization, MANGO uses the concepts
of *worker groups* and *MPI partitions*.

A *worker group* is a set of MPI processes that works together to evaluate the objective function.
An MPI partition is a division of the total set of \f$M\f$ MPI processors into one or more worker groups.
If there are \f$W\f$ worker groups, each group has approximately \f$M / W \f$ processors (approximate because one must
round up or down.)
MANGO has a class mango::MPI_Partition to manage this division of processors into worker groups.
Each MANGO optimization problem, represented by an instance of the mango::Problem class, has an instance of mango::MPI_Partition as a member.
The number of worker groups can be set by calling mango::MPI_Partition::set_N_worker_groups()
or by reading a value in from a file via mango::Problem::read_input_file().

Some optimization algorithms, such as Nelder-Mead, do not support concurrent evaluations of \f$ f \f$.
In this case, the number of worker groups should be \f$ W=1 \f$.
MANGO will automatically set \f$ W = 1\f$ for any such algorithm.
Any algorithm that uses derivatives, such as BFGS, can take advantage of multiple worker groups
to evaluate derivatives by finite differences. If the number of parameters (i.e. independent variables) is \f$ N \f$,
you ideally want to set \f$ W = N+1 \f$ when using 1-sided finite differences,
and set \f$ W = 2 N + 1 \f$ when using centered differences.
These ideal values are not required however - MANGO will evaluate finite difference derivatives for any value of \f$ W \f$,
and results should be exactly independent of \f$ W \f$.
Other derivative-free algorithms that intrinsically support parallelization,
such as HOPSPACK, can use any number of worker groups, not tied to the number of parameters.

An MPI partition is associated with three MPI communicators, "world", "worker_groups", and "group_leaders". The 
"world" communicator represents all \f$ M \f$ MPI processors available to MANGO. (Normally this communicator is the
same as `MPI_COMM_WORLD`, but it could be a subset thereof if you wish.)
The "worker_groups" communicator also contains the same \f$ M \f$ processors, but grouped
into colors, with a different color representing each worker group. Therefore
subroutines such as `MPI_Send` and `MPI_Bcast` on this communicator exchange data only within one worker group.
This "worker_groups" communicator is therefore the one that must be used for evaluation of the objective function.
Finally, the "group_leaders" communicator only includes the \f$ W \f$ processors that have rank 0 in the "worker_groups" communicator.
This communicator is used for communicating data within a parallel optimization *algorithm* (as opposed to within a parallelized objective function).
Each of the three MPI communicators has "get" methods in mango::MPI_Partition to determine the number of processors,
each processor's rank, and whether a given processor has rank 0.



## General vs least-squares problems

A general optimization problem has the form
\f[
\min_{\vec{x}} f(\vec{x}),
\f]
possibly with constraints. The vector \f$ \vec{x} \f$ has a dimension that is called `N_parameters` in MANGO.

A "least-squares problem" is one in which the objective function has the form
\f[
f(\vec{x}) = \sum_{j=1}^{N_t} \left( \frac{R_j(\vec{x}) - T_j}{\sigma_j}\right)^2.
\f]
MANGO supports both general and least-squares problems. General problems are represented
by the class mango::Problem, while least-squares problems are represeted by the subclass mango::Least_squares_problem.
In the latter, \f$N_t \f$ is called `N_terms`, and \f$ T_j \f$ and \f$ \sigma_j \f$ are
supplied as the arrays `targets` and `sigmas` to the constructor.
In a least-squares problem, you supply a subroutine to compute the "residuals" \f$ R_j \f$
rather than \f$ f \f$ itself.

Any optimization algorithm for general problems can be applied to least-squares problems.
However some algorithms (such as Levenberg-Marquardt) are specific to least-squares problems, and cannot be applied to general problems.
