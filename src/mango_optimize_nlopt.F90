subroutine mango_optimize_nlopt(problem)

  use mango_problem_mod

  implicit none

  type(mango_problem) :: problem

  !-------------------------------------------

#ifdef MANGO_NLOPT_AVAILABLE

  print *,"Hello world from mango_optimize_nlopt."

  select case (trim(problem%algorithm))
  case (mango_algorithm_nlopt_ln_cobyla)
  case default
     print "(a)","Error! Should not get here."
  end select


#else
  stop "Error! A NLOpt algorithm was requested, but Mango was compiled without NLOpt support"
#endif

end subroutine mango_optimize_nlopt
