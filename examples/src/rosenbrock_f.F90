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
! The standard 2D Rosenbrock banana function:
! f(x,y) = (x - 1)^2 + 100 * (y - x^2)^2
! The optimum is at (x,y) = (1,1), and the objective function there is f=0.
!
! This example demonstrates several things:
! * Unconstrained least-squares minimization.
! * The problem is more challenging than the 'quadratic_c' example due to the objective function having a steep curved valley.
! * Passing a double to the objective/residual function through the 'user_data'.

#define verbose_level 0

program rosenbrock

  use mango
  use iso_c_binding, only: c_loc

  implicit none

  include 'mpif.h'
  
  character(len=*), parameter :: extension = "rosenbrock_f"
  integer :: ierr, N_procs, mpi_rank
  logical :: proc0
  type(mango_problem) :: problem
  double precision, dimension(2) :: state_vector = (/ 0.0d+0, 0.0d+0 /)
  double precision, dimension(2) :: targets      = (/ 1.0d+0, 0.0d+0 /)
  double precision, dimension(2) :: sigmas       = (/ 1.0d+0, 0.1d+0 /)
  double precision :: best_residual_function(2), best_objective_function

  double precision, target :: my_data = 2.71828182845905d+0 ! "target" attribute is necessary since we will use my_data as an argument of c_loc.

  !---------------------------------------------

  if (verbose_level > 0) print *,"Hello world from rosenbrock_f"
  call mpi_init(ierr)

  !call mango_problem_create(problem,2,state_vector,dummy,objective_function)
  call mango_problem_create_least_squares(problem, 2, state_vector, 2, targets, sigmas, best_residual_function, residual_function)
  if (verbose_level > 0) print *,"Here comes state vector:",state_vector
  !call mango_set_algorithm(problem, 2)
  !call mango_set_algorithm_from_string(problem, "nlopt_ln_praxis")
  call mango_set_verbose(problem, verbose_level)
  call mango_read_input_file(problem, "../input/mango_in." // extension)
  call mango_set_output_filename(problem, "../output/mango_out." // extension)
  call mango_mpi_init(problem, MPI_COMM_WORLD)
  call mango_mpi_partition_write(problem, "../output/mango_mpi." // extension)
  call mango_set_max_function_evaluations(problem, 2000)

  call mango_set_user_data(problem, c_loc(my_data))

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
  integer(C_int), intent(in) :: N_parameters
  real(C_double), intent(in) :: x(N_parameters)
  integer(C_int), intent(in) :: N_terms
  real(C_double), intent(out) :: f(N_terms)
  integer(C_int), intent(out) :: failed
  type(mango_problem), value, intent(in) :: problem
  type(C_ptr), value, intent(in) :: void_user_data

  double precision, pointer :: user_data

  if (verbose_level > 0) print *,"Hi from fortran. N_parameters=",N_parameters," size(x)=",size(x)

  call mango_mobilize_workers(problem)

  ! Check that user data was passed correctly
  call c_f_pointer(void_user_data, user_data) ! This line effectively casts (void*) to (double*)
  if (abs(user_data - 2.71828182845905d+0) > 1d-13) then
     print *,"Error passing user_data to the residual function. user_data=",user_data
     stop
  end if

  !f = (x(1) - 1) * (x(1) - 1) + 100 * (x(2) - x(1)*x(1)) * (x(2) - x(1)*x(1))
  f(1) = x(1)
  f(2) = x(2) - x(1) * x(1)
  failed = 0

end subroutine residual_function

end program rosenbrock

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem

  do while (mango_continue_worker_loop(problem))
     if (verbose_level > 0) print "(a,i4,a)", "Proc",mango_get_mpi_rank_world(problem)," could do some work here."
  end do
  if (verbose_level > 0) print "(a,i4,a)", "Proc",mango_get_mpi_rank_world(problem)," is exiting."

end subroutine worker
