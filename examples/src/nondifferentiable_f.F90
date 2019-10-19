#define N_dim 3

program nondifferentiable

  use mango
  !use iso_c_binding

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, dimension(N_dim) :: state_vector
  !external objective_function
  !procedure(objective_function_interface), pointer :: objective_function
  integer :: dummy = 13

  !---------------------------------------------

  print *,"Hello world from nondifferentiable_f"
  !print *,"c_funloc(objective_function):",c_funloc(objective_function)
  call mpi_init(ierr)

  state_vector = 0
  call mango_problem_create(problem,N_dim,state_vector,dummy,objective_function)
  print *,"Here comes state vector:",state_vector
  !call mango_set_algorithm(problem, 2)
  !call mango_set_algorithm_from_string(problem, "nlopt_ln_praxis")
  call mango_read_input_file(problem, "../input/mango_in.nondifferentiable_f")
  call mango_set_output_filename(problem, "../output/mango_out.nondifferentiable_f")
  call mango_mpi_init(problem, MPI_COMM_WORLD)

!  if (mango_proc0_worker_groups(problem)) then
     call mango_optimize(problem)

!     ! Make workers stop
!     data = -1
!     call mpi_bcast(data,1,MPI_INTEGER,0,mango_mpi_comm_worker_groups(problem),ierr)
!     if (ierr .ne. 0) print *,"Error A on proc0!"
!  else
!     call worker(problem)
!  end if

  call mango_problem_destroy(problem)

  call mpi_finalize(ierr)

  print *,"Good bye!"

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

subroutine objective_function(N, x, f, failed, problem)
  use iso_c_binding
  implicit none
  integer(C_int), intent(in) :: N
  real(C_double), intent(in) :: x(N)
  real(C_double), intent(out) :: f
  integer(C_int), intent(out) :: failed
  type(mango_problem), value, intent(in) :: problem
  integer :: j

  print *,"Hi from fortran. N=",N," size(x)=",size(x)
  !f = sum((x-2)*(x-2))
  !print *,"In fortran, x=",x,", f=",f

  f = 0
  do j = 1, N
     f = f + abs(x(j) - j) / j
  end do

  failed = 0

end subroutine objective_function

end program nondifferentiable
