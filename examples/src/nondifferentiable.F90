#define N 3

program nondifferentiable

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  type(mango_problem) :: my_problem
  procedure(mango_objective_function_interface) :: objective_function

  !---------------------------------------------

  print *,"Hello world from nondifferentiable"

  call mpi_init(ierr)
  call mpi_comm_rank(MPI_COMM_WORLD, mpi_rank, ierr)
  call mpi_comm_size(MPI_COMM_WORLD, N_procs, ierr)
  proc0 = (mpi_rank==0)

  if (proc0) then
     !my_problem%objective_function => objective_function
     my_problem%mpi_comm = MPI_COMM_WORLD
     my_problem%algorithm = mango_algorithm_petsc_nm
     my_problem%output_filename = '../output/mango_out.nondifferentiable'
     
     ! Set initial condition
     allocate(my_problem%state_vector(N))
     my_problem%state_vector = 0.0d+0
     
     call mango_read_namelist(my_problem,'../input/mango_in.nondifferentiable')
     call mango_optimize(my_problem, objective_function)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
  else
     call worker()
  end if

  call mpi_finalize(ierr)

  print *,"Good bye!"

end program nondifferentiable


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine objective_function(x, f, failed)

  use mango

  implicit none

  include 'mpif.h'

  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f
  logical, intent(out) :: failed
  integer :: j
  integer :: data(1), ierr

  !---------------------------------------------

  print *,"nondifferentiable/objective_function: size(x)=",size(x)

  ! Mobilize workers:
  data = 1
  call mpi_bcast(data,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)

  f = 0
  do j = 1, N
     f = f + abs(x(j) - j) / j
  end do

  failed = .false.

  print *,"nondifferentiable/objective_function: x=",x,", f=",f

end subroutine objective_function

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker()

  implicit none

  include 'mpif.h'

  integer :: ierr, mpi_rank, data(1)

  call mpi_comm_rank(MPI_COMM_WORLD, mpi_rank, ierr)
  do
     call mpi_bcast(data,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",mpi_rank," is exiting."
        exit
     else
        print "(a,i4,a,i4)", "Proc",mpi_rank," is doing calculation",data(1)
     end if
  end do

end subroutine worker
