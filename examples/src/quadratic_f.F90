! Copyright 2019, University of Maryland and the MANGO development team.
!
! This file is part of MANGO.
!
! MANGO is free software: you can redistribute it and/or modify it
! under the terms of the GNU Lesser General Public License as
! published by the Free Software Foundation, either version 3 of the
! License, or (at your option) any later version.
!
! MANGO is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
! Lesser General Public License for more details.
!
! You should have received a copy of the GNU Lesser General Public
! License along with MANGO.  If not, see
! <https://www.gnu.org/licenses/>.
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! Minimize f = ((x[0] - 1) / 1)^2 + ((x[1] - 2) / 2)^2 + ((x[2] - 3) / 3)^2
! Bound constraints are set to [-5, 5] for all 3 parameters, just so global
! algorithms can be used, but the bound constraints are not active at the optimum.
! The optimum is x = (1, 2, 3) and the objective function there is f = 0.

! This example demonstrates:
! * Passing an integer to the objective/residual function using the user_data field.

#define N_dim 3
#define verbose_level 0

program quadratic

  use mango_mod
  use iso_c_binding, only: c_loc

  implicit none

  include 'mpif.h'

  character(len=*), parameter :: extension = "quadratic_f"
  integer :: ierr, N_procs, mpi_rank
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, dimension(N_dim) :: state_vector, targets, sigmas, best_residual_function
  double precision, dimension(N_dim) :: upper_bounds, lower_bounds
  !external objective_function
  !procedure(objective_function_interface), pointer :: objective_function
  integer :: j
  double precision :: best_objective_function
  integer, target :: my_data = 7

  !---------------------------------------------

  if (verbose_level > 0) print *,"Hello world from quadratic_f"
  call mpi_init(ierr)

  state_vector = 0
  do j = 1, N_dim
     targets(j) = j
     sigmas(j) = j
  end do
  call mango_problem_create_least_squares(problem,N_dim,state_vector,N_dim,targets,sigmas,best_residual_function,residual_function)
  if (verbose_level > 0) print *,"Here comes state vector:",state_vector
  !call mango_set_algorithm(problem, 2)
  !call mango_set_algorithm_from_string(problem, "nlopt_ln_praxis")
  call mango_set_verbose(problem, verbose_level)
  call mango_read_input_file(problem, "../input/mango_in." // extension)
  call mango_set_output_filename(problem, "../output/mango_out." // extension)
  call mango_mpi_init(problem, MPI_COMM_WORLD)
  call mango_mpi_partition_write(problem, "../output/mango_mpi." // extension)
  call mango_set_centered_differences(problem, .true.)
  call mango_set_max_function_evaluations(problem, 2000)
  call mango_set_user_data(problem, c_loc(my_data))
  call mango_set_N_line_search(problem, 3) ! To make results independent of the # of MPI processes, N_line_search must be set to any positive integer.

  lower_bounds = -5.0d+0
  upper_bounds =  5.0d+0
  call mango_set_bound_constraints(problem, lower_bounds, upper_bounds)

  if (mango_get_proc0_worker_groups(problem)) then
     best_objective_function = mango_optimize(problem)
     call mango_stop_workers(problem)
  else
     call worker(problem)
  end if

  if (mango_get_proc0_world(problem) .and. (verbose_level > 0)) then
     print *,"Best state vector:",state_vector
     print *,"Best objective function: ",best_objective_function
     print *,"Best residual function: ",best_residual_function
     print *,"Best function evaluation was ",mango_get_best_function_evaluation(problem)
  end if

  call mango_problem_destroy(problem)

  call mpi_finalize(ierr)

  if (verbose_level > 0) print *,"Good bye!"

contains


subroutine residual_function(N_parameters, x, N_terms, f, failed, problem, void_user_data)
  use iso_c_binding
  implicit none
  integer(C_int), intent(in) :: N_parameters, N_terms
  real(C_double), intent(in) :: x(N_parameters)
  real(C_double), intent(out) :: f(N_terms)
  integer(C_int), intent(out) :: failed
  type(mango_problem), value, intent(in) :: problem
  type(C_ptr), value, intent(in) :: void_user_data

  integer, pointer :: user_data
  integer :: j

  call mango_mobilize_workers(problem)

  call c_f_pointer(void_user_data, user_data) ! This line effectively casts (void*) to (int*).
  if (user_data .ne. 7) then
     print *, "Error passing user_data to the residual function. user_data=",user_data
     stop
  end if

  if (verbose_level > 0) then
     print *,"Hi from fortran. N=",N_parameters," size(x)=",size(x), ", size(f)=",size(f)
     print *,"mango_get_N_parameters(problem):",mango_get_N_parameters(problem)
     print *,"mango_get_N_terms(problem):",mango_get_N_terms(problem)
     print *,"This line might segfault:",mango_get_N_procs_worker_groups(problem)
     print *,"This line too:",mango_get_mpi_rank_world(problem)
  end if

  f = x(1:N_terms)

  failed = 0

end subroutine residual_function

end program quadratic

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango_mod

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem

  do while (mango_continue_worker_loop(problem))
     if (verbose_level > 0) print "(a,i4,a)", "Proc",mango_get_mpi_rank_world(problem)," could do some work here."
  end do
  if (verbose_level > 0) print "(a,i4,a)", "Proc",mango_get_mpi_rank_world(problem)," is exiting."

end subroutine worker
