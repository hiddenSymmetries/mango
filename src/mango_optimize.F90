subroutine mango_optimize(problem)

  use mango_problem_mod

  implicit none

  type(mango_problem) :: problem
  double precision :: x(2), f
  logical :: failed

  !-------------------------------------------

  print *,"Hello world from mango_optimize."
  print *,"problem%least_squared=",problem%least_squares

  x(1) = 0.5
  x(2) = 2
  call problem%objective_function(x,f,failed)

end subroutine mango_optimize
