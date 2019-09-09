subroutine mango_optimize(problem, objective_function)

  use mango

  implicit none

  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function

  !-------------------------------------------

  print *,"Hello world from mango_optimize."

  if (.not. allocated(problem%state_vector)) stop "State vector has not been allocated."

  problem%N_parameters = size(problem%state_vector)
  print *,"Detected N_parameters=",problem%N_parameters

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_pounders)
     stop "Error! The petsc_pounders algorithm is for least-squares problems only."
  case (mango_algorithm_petsc_nm)
     call mango_optimize_petsc(problem, objective_function)
  case (mango_algorithm_hopspack)
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
       mango_algorithm_nlopt_ln_sbplx  )
     call mango_optimize_nlopt(problem, objective_function)
  case default
     print "(a,a)","Error! Unrecognized algorithm: ",trim(problem%algorithm)
     stop
  end select

end subroutine mango_optimize

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine mango_optimize_least_squares(problem, residual_function)

  use mango

  implicit none

  type(mango_least_squares_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function

  !-------------------------------------------

  print *,"Hello world from mango_optimize_least_squares."

  if (.not. allocated(problem%state_vector)) stop "State vector has not been allocated."
  if (.not. allocated(problem%targets)) stop "Targets vector has not been allocated."
  if (.not. allocated(problem%sigmas)) stop "Sigmas vector has not been allocated."

  problem%N_parameters = size(problem%state_vector)
  print *,"Detected N_parameters=",problem%N_parameters

  problem%N_terms = size(problem%targets)
  print *,"Detected N_terms=",problem%N_terms
  if (size(problem%sigmas) .ne. problem%N_terms) stop "size(sigmas) .ne. size(targets)"

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_pounders)
     call mango_optimize_least_squares_petsc(problem, residual_function)
  case (mango_algorithm_petsc_nm)
     call mango_optimize_petsc(problem, least_squares_to_single_objective)
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
       mango_algorithm_nlopt_ln_sbplx  )
     call mango_optimize_nlopt(problem, least_squares_to_single_objective)
  case default
     print "(a,a)","Error! Unrecognized algorithm: ",trim(problem%algorithm)
     stop
  end select

contains

  subroutine least_squares_to_single_objective(x, f, failed)

    implicit none

    double precision, intent(in) :: x(:)
    double precision, intent(out) :: f
    logical, intent(out) :: failed

    double precision, allocatable :: residual_vector(:)


    print *,"Hello from least_squares_to_single_objective"
    allocate(residual_vector(problem%N_terms))
    call residual_function(x, residual_vector, failed)
    f = sum(((residual_vector - problem%targets) / problem%sigmas) ** 2)
    print *,"residual_vector:",residual_vector,", f:",f
    deallocate(residual_vector)

  end subroutine least_squares_to_single_objective


end subroutine mango_optimize_least_squares
