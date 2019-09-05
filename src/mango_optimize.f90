subroutine mango_optimize(problem)

  use mango_problem_mod

  implicit none

  type(mango_problem) :: problem

  !-------------------------------------------

  print *,"Hello world from mango_optimize."
  print *,"problem%least_squared=",problem%least_squares

end subroutine mango_optimize
