subroutine mango_dummy_residual_function(problem,x,f,failed)

  use mango

  implicit none

  type(mango_problem) :: problem
  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed

  stop "The code somehow got to mango_dummy_residual_function. Code should not get here!"

end subroutine mango_dummy_residual_function
