#define N 3

program nondifferentiable

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr
  type(mango_problem) :: my_problem
  procedure(mango_objective_function_interface) :: objective_function

  !---------------------------------------------

  print *,"Hello world from nondifferentiable"

  call mpi_init(ierr)

  !my_problem%objective_function => objective_function
  my_problem%mpi_comm = MPI_COMM_WORLD
  my_problem%algorithm = mango_algorithm_petsc_nm
  my_problem%output_filename = '../output/mango_out.nondifferentiable'

  ! Set initial condition
  allocate(my_problem%state_vector(N))
  my_problem%state_vector = 0.0d+0

  call mango_read_namelist(my_problem,'../input/mango_in.nondifferentiable')
  call mango_optimize(my_problem, objective_function)

  call mpi_finalize(ierr)

  print *,"Good bye!"

end program nondifferentiable


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine objective_function(x, f, failed)

  use mango

  implicit none

  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f
  logical, intent(out) :: failed
  integer :: j
  
  !---------------------------------------------

  print *,"nondifferentiable/objective_function: size(x)=",size(x)

  f = 0
  do j = 1, N
     f = f + abs(x(j) - j) / j
  end do

  failed = .false.

  print *,"nondifferentiable/objective_function: x=",x,", f=",f

end subroutine objective_function
