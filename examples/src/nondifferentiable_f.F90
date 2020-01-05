! This example demonstrates several things:
! * An objective function that cannot be written in least-squares form.
! * Passing a derived data type to the objective/residual function using the user_data field.

#define N_dim 3
#define verbose_level 0

module my_type_module
  type my_type
     integer :: i
     double precision :: f
  end type my_type
end module my_type_module

program nondifferentiable

  use mango
  use iso_c_binding, only: c_loc
  use my_type_module

  implicit none

  include 'mpif.h'
  
  character(len=*), parameter :: extension = "nondifferentiable_f"
  integer :: ierr, N_procs, mpi_rank
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, dimension(N_dim) :: state_vector
  double precision :: best_objective_function
  !external objective_function
  !procedure(objective_function_interface), pointer :: objective_function
  integer :: dummy = 13
  type(my_type), target :: my_data

  !---------------------------------------------

  if (verbose_level > 0) print *,"Hello world from nondifferentiable_f"
  !print *,"c_funloc(objective_function):",c_funloc(objective_function)
  call mpi_init(ierr)

  my_data%i = 7
  my_data%f = 2.71828182845905d+0

  if (verbose_level > 0) then
     print *,"Is foobar a valid algorithm? ", mango_does_algorithm_exist('foobar')
     print *,"Is petsc_pounders a valid algorithm? ", mango_does_algorithm_exist('petsc_pounders')
     print *,"Is 'petsc_pounders ' a valid algorithm? ", mango_does_algorithm_exist('petsc_pounders ')
     print *,"Is petsc_nm_blurg a valid algorithm? ", mango_does_algorithm_exist('petsc_nm_blurg')
     print *,"Is petsc_n a valid algorithm? ", mango_does_algorithm_exist('petsc_n')
     print *,"Is nlopt_ld_var2 a valid algorithm? ", mango_does_algorithm_exist('nlopt_ld_var2')
  end if

  state_vector = 0
  call mango_problem_create(problem,N_dim,state_vector,dummy,objective_function)
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

  if (mango_get_proc0_worker_groups(problem)) then
     best_objective_function = mango_optimize(problem)
     call mango_stop_workers(problem)
  else
     call worker(problem)
  end if

  if (mango_get_proc0_world(problem) .and. (verbose_level > 0)) then
     print *,"Best state vector:",state_vector
     print *,"Best objective function: ",best_objective_function
     print *,"Best function evaluation was ",mango_get_best_function_evaluation(problem)
  end if

  call mango_problem_destroy(problem)

  call mpi_finalize(ierr)

  if (verbose_level > 0) print *,"Good bye!"

contains

!subroutine objective_function(N, x, f)
!  implicit none
!  integer, intent(in) :: N
!  double precision, intent(in) :: x(N)
!  double precision, intent(out) :: f
!
!  print *,"Hi from fortran. N=",N," size(x)=",size(x)
!  f = sum((x-2)*(x-2))
!  print *,"In fortran, x=",x,", f=",f
!
!end subroutine objective_function

subroutine objective_function(N, x, f, failed, problem, void_user_data)
  use iso_c_binding
  use my_type_module
  implicit none
  integer(C_int), intent(in) :: N
  real(C_double), intent(in) :: x(N)
  real(C_double), intent(out) :: f
  integer(C_int), intent(out) :: failed
  type(mango_problem), value, intent(in) :: problem
  type(C_ptr), value, intent(in) :: void_user_data
  integer :: j
  type(my_type), pointer :: user_data

  if (verbose_level > 0) print *,"Hi from fortran. N=",N," size(x)=",size(x)

  call mango_mobilize_workers(problem)
  
  ! Check that user_data was passed successfully
  call c_f_pointer(void_user_data, user_data) ! This line effectively casts (void*) to (my_type*)
  if ((user_data%i .ne. 7) .or. (abs(user_data%f - 2.71828182845905d+0) > 1.0d-13)) then
     print *,"Error! user_data was not successfully passed to the objective function."
     print *,"user_data%i = ",user_data%i
     print *,"user_data%f = ",user_data%f
     stop
  end if

  f = 0
  do j = 1, N
     f = f + abs(x(j) - j) / j
  end do

  failed = 0

end subroutine objective_function

end program nondifferentiable

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
