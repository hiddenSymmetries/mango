#define N 3

program quadratic

  use mango
#if defined(MANGO_PETSC_AVAILABLE)
!#include <petsc/finclude/petscsys.h>
  use petscsys ! Needed only for PETSC_NULL_CHARACTER
#endif

  implicit none

!  include 'mpif.h'
  
  integer :: ierr
  type(mango_problem) :: my_problem
  !procedure(objective_function_interface) :: objective_function
  procedure(mango_objective_function_interface) :: objective_function

  !---------------------------------------------

  print *,"Hello world from quadratic"

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
  allocate(my_problem%state_vector(N))
  my_problem%state_vector = 0.0d+0

  call mango_read_namelist(my_problem,'../input/mango_in.quadratic')
  call mango_optimize(my_problem, objective_function)

#if defined(MANGO_PETSC_AVAILABLE)
  print *,"Using PETSc"
  call PETScFinalize(ierr)
#else
  print *,"Not using PETSc"
  call mpi_finalize(ierr)
#endif

  print *,"Good bye!"

end program quadratic


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine objective_function(x, f, failed)

  use mango

  implicit none

  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f
  logical, intent(out) :: failed
  !type(mango_problem) :: problem
  !double precision, parameter :: a = 1, b=100
  integer :: j
  
  !---------------------------------------------

  print *,"quadratic/objective_function: size(x)=",size(x)

  f = 0
  do j = 1, N
     f = f + abs(x(j) - j) / j
  end do

  failed = .false.

  !print *,"Hello from quadratic/objective_function."
  print *,"quadratic/objective_function: x=",x,", f=",f

end subroutine objective_function
