subroutine mango_optimize_petsc(problem)

  use mango_problem_mod
#ifdef MANGO_PETSC_AVAILABLE
#include <petsc/finclude/petsctao.h>
  use petsctao
#endif

  implicit none

  type(mango_problem) :: problem

#ifdef MANGO_PETSC_AVAILABLE
  PetscErrorCode :: ierr
  PetscInt :: user_context ! Not used
  Tao :: my_tao
  Vec :: tao_state_vec, tao_residual_vec
#endif

  !external mango_petsc_objective_function

  !-------------------------------------------

#ifdef MANGO_PETSC_AVAILABLE

  print *,"Hello world from mango_optimize_petsc."

  call TaoCreate(PETSC_COMM_WORLD, my_tao, ierr)

  !call VecCreateSeq(PETSC_COMM_WORLD, 2, x0, ierr)                                                                                                                  
  call VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, 2, tao_state_vec, ierr)
  call VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, 1, tao_residual_vec, ierr)
  call VecSet(tao_state_vec, 0.0d+0, ierr)
  call TaoSetInitialVector(my_tao, tao_state_vec, ierr)

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_nm)
     call TaoSetType(my_tao, TAONM, ierr)
     call TaoSetObjectiveRoutine(my_tao, mango_petsc_objective_function, user_context, ierr)
  case (mango_algorithm_petsc_pounders)
     call TaoSetType(my_tao, TAOPOUNDERS, ierr)
     !call TaoSetResidualRoutine(my_tao, f, objective_function, user_context, ierr) ! Recent versions of PETSc                                                          
     !call TaoSetSeparableObjectiveRoutine(my_tao, f, objective_function, user_context, ierr)
  case default
     print "(a)","Error! Should not get here."
  end select

  call TaoSetFromOptions(my_tao, ierr)

  call TaoSolve(my_tao, ierr)

  call TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF, ierr)

  call TaoDestroy(my_tao, ierr)


contains

  subroutine mango_petsc_objective_function(my_tao, x, f, user_context, ierr)

    implicit none

    Tao :: my_tao
    Vec :: x
    PetscReal :: f
    PetscInt :: user_context
    PetscErrorCode :: ierr
    PetscScalar, pointer :: x_array(:)
    logical :: failed

    call VecGetArrayF90(x, x_array, ierr)
    !print *,"Hello from mango_petsc_objective_function. x=",x_array

    !f = log(0.2 + (x_array(1) + 1) ** 2 + (x_array(2) + 2) ** 2)
    call problem%objective_function(x_array, f, failed)
    if (failed) f = mango_huge

    !print *,"x=",x_array,", f=",f

    call VecRestoreArrayF90(x, x_array, ierr)
    
  end subroutine mango_petsc_objective_function

#else
  stop "Error! A PETSc algorithm was requested, but Mango was compiled without PETSc support"
#endif

end subroutine mango_optimize_petsc

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1

