program rosenbrock

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function

  !---------------------------------------------

  print *,"Hello world from rosenbrock"

  call mpi_init(ierr)

  call mango_read_namelist(problem,'../input/mango_in.rosenbrock')

  call mango_mpi_init(problem, MPI_COMM_WORLD)

  problem%output_filename = '../output/mango_out.rosenbrock'

  ! Set initial condition
  allocate(problem%state_vector(2))
  problem%state_vector = 0.0d+0
     
  allocate(problem%targets(2))
  problem%targets(1) = 1.0d+0
  problem%targets(2) = 0.0d+0
     
  allocate(problem%sigmas(2))
  problem%sigmas(1) = 1.0d+0
  problem%sigmas(2) = 1.0d-1
  !problem%sigmas(2) = 0.5d+0
     
  if (problem%proc0_worker_groups) then
     call mango_optimize_least_squares(problem, residual_function)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
     if (ierr .ne. 0) print *,"Error A on proc0!"
  else
     call worker(problem)
  end if

  call mango_mpi_finalize(problem)
  call mpi_finalize(ierr)

  print *,"Good bye!"

end program rosenbrock


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine residual_function(problem, x, f, failed)

  use mango

  implicit none

  include 'mpif.h'

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed
  !double precision, parameter :: a = 1, b=100
  integer :: data(1), ierr

  !---------------------------------------------

  print *,"rosenbrock/residual_function: size(x)=",size(x),", size(f)=",size(f)

  ! Mobilize the workers in the group with this group leader:
  data = problem%worker_group
  call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
  if (ierr .ne. 0) print *,"Error on proc0 in residual_function!"

  !f = (a - x(1)) ** 2 + b * (x(2) - x(1)**2) ** 2
  f(1) = x(1)
  f(2) = x(2) - x(1) ** 2
  failed = .false.

  print *,"rosenbrock/residual_function: x=",x,", f=",f

end subroutine residual_function

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango

  implicit none

  include 'mpif.h'

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  integer :: ierr, mpi_rank, data(1)

  call mpi_comm_rank(MPI_COMM_WORLD, mpi_rank, ierr)
  do
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
     if (ierr .ne. 0) print *,"Error on proc",mpi_rank," in worker: bcast!"
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",mpi_rank," is exiting."
        exit
     else
        print "(a,i4,a,i4)", "Proc",mpi_rank," is doing calculation",data(1)
     end if
  end do

end subroutine worker
