subroutine mango_optimize_petsc(problem, objective_function)

  use mango_problem_mod
#ifdef MANGO_PETSC_AVAILABLE
#include <petsc/finclude/petsctao.h>
  use petsctao
#endif

  implicit none

  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function

#ifdef MANGO_PETSC_AVAILABLE
  PetscErrorCode :: ierr
  PetscInt :: user_context ! Not used
  Tao :: my_tao
  Vec :: tao_state_vec
  PetscScalar, pointer :: temp_array(:)
#endif

  !external mango_petsc_objective_function

  !-------------------------------------------

#ifdef MANGO_PETSC_AVAILABLE

  print *,"Hello world from mango_optimize_petsc."

  call TaoCreate(PETSC_COMM_WORLD, my_tao, ierr)

  !call VecCreateSeq(PETSC_COMM_WORLD, 2, x0, ierr)                                                                                                                  
  call VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, problem%N_parameters, tao_state_vec, ierr)

  ! Set initial condition
  !call VecSet(tao_state_vec, 0.0d+0, ierr)
  call VecGetArrayF90(tao_state_vec, temp_array, ierr)
  temp_array = problem%state_vector
  call VecRestoreArrayF90(tao_state_vec, temp_array, ierr)
  call TaoSetInitialVector(my_tao, tao_state_vec, ierr)

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_nm)
     call TaoSetType(my_tao, TAONM, ierr)
     call TaoSetObjectiveRoutine(my_tao, mango_petsc_objective_function, user_context, ierr)
  case (mango_algorithm_petsc_pounders)
     stop "Should not get here. For the petsc_pounders algorithm, mango_optimize_least_squares_petsc should be called instead of mango_optimize_petsc."
  case default
     print "(a)","Error! Should not get here."
  end select

  call TaoSetFromOptions(my_tao, ierr)

  call TaoSolve(my_tao, ierr)

  call TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF, ierr)

  call TaoDestroy(my_tao, ierr)
  call VecDestroy(tao_state_vec, ierr)

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
    call objective_function(x_array, f, failed)
    if (failed) f = mango_huge

    !print *,"x=",x_array,", f=",f

    call VecRestoreArrayF90(x, x_array, ierr)
    
  end subroutine mango_petsc_objective_function

#else
  stop "Error! A PETSc algorithm was requested, but Mango was compiled without PETSc support"
#endif

end subroutine mango_optimize_petsc

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine mango_optimize_least_squares_petsc(problem, residual_function)

  use mango_problem_mod
#ifdef MANGO_PETSC_AVAILABLE
#include <petsc/finclude/petsctao.h>
  use petsctao
#endif

  implicit none

  type(mango_least_squares_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function

#ifdef MANGO_PETSC_AVAILABLE
  PetscErrorCode :: ierr
  PetscInt :: user_context ! Not used
  Tao :: my_tao
  Vec :: tao_state_vec, tao_residual_vec
  PetscScalar, pointer :: temp_array(:)
#endif

  !external mango_petsc_objective_function

  !-------------------------------------------

#ifdef MANGO_PETSC_AVAILABLE

  print *,"Hello world from mango_optimize_least_squares_petsc."

  call TaoCreate(PETSC_COMM_WORLD, my_tao, ierr)

  !call VecCreateSeq(PETSC_COMM_WORLD, 2, x0, ierr)                                                                                                                  
  call VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, problem%N_parameters, tao_state_vec, ierr)
  call VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, problem%N_terms, tao_residual_vec, ierr)

  ! Set initial condition
  !call VecSet(tao_state_vec, 0.0d+0, ierr)
  call VecGetArrayF90(tao_state_vec, temp_array, ierr)
  temp_array = problem%state_vector
  call VecRestoreArrayF90(tao_state_vec, temp_array, ierr)
  call TaoSetInitialVector(my_tao, tao_state_vec, ierr)

  print *,"Here comes tao_state_vec:"
  call VecView(tao_state_vec, PETSC_VIEWER_STDOUT_WORLD, ierr)

  select case (trim(problem%algorithm))
  case (mango_algorithm_petsc_nm)
     stop "Should not get here. For the petsc_nm algorithm, mango_optimize_petsc should be called instead of mango_optimize_least_squares_petsc."
  case (mango_algorithm_petsc_pounders)
     call TaoSetType(my_tao, TAOPOUNDERS, ierr)
     !call TaoSetResidualRoutine(my_tao, tao_residual_vec, mango_petsc_residual_function, user_context, ierr) ! Recent versions of PETSc
     call TaoSetSeparableObjectiveRoutine(my_tao, tao_residual_vec, mango_petsc_residual_function, user_context, ierr)
  case default
     print "(a)","Error! Should not get here."
  end select

  call TaoSetFromOptions(my_tao, ierr)

  call TaoSolve(my_tao, ierr)

  call TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF, ierr)

  call TaoDestroy(my_tao, ierr)
  call VecDestroy(tao_state_vec, ierr)
  call VecDestroy(tao_residual_vec, ierr)


contains

  subroutine mango_petsc_residual_function(my_tao, x, f, user_context, ierr)

    implicit none

    Tao :: my_tao
    Vec :: x, f
    PetscInt :: user_context
    PetscErrorCode :: ierr
    PetscScalar, pointer :: x_array(:), f_array(:)
    logical :: failed

    call VecGetArrayF90(x, x_array, ierr)
    call VecGetArrayF90(f, f_array, ierr)
    !print *,"Hello from mango_petsc_objective_function. x=",x_array

    call residual_function(x_array, f_array, failed)
    ! PETSc's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately:
    f_array = (f_array - problem%targets) / problem%sigmas
    if (failed) f_array = mango_huge

    call VecRestoreArrayF90(x, x_array, ierr)
    call VecRestoreArrayF90(f, f_array, ierr)
    
  end subroutine mango_petsc_residual_function

#else
  stop "Error! A PETSc algorithm was requested, but Mango was compiled without PETSc support"
#endif

end subroutine mango_optimize_least_squares_petsc

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

