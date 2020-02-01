! Minimize f = (x[0] - x[1]) ^ 2 + x[1] ^ 2
! subject to
! x[0] in [2, 4]
! x[1] in [-3, 1]
!
! The solution is (x[0], x[1]) = (2, 1), and at this point f = 2.
!
! This script also demonstrates set_relative_bound_constraints().

#define N_dim 2
#define verbose_level 0

program active_bound_constraints

  use mango
  !use iso_c_binding

  implicit none

  include 'mpif.h'
  
  character(len=*), parameter :: extension = "active_bound_constraints_f"
  integer :: ierr, N_procs, mpi_rank
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, dimension(N_dim) :: targets = (/ 0.0d+0, 0.0d+0 /), sigmas = (/ 1.0d+0, 1.0d+0 /)
  double precision, dimension(N_dim) :: lower_bounds
  double precision, dimension(N_dim) :: upper_bounds
  double precision, dimension(N_dim) :: state_vector = (/ 3.0d+0, -2.0d+0 /) ! Initial condition
  double precision, dimension(N_dim) :: best_residual_function
  integer :: j
  double precision :: best_objective_function

  !---------------------------------------------

  if (verbose_level > 0) print *,"Hello world from active_bound_constraints_f"
  call mpi_init(ierr)

  call mango_problem_create_least_squares(problem,N_dim,state_vector,N_dim,targets,sigmas,best_residual_function,residual_function)
  if (verbose_level > 0) print *,"Here comes state vector:",state_vector
  call mango_set_verbose(problem, verbose_level)
  call mango_read_input_file(problem, "../input/mango_in." // extension)
  call mango_set_output_filename(problem, "../output/mango_out." // extension)
  call mango_mpi_init(problem, MPI_COMM_WORLD)
  call mango_set_max_function_evaluations(problem, 500)
  call mango_set_bound_constraints(problem, lower_bounds, upper_bounds)

  call mango_set_relative_bound_constraints(problem, 0.5d+0, 2.0d+0, 0.0d+0, .false.)
  if (verbose_level > 0) then
     print *, "After calling set_relative_bound_constraints(0.5, 2.0, 0.0, false),"
     print *, "  Lower bounds = ", lower_bounds
     print *, "  Upper bounds = ", upper_bounds
  end if
  call mango_set_relative_bound_constraints(problem, 0.5d+0, 2.0d+0, 0.0d+0, .true.)
  if (verbose_level > 0) then
     print *, "After calling set_relative_bound_constraints(0.5, 2.0, 0.0, true),"
     print *, "  Lower bounds = ", lower_bounds
     print *, "  Upper bounds = ", upper_bounds
  end if

  lower_bounds = (/ 2.0d+0, -3.0d+0 /)
  upper_bounds = (/ 4.0d+0,  1.0d+0 /)

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


subroutine residual_function(N_parameters, x, N_terms, f, failed, problem, user_data)
  use iso_c_binding
  implicit none
  integer(C_int), intent(in) :: N_parameters, N_terms
  real(C_double), intent(in) :: x(N_parameters)
  real(C_double), intent(out) :: f(N_terms)
  integer(C_int), intent(out) :: failed
  type(mango_problem), value, intent(in) :: problem
  type(C_ptr), value, intent(in) :: user_data
  integer :: j

  if (verbose_level > 0) then
     print *,"Hi from fortran. N=",N_parameters," size(x)=",size(x), ", size(f)=",size(f)
     print *,"mango_get_N_parameters(problem):",mango_get_N_parameters(problem)
     print *,"mango_get_N_terms(problem):",mango_get_N_terms(problem)
     print *,"This line might segfault:",mango_get_N_procs_worker_groups(problem)
     print *,"This line too:",mango_get_mpi_rank_world(problem)
  end if

  call mango_mobilize_workers(problem)

  f(1) = x(1) - x(2)
  f(2) = x(2)

  failed = 0

end subroutine residual_function

end program active_bound_constraints

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
