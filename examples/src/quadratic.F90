#define N 2

program quadratic

  use mango
#if defined(MANGO_PETSC_AVAILABLE)
!#include <petsc/finclude/petscsys.h>
  use petscsys ! Needed only for PETSC_NULL_CHARACTER
#endif

  implicit none

#if !defined(MANGO_PETSC_AVAILABLE)
  include 'mpif.h'
#endif
  
  integer :: ierr
  type(mango_least_squares_problem) :: my_problem
  !procedure(objective_function_interface) :: objective_function
  procedure(mango_residual_function_interface) :: residual_function

  integer :: j

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

  allocate(my_problem%targets(N))
  allocate(my_problem%sigmas(N))
  do j = 1, N
     my_problem%targets(j) = j
     my_problem%sigmas(j)  = j
  end do

  call mango_read_namelist(my_problem,'../input/mango_in.quadratic')
  call mango_optimize_least_squares(my_problem, residual_function)

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

subroutine residual_function(x, f, failed)

  use mango

  implicit none

  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed
  !type(mango_problem) :: problem
  !double precision, parameter :: a = 1, b=100
  
  !---------------------------------------------

  print *,"quadratic/residual_function: size(x)=",size(x),", size(f)=",size(f)

  f = x
  failed = .false.

  !print *,"Hello from quadratic/objective_function."
  print *,"quadratic/residual_function: x=",x,", f=",f

end subroutine residual_function
