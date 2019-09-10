program rosenbrock

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  type(mango_least_squares_problem) :: my_problem
  procedure(mango_residual_function_interface) :: residual_function

  !---------------------------------------------

  print *,"Hello world from rosenbrock"

  call mpi_init(ierr)

  call mpi_comm_rank(MPI_COMM_WORLD, mpi_rank, ierr)
  call mpi_comm_size(MPI_COMM_WORLD, N_procs, ierr)
  proc0 = (mpi_rank==0)

  if (proc0) then
     !my_problem%objective_function => objective_function
     my_problem%mpi_comm = MPI_COMM_WORLD
     my_problem%algorithm = mango_algorithm_petsc_nm
     my_problem%output_filename = '../output/mango_out.rosenbrock'
     
     ! Set initial condition
     allocate(my_problem%state_vector(2))
     my_problem%state_vector = 0.0d+0
     
     allocate(my_problem%targets(2))
     my_problem%targets(1) = 1.0d+0
     my_problem%targets(2) = 0.0d+0
     
     allocate(my_problem%sigmas(2))
     my_problem%sigmas(1) = 1.0d+0
     my_problem%sigmas(2) = 1.0d-1
     !my_problem%sigmas(2) = 0.5d+0
     
     call mango_read_namelist(my_problem,'../input/mango_in.rosenbrock')
     call mango_optimize_least_squares(my_problem, residual_function)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
     if (ierr .ne. 0) print *,"Error A on proc0!"
  else
     call worker()
  end if

  call mpi_finalize(ierr)

  print *,"Good bye!"

end program rosenbrock


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine residual_function(x, f, failed)

  use mango

  implicit none

  include 'mpif.h'

  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed
  !type(mango_problem) :: problem
  !double precision, parameter :: a = 1, b=100
  integer :: data(1), ierr

  !---------------------------------------------

  print *,"rosenbrock/residual_function: size(x)=",size(x),", size(f)=",size(f)

  ! Mobilize workers:
  data = 1
  call mpi_bcast(data,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
  if (ierr .ne. 0) print *,"Error on proc0 in residual_function!"

  !f = (a - x(1)) ** 2 + b * (x(2) - x(1)**2) ** 2
  f(1) = x(1)
  f(2) = x(2) - x(1) ** 2
  failed = .false.

  print *,"rosenbrock/residual_function: x=",x,", f=",f

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
     if (ierr .ne. 0) print *,"Error on proc",mpi_rank," in worker: bcast!"
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",mpi_rank," is exiting."
        exit
     else
        print "(a,i4,a,i4)", "Proc",mpi_rank," is doing calculation",data(1)
     end if
  end do

end subroutine worker
