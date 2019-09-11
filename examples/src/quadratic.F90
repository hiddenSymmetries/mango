#define N 2

program quadratic

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  type(mango_least_squares_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function

  integer :: j

  !---------------------------------------------

  print *,"Hello world from quadratic"

  call mpi_init(ierr)

  call mango_read_namelist(problem,'../input/mango_in.quadratic')

  call mango_mpi_init(problem, MPI_COMM_WORLD)

  problem%output_filename = '../output/mango_out.quadratic'

  ! Set initial condition
  allocate(problem%state_vector(N))
  problem%state_vector = 0.0d+0

  allocate(problem%targets(N))
  allocate(problem%sigmas(N))
  do j = 1, N
     problem%targets(j) = j
     problem%sigmas(j)  = j
  end do

  if (problem%proc0_worker_groups) then
     call mango_optimize_least_squares(problem, residual_function)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
  else
     call worker(problem)
  end if

  call mango_mpi_finalize(problem)
  call mpi_finalize(ierr)

  print *,"Good bye!"

end program quadratic


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine residual_function(problem, x, f, failed)

  use mango

  implicit none

  include 'mpif.h'

  type(mango_least_squares_problem) :: problem
  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed
  integer :: data(1), ierr

  !---------------------------------------------

  print *,"quadratic/residual_function: size(x)=",size(x),", size(f)=",size(f)

  ! Mobilize the workers in the group with this group leader:
  data = problem%worker_group
  call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)

  f = x
  failed = .false.

  print *,"quadratic/residual_function: x=",x,", f=",f

end subroutine residual_function

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango

  implicit none

  include 'mpif.h'

  type(mango_least_squares_problem) :: problem
  integer :: ierr, mpi_rank, data(1)

  call mpi_comm_rank(MPI_COMM_WORLD, mpi_rank, ierr)
  do
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",mpi_rank," is exiting."
        exit
     else
        print "(a,i4,a,i4)", "Proc",mpi_rank," is doing calculation",data(1)
     end if
  end do

end subroutine worker
