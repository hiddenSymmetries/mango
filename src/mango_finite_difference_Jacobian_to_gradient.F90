subroutine mango_finite_difference_Jacobian_to_gradient(problem, residual_function, state_vector, base_case_objective_function, gradient)

  ! gradient should have been allocated already, with size problem%N_parameters.

  use mango

  implicit none

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function
  double precision, intent(in) :: state_vector(problem%N_parameters)
  double precision, intent(out) :: base_case_objective_function, gradient(problem%N_parameters)
  double precision, allocatable :: base_case_residual_vector(:), Jacobian(:,:)
  integer :: j, k
  double precision :: temp

  !-----------------------------------------------------------------

  print "(a,i3)","Hello from mango_finite_difference_Jacobian_to_gradient from proc",problem%mpi_rank_world

  if (.not.problem%proc0_world) then
     stop "I thought only proc0_world would get here..."
  end if

  allocate(base_case_residual_vector(problem%N_terms))
  allocate(Jacobian(problem%N_terms, problem%N_parameters))

  call mango_finite_difference_Jacobian(problem, residual_function, state_vector, base_case_residual_vector, Jacobian)

  base_case_objective_function = sum(((base_case_residual_vector - problem%targets) / problem%sigmas) ** 2)

  gradient = 0
  do j = 1, problem%N_terms
     temp = (base_case_residual_vector(j) - problem%targets(j)) / problem%sigmas(j)
     do k = 1, problem%N_parameters
        gradient(k) = gradient(k) + 2 * (temp / problem%sigmas(j)) * Jacobian(j,k)
     end do
  end do

  deallocate(base_case_residual_vector, Jacobian)

end subroutine mango_finite_difference_Jacobian_to_gradient
