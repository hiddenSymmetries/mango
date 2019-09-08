subroutine mango_optimize_hopspack(problem)

  use mango_problem_mod

  implicit none

  type(mango_problem) :: problem

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
