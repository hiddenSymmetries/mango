subroutine mango_finite_difference_Jacobian(problem, residual_function, state_vector, base_case_residual_function, Jacobian)

  ! Jacobian should have been allocated already, with size (problem%N_terms,problem%N_parameters).
  ! Also base_case_residual_function should have been allocated already, with size problem%N_terms.
  use mango

  implicit none

  include 'mpif.h'

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function
  double precision, intent(in) :: state_vector(problem%N_parameters)
  double precision, intent(out) :: base_case_residual_function(problem%N_terms), Jacobian(problem%N_terms,problem%N_parameters)
  integer :: j_evaluation, N_evaluations, j_parameter, j_term
  double precision, allocatable :: objective_functions(:)
  double precision, allocatable :: perturbed_state_vector(:), residual_functions(:,:), state_vectors(:,:)
  integer :: ierr, data(1)
  logical :: failed

  !-----------------------------------------------------------------

  print "(a,i3)","Hello from mango_finite_difference_Jacobian from proc",problem%mpi_rank_world

  if (problem%proc0_world) then
     ! Tell the group leaders to start this subroutine
     data = 1
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_group_leaders,ierr)
  end if

  ! Only proc0_world has a meaningful state vector at this point.
  call mpi_bcast(state_vector, problem%N_parameters, MPI_DOUBLE_PRECISION, 0, problem%mpi_comm_group_leaders, ierr)
  print *,"proc",problem%mpi_rank_world," state_vector:",state_vector

  allocate(perturbed_state_vector(problem%N_parameters))

  if (problem%centered_differences) then
     N_evaluations = problem%N_parameters + 1
  else
     N_evaluations = problem%N_parameters * 2 + 1
  end if
  allocate(residual_functions(problem%N_terms, N_evaluations))
  allocate(objective_functions(N_evaluations))
  allocate(state_vectors(problem%N_parameters, N_evaluations))
  base_case_residual_function = 0
  residual_functions = 0

  ! Build the set of state vectors that will be considered.
  do j_evaluation = 1, N_evaluations
     perturbed_state_vector = state_vector
     if (j_evaluation == 1) then
        ! This is the base case, so do not perturb the state vector.
     elseif (j_evaluation <= problem%N_parameters + 1) then
        ! We are doing a forward step
        perturbed_state_vector(j_evaluation - 1) = perturbed_state_vector(j_evaluation - 1) + problem%finite_difference_step_size
     else
        ! We must be doing a backwards step
        perturbed_state_vector(j_evaluation - 1 - problem%N_parameters) = perturbed_state_vector(j_evaluation - 1 - problem%N_parameters) - problem%finite_difference_step_size
     end if
     state_vectors(:,j_evaluation) = perturbed_state_vector
  end do

  if (problem%proc0_world) then
     print *,"Here comes state_vectors:"
     do j_parameter = 1, problem%N_parameters
        print *,state_vectors(j_parameter,:)
     end do
  end if

  do j_evaluation = 1, N_evaluations
     if (mod(j_evaluation-1, problem%N_worker_groups) == problem%mpi_rank_group_leaders) then
        call residual_function(problem, state_vectors(:,j_evaluation), residual_functions(:,j_evaluation), failed)
     end if
  end do

  ! Send results back to the world master.
  ! Make sure not to reduce over MPI_COMM_WORLD, since then the objective function values will be multiplied by # of workers per worker group.
  if (problem%proc0_world) then
     call mpi_reduce(MPI_IN_PLACE, residual_functions, N_evaluations * problem%N_terms, &
          MPI_DOUBLE_PRECISION, MPI_SUM, 0, problem%mpi_comm_group_leaders, ierr)
  else
     call mpi_reduce(residual_functions, residual_functions, N_evaluations * problem%N_terms, &
          MPI_DOUBLE_PRECISION, MPI_SUM, 0, problem%mpi_comm_group_leaders, ierr)
  end if

  ! proc0_world now has all the residual vectors, so it can compute the corresponding values of the total objective function.
  if (problem%proc0_world) then
     do j_evaluation = 1, N_evaluations
        objective_functions(j_evaluation) = sum(((residual_functions(:,j_evaluation) - problem%targets) / problem%sigmas) ** 2)
     end do
  end if

  ! Print results.
  if (problem%proc0_world) then
     do j_evaluation = 1, N_evaluations
        problem%function_evaluations = problem%function_evaluations + 1
        write (problem%output_unit,"(i7)",advance="no") problem%function_evaluations
        do j_parameter = 1, problem%N_parameters
           write (problem%output_unit,"(a,es24.15)",advance="no") ",", state_vectors(j_parameter,j_evaluation)
        end do
        write (problem%output_unit,"(a,es24.15)",advance="no") ",", objective_functions(j_evaluation)
        do j_term = 1, problem%N_terms
           write (problem%output_unit,"(a,es24.15)",advance="no") ",", residual_functions(j_term,j_evaluation)
        end do
        write (problem%output_unit,"(a)") ""
        call flush(problem%output_unit)
     end do
  end if

  ! Evaluate the finite differences
  base_case_residual_function = residual_functions(:, 1)
  if (problem%centered_differences) then
     do j_parameter = 1, problem%N_parameters
        Jacobian(:, j_parameter) = (residual_functions(:, j_parameter+1) - residual_functions(:, j_parameter+1+problem%N_parameters)) &
             / (2 * problem%finite_difference_step_size)
     end do
  else
     do j_parameter = 1, problem%N_parameters
        Jacobian(:, j_parameter) = (residual_functions(:, j_parameter+1) - base_case_residual_function) / problem%finite_difference_step_size
     end do
  end if

  ! Clean up
  deallocate(perturbed_state_vector, state_vectors, residual_functions, objective_functions)

end subroutine mango_finite_difference_Jacobian
