program quadratic

  use mango

  implicit none

  type(mango_problem) :: my_problem

  !---------------------------------------------

  print *,"Hello world from quadratic"

  my_problem%least_squares = .true.

  call mango_optimize(my_problem)

  print *,"Good bye!"

end program quadratic
