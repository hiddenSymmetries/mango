#include<iostream>
#include<iomanip>
#include "mango.hpp"
#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

#ifdef MANGO_PETSC_AVAILABLE
PetscErrorCode mango_petsc_residual_function(Tao, Vec, Vec, void*);
#endif

static  char help[]="";

void mango::problem::optimize_least_squares_petsc() {
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
  Vec tao_residual_vec;
  VecCreateSeq(PETSC_COMM_SELF, N_terms, &tao_residual_vec);

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
  case PETSC_POUNDERS:
    TaoSetType(my_tao, TAOPOUNDERS);
    TaoSetSeparableObjectiveRoutine(my_tao, tao_residual_vec, &mango_petsc_residual_function, (void*)this);
    break;
  default:
    std::cout << "Should not get here! algorithm = " << algorithm << " i.e. " << algorithm_name << "\n";
    exit(1);
  }

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
PetscErrorCode mango_petsc_residual_function(Tao my_tao, Vec x, Vec f, void* user_context) {

  int j;
  double* x_array;
  double* f_array;
  VecGetArray(x, &x_array);
  VecGetArray(f, &f_array);
  
  mango::problem* this_problem = (mango::problem*) user_context;

  bool failed;
  this_problem->residual_function_wrapper(x_array, f_array, &failed);

  std::cout << "mango_petsc_residual_function before sigma shift. state_vector:";
  for (j=0; j < this_problem->get_N_parameters(); j++) {
    std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
  }
  std::cout << "\n";
  std::cout << "residual:";
  for (j=0; j < this_problem->get_N_terms(); j++) {
    std::cout << std::setw(24) << std::setprecision(15) << f_array[j];
  }
  std::cout << "\n" << std::flush;

  /* PETSc's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately: */
  for (j=0; j<this_problem->get_N_terms(); j++) {
    f_array[j] = (f_array[j] - this_problem->targets[j]) / this_problem->sigmas[j];
    if (failed) f_array[j] = mango::NUMBER_FOR_FAILED;
  }

  std::cout << "mango_petsc_residual_function after sigma shift. state_vector:";
  for (j=0; j < this_problem->get_N_parameters(); j++) {
    std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
  }
  std::cout << "\n";
  std::cout << "residual:";
  for (j=0; j < this_problem->get_N_terms(); j++) {
    std::cout << std::setw(24) << std::setprecision(15) << f_array[j];
  }
  std::cout << "\n" << std::flush;

  VecRestoreArray(x, &x_array);
  VecRestoreArray(f, &f_array);

  return(0);
}
#endif
