subroutine mango_optimize(problem, objective_function)

  ! This subroutine should be called by all the group leaders.

  use mango

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function
  integer :: j, ierr, data(1)
  external mango_dummy_residual_function

  !-------------------------------------------

  problem%least_squares = .false.
  
  if (.not. problem%proc0_worker_groups) stop "The mango_optimize() subroutine should only be called by group leaders, not by all workers."

  if (problem%proc0_world) then
     if (.not. allocated(problem%state_vector)) stop "State vector has not been allocated."

     problem%N_parameters = size(problem%state_vector)
     print *,"Detected N_parameters=",problem%N_parameters
  end if

  ! Make sure that parameters used by the finite-difference gradient routine are the same for all group leaders:
  call mpi_bcast(problem%N_parameters, 1, MPI_INTEGER, 0, problem%mpi_comm_group_leaders, ierr)
  call mpi_bcast(problem%centered_differences, 1, MPI_LOGICAL, 0, problem%mpi_comm_group_leaders, ierr)
  call mpi_bcast(problem%finite_difference_step_size, 1, MPI_DOUBLE_PRECISION, 0, problem%mpi_comm_group_leaders, ierr)

  if (.not. problem%proc0_world) then
     call mango_group_leaders_loop(problem, objective_function)
     return
  end if
  ! Only proc0_world continues past this point.

  print *,"Hello world from mango_optimize."

  problem%function_evaluations = 0

  ! Write header line of output file
  open(unit = problem%output_unit, file = trim(problem%output_filename))
  write (problem%output_unit,"(a)") "Least squares?"
  write (problem%output_unit,"(a)") "no"
  write (problem%output_unit,"(a)") "N_parameters:"
  write (problem%output_unit,"(i0)") problem%N_parameters
  write (problem%output_unit,"(a)",advance="no") "function_evaluation"
  do j = 1, problem%N_parameters
     write (problem%output_unit,"(a,i0,a)",advance="no") ",x(",j,")"
  end do
  write (problem%output_unit,"(a)") ",objective_function"
  call flush(problem%output_unit)

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_pounders)
     stop "Error! The petsc_pounders algorithm is for least-squares problems only."
  case (mango_algorithm_petsc_nm)
     !call mango_optimize_petsc(problem, objective_function_wrapper)
     call mango_optimize_petsc(problem, objective_function, mango_dummy_residual_function)
  case (mango_algorithm_hopspack)
     !call mango_optimize_hopspack(problem, objective_function_wrapper)
     call mango_optimize_hopspack(problem, objective_function)
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
       mango_algorithm_nlopt_ld_lbfgs  )
     !call mango_optimize_nlopt(problem, objective_function_wrapper)
     call mango_optimize_nlopt(problem, objective_function, mango_dummy_residual_function)
  case default
     print "(a,a)","Error! Unrecognized algorithm: ",trim(problem%algorithm)
     stop
  end select

  close(problem%output_unit)

  ! Tell the other group leaders to exit.
  data = -1
  call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_group_leaders,ierr)

end subroutine mango_optimize

!contains

  !------------------------------------------------

!subroutine objective_function_wrapper(x, f, failed)
  subroutine mango_objective_function_wrapper(problem, objective_function, x, f, failed)

    use mango

    implicit none

    type(mango_problem) :: problem
    procedure(mango_objective_function_interface) :: objective_function
    double precision, intent(in) :: x(problem%N_parameters)
    double precision, intent(out) :: f
    logical, intent(out) :: failed
    integer :: j

    print *,"mango_objective_function_wrapper: size(x)=",size(x)
    print *,"x=",x
    problem%function_evaluations = problem%function_evaluations + 1
    call objective_function(problem, x, f, failed)
    
    write (problem%output_unit,"(i7)",advance="no") problem%function_evaluations
    do j = 1, problem%N_parameters
       write (problem%output_unit,"(a,es24.15)",advance="no") ",", x(j)
    end do
    write (problem%output_unit,"(a,es24.15)") ",", f
    call flush(problem%output_unit)

  end subroutine mango_objective_function_wrapper
  !end subroutine objective_function_wrapper

!end subroutine mango_optimize
