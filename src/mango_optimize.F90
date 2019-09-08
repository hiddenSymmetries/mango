subroutine mango_optimize(problem)

  use mango_problem_mod

  implicit none

  type(mango_problem) :: problem
  double precision :: x(2), f
  logical :: failed

  !-------------------------------------------

  print *,"Hello world from mango_optimize."
  print *,"problem%least_squared=",problem%least_squares

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_nm,mango_algorithm_petsc_pounders)
     call mango_optimize_petsc(problem)
  case (mango_algorithm_hopspack)
     call mango_optimize_hopspack(problem)
  case (mango_algorithm_nlopt_ln_cobyla)
     call mango_optimize_nlopt(problem)
  case default
     print "(a,a)","Error! Unrecognized algorithm: ",trim(problem%algorithm)
     stop
  end select

end subroutine mango_optimize
