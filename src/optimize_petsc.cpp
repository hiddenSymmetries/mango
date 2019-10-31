#include<iostream>
#include<stdlib.h>
#include "mango.hpp"
#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

/*
#ifdef MANGO_PETSC_AVAILABLE
PetscErrorCode mango_petsc_objective_function(Tao, Vec, PetscReal*, void*);
#endif
*/

static  char help[]="";

void mango::problem::optimize_petsc() {
#ifdef MANGO_PETSC_AVAILABLE

  /* The need for this line is described on https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Sys/PetscInitialize.html */
  PETSC_COMM_WORLD = MPI_COMM_SELF;

  int ierr;
  ierr = PetscInitialize(&argc,&argv,(char *)0,help);
  if (ierr) {
    std::cout << "Error in PetscInitialize.\n";
    exit(1);
  }
  ierr = PetscInitializeFortran();

  Tao my_tao;
  TaoCreate(PETSC_COMM_SELF, &my_tao);

  Vec tao_state_vec;
  VecCreateSeq(PETSC_COMM_SELF, N_parameters, &tao_state_vec);

  /* Set initial condition */
  double* temp_array;
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(temp_array, state_vector, N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);
  std::cout << "Here comes petsc vec for initial condition:\n";
  VecView(tao_state_vec, PETSC_VIEWER_STDOUT_SELF);
  TaoSetInitialVector(my_tao, tao_state_vec);

  std::cout << "PETSc has been initialized.\n";

  switch (algorithm) {
  case PETSC_NM:
    TaoSetType(my_tao, TAONM);
    TaoSetObjectiveRoutine(my_tao, &mango_petsc_objective_function, (void*)this);
    break;
  case PETSC_POUNDERS:
    std::cout << "Should not get here! For the petsc_pounders algorithm, mango_optimize_least_squares_petsc should be called instead of mango_optimize_petsc.\n";
    exit(1);
    break;
  default:
    std::cout << "Should not get here!\n";
    exit(1);
  }

  TaoSetMaximumFunctionEvaluations(my_tao, (PetscInt) max_function_and_gradient_evaluations);
  TaoSetFromOptions(my_tao);
  TaoSolve(my_tao);
  TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF);

  /* Copy PETSc solution to the mango state vector. */
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(state_vector, temp_array, N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);


  TaoDestroy(&my_tao);
  VecDestroy(&tao_state_vec);

  PetscFinalize();

#else
  std::cout << "Error! A PETSc algorithm was requested, but Mango was compiled without PETSc support.\n";
  exit(1);
#endif

}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef MANGO_PETSC_AVAILABLE
PetscErrorCode mango::problem::mango_petsc_objective_function(Tao my_tao, Vec x, PetscReal* f_petsc, void* user_context) {

  double* x_array;
  VecGetArray(x, &x_array);
  
  mango::problem* this_problem = (mango::problem*) user_context;

  bool failed;
  double f;
  this_problem->objective_function_wrapper(x_array, &f, &failed);
  /* objective_function_wrapper(x, &f, &failed); */

  if (failed) f = (PetscReal)mango::NUMBER_FOR_FAILED;

  VecRestoreArray(x, &x_array);

  *f_petsc = f;

  return(0);
}
#endif
