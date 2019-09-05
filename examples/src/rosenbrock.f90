program rosenbrock

  use mango

  implicit none

  type(mango_problem) :: my_problem

  !---------------------------------------------

  print *,"Hello world from rosenbrock"

  my_problem%least_squares = .false.

  call mango_optimize(my_problem)

  print *,"Good bye!"

end program rosenbrock
