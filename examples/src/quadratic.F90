#define N 2

program quadratic

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  type(mango_least_squares_problem) :: my_problem
  procedure(mango_residual_function_interface) :: residual_function

  integer :: j

  !---------------------------------------------

  print *,"Hello world from quadratic"

  call mpi_init(ierr)
  call mpi_comm_rank(MPI_COMM_WORLD, mpi_rank, ierr)
  call mpi_comm_size(MPI_COMM_WORLD, N_procs, ierr)
  proc0 = (mpi_rank==0)

  if (proc0) then
     !my_problem%objective_function => objective_function
     my_problem%mpi_comm = MPI_COMM_WORLD
     my_problem%algorithm = mango_algorithm_petsc_nm
     my_problem%output_filename = '../output/mango_out.quadratic'

     ! Set initial condition
     allocate(my_problem%state_vector(N))
     my_problem%state_vector = 0.0d+0

     allocate(my_problem%targets(N))
     allocate(my_problem%sigmas(N))
     do j = 1, N
        my_problem%targets(j) = j
        my_problem%sigmas(j)  = j
     end do

     call mango_read_namelist(my_problem,'../input/mango_in.quadratic')
     call mango_optimize_least_squares(my_problem, residual_function)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
  else
     call worker()
  end if

  call mpi_finalize(ierr)

  print *,"Good bye!"

end program quadratic


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine residual_function(x, f, failed)

  use mango

  implicit none

  include 'mpif.h'

  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed
  integer :: data(1), ierr

  !---------------------------------------------

  print *,"quadratic/residual_function: size(x)=",size(x),", size(f)=",size(f)

  ! Mobilize workers:
  data = 1
  call mpi_bcast(data,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)

  f = x
  failed = .false.

  print *,"quadratic/residual_function: x=",x,", f=",f

end subroutine residual_function

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
