subroutine mango_optimize_hopspack(problem, objective_function)

  use mango

  implicit none

  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function

  !-------------------------------------------

#ifdef MANGO_HOPSPACK_AVAILABLE

  print *,"Hello world from mango_optimize_hopspack."

  select case (trim(problem%algorithm))
  case (mango_algorithm_hopspack)
  case default
     print "(a)","Error! Should not get here!"
  end select

#else
  stop "Error! The HOPSPACK algorithm was requested, but Mango was compiled without HOPSPACK support"
#endif

end subroutine mango_optimize_hopspack
