#include <iostream>
#include <stdexcept>
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Package_petsc.hpp"
#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

static  char help[]="";

void mango::Package_petsc::optimize(Problem_data* problem_data) {
#ifdef MANGO_PETSC_AVAILABLE

  // The need for this line is described on https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Sys/PetscInitialize.html
  PETSC_COMM_WORLD = MPI_COMM_SELF;

  int ierr;
  ierr = PetscInitialize(&(problem_data->argc),&(problem_data->argv),(char *)0,help);
  if (ierr) throw std::runtime_error("Error in PetscInitialize in mango::Package_petsc::optimize().");
  ierr = PetscInitializeFortran();

  Tao my_tao;
  TaoCreate(PETSC_COMM_SELF, &my_tao);

  Vec tao_state_vec;
  VecCreateSeq(PETSC_COMM_SELF, problem_data->N_parameters, &tao_state_vec);

  // Set initial condition
  double* temp_array;
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(temp_array, problem_data->state_vector, problem_data->N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);
  if (problem_data->verbose > 0) {
    std::cout << "Here comes petsc vec for initial condition:" << std::endl;
    VecView(tao_state_vec, PETSC_VIEWER_STDOUT_SELF);
  }
  TaoSetInitialVector(my_tao, tao_state_vec);

  if (problem_data->verbose > 0) std::cout << "PETSc has been initialized." << std::endl;

  switch (problem_data->algorithm) {
  case PETSC_NM:
    TaoSetType(my_tao, TAONM);
    TaoSetObjectiveRoutine(my_tao, &mango_petsc_objective_function, (void*)problem_data);
    break;
  case PETSC_POUNDERS:
    throw std::runtime_error("Should not get here! For the petsc_pounders algorithm, mango_optimize_least_squares_petsc should be called instead of mango_optimize_petsc.");
    break;
  default:
    throw std::runtime_error("Should not get here!");
  }

  TaoSetMaximumFunctionEvaluations(my_tao, (PetscInt) problem_data->max_function_and_gradient_evaluations);

  Vec lower_bounds_vec, upper_bounds_vec;
  if (problem_data->bound_constraints_set) {
    VecCreateSeq(PETSC_COMM_SELF, problem_data->N_parameters, &lower_bounds_vec);
    VecCreateSeq(PETSC_COMM_SELF, problem_data->N_parameters, &upper_bounds_vec);

    for (int j=0; j<problem_data->N_parameters; j++) {
      VecSetValue(lower_bounds_vec, j, problem_data->lower_bounds[j], INSERT_VALUES);
      VecSetValue(upper_bounds_vec, j, problem_data->upper_bounds[j], INSERT_VALUES);
    }
    VecAssemblyBegin(lower_bounds_vec);
    VecAssemblyBegin(upper_bounds_vec);
    VecAssemblyEnd(lower_bounds_vec);
    VecAssemblyEnd(upper_bounds_vec);

    TaoSetVariableBounds(my_tao, lower_bounds_vec, upper_bounds_vec);
  }

  // TaoSetTolerances(my_tao, 1e-30, 1e-30, 1e-30);
  TaoSetFromOptions(my_tao);
  TaoSolve(my_tao);
  if (problem_data->verbose > 0) TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF);

  // Copy PETSc solution to the mango state vector.
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(problem_data->state_vector, temp_array, problem_data->N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);

  if (problem_data->bound_constraints_set) {
    VecDestroy(&lower_bounds_vec);
    VecDestroy(&upper_bounds_vec);
  }

  TaoDestroy(&my_tao);
  VecDestroy(&tao_state_vec);

  PetscFinalize();

#else
  throw std::runtime_error("Error! A PETSc algorithm was requested, but Mango was compiled without PETSc support.");
#endif

}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#ifdef MANGO_PETSC_AVAILABLE
PetscErrorCode mango::Package_petsc::mango_petsc_objective_function(Tao my_tao, Vec x, PetscReal* f_petsc, void* user_context) {

  const double* x_array;
  VecGetArrayRead(x, &x_array);
  
  mango::Problem_data* problem_data = (mango::Problem_data*) user_context;

  bool failed;
  double f;
  problem_data->objective_function_wrapper(x_array, &f, &failed);

  if (failed) f = (PetscReal)mango::NUMBER_FOR_FAILED;

  VecRestoreArrayRead(x, &x_array);

  *f_petsc = f;

  return(0);
}
#endif
