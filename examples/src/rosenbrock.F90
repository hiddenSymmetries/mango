program rosenbrock

  use mango
#if defined(MANGO_PETSC_AVAILABLE)
!#include <petsc/finclude/petscsys.h>
  use petscsys
#endif

  implicit none

!  include 'mpif.h'
  
  integer :: ierr
  type(mango_problem) :: my_problem
  procedure(objective_function_interface) :: objective_function

  !---------------------------------------------

  print *,"Hello world from rosenbrock"

#if defined(MANGO_PETSC_AVAILABLE)
  print *,"Using PETSc"
  call PETSCInitialize(PETSC_NULL_CHARACTER, ierr)
#else
  print *,"Not using PETSc"
  call mpi_init(ierr)
#endif

  my_problem%least_squares = .false.
  my_problem%N_parameters = 2
  my_problem%objective_function => objective_function
  my_problem%algorithm = mango_algorithm_petsc_nm
  call mango_read_namelist(my_problem,'../input/mango_in.rosenbrock')
  call mango_optimize(my_problem)

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

subroutine objective_function(x, f, failed)

  use mango

  implicit none

  double precision, intent(in) :: x(2)
  double precision, intent(out) :: f
  logical, intent(out) :: failed
  !type(mango_problem) :: problem
  double precision, parameter :: a = 1, b=100
  
  !---------------------------------------------

  f = (a - x(1)) ** 2 + b * (x(2) - x(1)**2) ** 2
  failed = .false.

  !print *,"Hello from rosenbrock/objective_function."
  print *,"x=",x,", f=",f

end subroutine objective_function
