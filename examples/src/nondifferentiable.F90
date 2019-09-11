#define N 3

program nondifferentiable

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, data(1)
  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function

  !---------------------------------------------

  print *,"Hello world from nondifferentiable"

  call mpi_init(ierr)

  call mango_read_namelist(problem,'../input/mango_in.nondifferentiable')

  call mango_mpi_init(problem, MPI_COMM_WORLD)

  !call mpi_barrier(MPI_COMM_WORLD,ierr)
  !return

  problem%output_filename = '../output/mango_out.nondifferentiable'
     
  ! Set initial condition
  allocate(problem%state_vector(N))
  problem%state_vector = 0.0d+0
     
  if (problem%proc0_worker_groups) then
     ! All group leaders call mango_optimize().
     call mango_optimize(problem, objective_function)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
  else
     call worker(problem)
  end if

  call mango_mpi_finalize(problem)
  call mpi_finalize(ierr)

  print *,"Good bye!"

end program nondifferentiable

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine objective_function(problem, x, f, failed)

  use mango

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem
  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f
  logical, intent(out) :: failed
  integer :: j
  integer :: data(1), ierr

  !---------------------------------------------

  print *,"nondifferentiable/objective_function: size(x)=",size(x)

  ! Mobilize the workers in the group with this group leader:
  data = problem%worker_group
  call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
  
  f = 0
  do j = 1, N
     f = f + abs(x(j) - j) / j
  end do
  
  failed = .false.
  
  print *,"nondifferentiable/objective_function: x=",x,", f=",f
  
end subroutine objective_function

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem
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
