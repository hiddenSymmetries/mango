subroutine mango_optimize(problem, objective_function)

  use mango

  implicit none

  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function
  integer :: j

  !-------------------------------------------

  print *,"Hello world from mango_optimize."

  if (.not. allocated(problem%state_vector)) stop "State vector has not been allocated."

  problem%function_evaluations = 0

  problem%N_parameters = size(problem%state_vector)
  print *,"Detected N_parameters=",problem%N_parameters

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
     call mango_optimize_petsc(problem, objective_function_wrapper)
  case (mango_algorithm_hopspack)
     call mango_optimize_hopspack(problem, objective_function_wrapper)
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
       mango_algorithm_nlopt_ln_sbplx  )
     call mango_optimize_nlopt(problem, objective_function_wrapper)
  case default
     print "(a,a)","Error! Unrecognized algorithm: ",trim(problem%algorithm)
     stop
  end select

  close(problem%output_unit)

contains

  !------------------------------------------------

  subroutine objective_function_wrapper(x, f, failed)

    implicit none

    double precision, intent(in) :: x(:)
    double precision, intent(out) :: f
    logical, intent(out) :: failed
    integer :: j

    problem%function_evaluations = problem%function_evaluations + 1
    call objective_function(x, f, failed)
    
    write (problem%output_unit,"(i7)",advance="no") problem%function_evaluations
    do j = 1, problem%N_parameters
       write (problem%output_unit,"(a,es24.15)",advance="no") ",", x(j)
    end do
    write (problem%output_unit,"(a,es24.15)") ",", f
    call flush(problem%output_unit)

  end subroutine objective_function_wrapper

end subroutine mango_optimize
