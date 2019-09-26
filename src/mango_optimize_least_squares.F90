subroutine mango_optimize_least_squares(problem, residual_function)

  use mango

  implicit none

  include 'mpif.h'

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function
  integer :: j, ierr, data(1)

  !-------------------------------------------

  problem%least_squares = .true.
  
  if (.not. problem%proc0_worker_groups) stop "The mango_optimize_least_squares() subroutine should only be called by group leaders, not by all workers."

  if (problem%proc0_world) then
     if (.not. allocated(problem%state_vector)) stop "State vector has not been allocated."
     if (.not. allocated(problem%targets)) stop "Targets vector has not been allocated."
     if (.not. allocated(problem%sigmas)) stop "Sigmas vector has not been allocated."

     problem%N_parameters = size(problem%state_vector)
     print *,"Detected N_parameters=",problem%N_parameters

     problem%N_terms = size(problem%targets)
     print *,"Detected N_terms=",problem%N_terms
     if (size(problem%sigmas) .ne. problem%N_terms) stop "size(sigmas) .ne. size(targets)"
  end if

  ! Make sure that parameters used by the finite-difference gradient routine are the same for all group leaders:
  call mpi_bcast(problem%N_parameters, 1, MPI_INTEGER, 0, problem%mpi_comm_group_leaders, ierr)
  call mpi_bcast(problem%N_terms, 1, MPI_INTEGER, 0, problem%mpi_comm_group_leaders, ierr)
  call mpi_bcast(problem%centered_differences, 1, MPI_LOGICAL, 0, problem%mpi_comm_group_leaders, ierr)
  call mpi_bcast(problem%finite_difference_step_size, 1, MPI_DOUBLE_PRECISION, 0, problem%mpi_comm_group_leaders, ierr)

  if (.not. problem%proc0_world) then
     call mango_group_leaders_least_squares_loop(problem, residual_function)
     return
  end if
  ! Only proc0_world continues past this point.

  print *,"Hello world from mango_optimize_least_squares."

  problem%function_evaluations = 0

  ! Write header line of output file
  open(unit = problem%output_unit, file = trim(problem%output_filename))
  write (problem%output_unit,"(a)") "Least squares?"
  write (problem%output_unit,"(a)") "yes"
  write (problem%output_unit,"(a)") "N_parameters, N_terms:"
  write (problem%output_unit,"(i0,a,i0)") problem%N_parameters, ", ", problem%N_terms
  write (problem%output_unit,"(a)",advance="no") "function_evaluation"
  do j = 1, problem%N_parameters
     write (problem%output_unit,"(a,i0,a)",advance="no") ",x(",j,")"
  end do
  write (problem%output_unit,"(a)",advance="no") ",objective_function"
  do j = 1, problem%N_terms
     write (problem%output_unit,"(a,i0,a)",advance="no") ",F(",j,")"
  end do
  write (problem%output_unit,"(a)") ""
  call flush(problem%output_unit)

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_pounders)
     call mango_optimize_least_squares_petsc(problem, residual_function)
  case (mango_algorithm_petsc_nm)
     call mango_optimize_petsc(problem, least_squares_to_single_objective, residual_function)
  case (mango_algorithm_hopspack)
     call mango_optimize_hopspack(problem, least_squares_to_single_objective)
  case (mango_algorithm_nlopt_gn_direct, &
       mango_algorithm_nlopt_gn_direct_l, &
       mango_algorithm_nlopt_gn_direct_l_rand, &
       mango_algorithm_nlopt_gn_direct_noscal, &
       mango_algorithm_nlopt_gn_direct_l_noscal, &
       mango_algorithm_nlopt_gn_direct_l_rand_noscal, &
       mango_algorithm_nlopt_gn_orig_direct, &
       mango_algorithm_nlopt_gn_orig_direct_l, &
       mango_algorithm_nlopt_gn_crs2_lm, &
       mango_algorithm_nlopt_ln_cobyla, &
       mango_algorithm_nlopt_ln_bobyqa, &
       mango_algorithm_nlopt_ln_praxis, &
       mango_algorithm_nlopt_ln_neldermead, &
       mango_algorithm_nlopt_ln_sbplx, &
       mango_algorithm_nlopt_ld_lbfgs)
     call mango_optimize_nlopt(problem, least_squares_to_single_objective, residual_function)
  case default
     print "(a,a)","Error! Unrecognized algorithm: ",trim(problem%algorithm)
     stop
  end select

  close(problem%output_unit)

  ! Tell the other group leaders to exit.
  data = -1
  call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_group_leaders,ierr)

contains

  ! ------------------------------------------------------

  subroutine least_squares_to_single_objective(non_least_squares_problem, x, f, failed)

    implicit none

    type(mango_problem) :: non_least_squares_problem
    double precision, intent(in) :: x(:)
    double precision, intent(out) :: f
    logical, intent(out) :: failed
    double precision, allocatable :: residual_vector(:)
    external mango_residual_function_wrapper

    print *,"Hello from least_squares_to_single_objective"
    allocate(residual_vector(problem%N_terms))
    call mango_residual_function_wrapper(problem, residual_function, x, residual_vector, failed)
    !call residual_function(problem, x, residual_vector, failed)
    f = sum(((residual_vector - problem%targets) / problem%sigmas) ** 2)
    print *,"residual_vector:",residual_vector,", f:",f
    deallocate(residual_vector)

  end subroutine least_squares_to_single_objective

end subroutine mango_optimize_least_squares

  !------------------------------------------------

subroutine mango_residual_function_wrapper(problem, residual_function, x, f, failed)

  use mango

  implicit none

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function
  double precision, intent(in) :: x(problem%N_parameters)
  double precision, intent(out) :: f(problem%N_terms)
  logical, intent(out) :: failed
  integer :: j

  problem%function_evaluations = problem%function_evaluations + 1
  call residual_function(problem, x, f, failed)
    
  write (problem%output_unit,"(i7)",advance="no") problem%function_evaluations
  do j = 1, problem%N_parameters
     write (problem%output_unit,"(a,es24.15)",advance="no") ",", x(j)
  end do
  write (problem%output_unit,"(a,es24.15)",advance="no") ",", sum(((f - problem%targets) / problem%sigmas) ** 2)
  do j = 1, problem%N_terms
     write (problem%output_unit,"(a,es24.15)",advance="no") ",", f(j)
  end do
  write (problem%output_unit,"(a)") ""
  call flush(problem%output_unit)
  
end subroutine mango_residual_function_wrapper
  

