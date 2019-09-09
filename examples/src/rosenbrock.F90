program rosenbrock

  use mango
#if defined(MANGO_PETSC_AVAILABLE)
!#include <petsc/finclude/petscsys.h>
  use petscsys ! Needed only for PETSC_NULL_CHARACTER
#endif

  implicit none

!  include 'mpif.h'
  
  integer :: ierr
  type(mango_least_squares_problem) :: my_problem
  !procedure(objective_function_interface) :: objective_function
  procedure(mango_residual_function_interface) :: residual_function

  !---------------------------------------------

  print *,"Hello world from rosenbrock"

#if defined(MANGO_PETSC_AVAILABLE)
  print *,"Using PETSc"
  call PETSCInitialize(PETSC_NULL_CHARACTER, ierr)
#else
  print *,"Not using PETSc"
  call mpi_init(ierr)
#endif

  !my_problem%least_squares = .false.
  !my_problem%objective_function => objective_function
  my_problem%mpi_comm = MPI_COMM_WORLD
  my_problem%algorithm = mango_algorithm_petsc_nm

  ! Set initial condition
  allocate(my_problem%state_vector(2))
  my_problem%state_vector = 0.0d+0

  allocate(my_problem%targets(2))
  my_problem%targets(1) = 1.0d+0
  my_problem%targets(2) = 0.0d+0

  allocate(my_problem%sigmas(2))
  my_problem%sigmas(1) = 1.0d+0
  !my_problem%sigmas(2) = 1.0d-1
  my_problem%sigmas(2) = 0.5d+0

  call mango_read_namelist(my_problem,'../input/mango_in.rosenbrock')
  call mango_optimize_least_squares(my_problem, residual_function)

#if defined(MANGO_PETSC_AVAILABLE)
  print *,"Using PETSc"
  call PETScFinalize(ierr)
#else
  print *,"Not using PETSc"
  call mpi_finalize(ierr)
#endif

  print *,"Good bye!"

end program rosenbrock


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine residual_function(x, f, failed)

  use mango

  implicit none

  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed
  !type(mango_problem) :: problem
  !double precision, parameter :: a = 1, b=100
  
  !---------------------------------------------

  print *,"rosenbrock/residual_function: size(x)=",size(x),", size(f)=",size(f)

  !f = (a - x(1)) ** 2 + b * (x(2) - x(1)**2) ** 2
  f(1) = x(1)
  f(2) = x(2) - x(1) ** 2
  failed = .false.

  !print *,"Hello from rosenbrock/objective_function."
  print *,"rosenbrock/residual_function: x=",x,", f=",f

end subroutine residual_function
