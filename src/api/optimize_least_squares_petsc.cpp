// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cassert>
#include "mango.hpp"
#include "Least_squares_solver.hpp"
#include "Package_petsc.hpp"

#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

static  char help[]="";

void mango::Package_petsc::optimize_least_squares(Least_squares_solver* solver) {
#ifdef MANGO_PETSC_AVAILABLE

  int N_parameters = solver->N_parameters;
  int N_terms = solver->N_terms;

  // The need for this line is described on https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Sys/PetscInitialize.html
  PETSC_COMM_WORLD = MPI_COMM_SELF;

  int ierr;
  ierr = PetscInitialize(&(solver->argc),&(solver->argv),(char *)0,help);
  if (ierr) throw std::runtime_error("Error in PetscInitialize in mango::Package_petsc::optimize_least_squares().");
  ierr = PetscInitializeFortran();

  Tao my_tao;
  TaoCreate(PETSC_COMM_SELF, &my_tao);

  Vec tao_state_vec;
  VecCreateSeq(PETSC_COMM_SELF, N_parameters, &tao_state_vec);
  Vec tao_residual_vec;
  VecCreateSeq(PETSC_COMM_SELF, N_terms, &tao_residual_vec);
  Mat petsc_Jacobian;
  MatCreateSeqDense(MPI_COMM_SELF,N_terms,N_parameters,NULL,&petsc_Jacobian);

  // Set initial condition
  double* temp_array;
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(temp_array, solver->state_vector, N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);
  if (solver->verbose > 0) {
    std::cout << "Here comes petsc vec for initial condition:" << std::endl;
    VecView(tao_state_vec, PETSC_VIEWER_STDOUT_SELF);
  }
  TaoSetInitialVector(my_tao, tao_state_vec);

  if (solver->verbose > 0) std::cout << "PETSc has been initialized." << std::endl;

#if (PETSC_VERSION_MAJOR < 3 || (PETSC_VERSION_MAJOR==3 && PETSC_VERSION_MINOR < 11))
  TaoSetSeparableObjectiveRoutine(my_tao, tao_residual_vec, &mango_petsc_residual_function, (void*)solver);
#else
  TaoSetResidualRoutine(my_tao, tao_residual_vec, &mango_petsc_residual_function, (void*)solver);
#endif

  switch (solver->algorithm) {
  case PETSC_POUNDERS:
    TaoSetType(my_tao, TAOPOUNDERS);
    break;
  case PETSC_BRGN:
#if (PETSC_VERSION_MAJOR < 3 || (PETSC_VERSION_MAJOR==3 && PETSC_VERSION_MINOR < 11))
    throw std::runtime_error("The petsc_brgn algorithm requires PETSc version 3.11 or newer.");
#else
    TaoSetJacobianResidualRoutine(my_tao, petsc_Jacobian, petsc_Jacobian, &mango_petsc_Jacobian_function, (void*)solver);
    TaoSetType(my_tao, TAOBRGN);
    break;
#endif
  default:
    std::cerr << "Should not get here! algorithm = " << solver->algorithm << " i.e. " << algorithms[solver->algorithm].name << std::endl;
    throw std::runtime_error("Error in mango::problem::optimize_least_squares_petsc()");
  }

  TaoSetMaximumFunctionEvaluations(my_tao, (PetscInt) solver->max_function_and_gradient_evaluations);

  Vec lower_bounds_vec, upper_bounds_vec;
  if (solver->bound_constraints_set) {
    VecCreateSeq(PETSC_COMM_SELF, N_parameters, &lower_bounds_vec);
    VecCreateSeq(PETSC_COMM_SELF, N_parameters, &upper_bounds_vec);

    for (int j=0; j<N_parameters; j++) {
      VecSetValue(lower_bounds_vec, j, solver->lower_bounds[j], INSERT_VALUES);
      VecSetValue(upper_bounds_vec, j, solver->upper_bounds[j], INSERT_VALUES);
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
  if (solver->verbose > 0) TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF);
  //TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF);

  // Copy PETSc solution to the mango state vector.
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(solver->state_vector, temp_array, N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);


  TaoDestroy(&my_tao);
  VecDestroy(&tao_state_vec);

  PetscFinalize();

#else
  throw std::runtime_error("Error! A PETSc algorithm was requested, but Mango was compiled without PETSc support.");
#endif

}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

#ifdef MANGO_PETSC_AVAILABLE
PetscErrorCode mango::Package_petsc::mango_petsc_residual_function(Tao my_tao, Vec x, Vec f, void* user_context) {

  int j;
  const double* x_array;
  double* f_array;
  VecGetArrayRead(x, &x_array);
  VecGetArray(f, &f_array);
  
  Least_squares_solver* solver = (Least_squares_solver*) user_context;

  assert(solver->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  bool failed;
  solver->residual_function_wrapper(x_array, f_array, &failed);

  if (solver->verbose > 0) {
    std::cout << "mango_petsc_residual_function before sigma shift. state_vector:";
    for (j=0; j < solver->N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl;
    std::cout << "residual:";
    for (j=0; j < solver->N_terms; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << f_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  // PETSc's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately:
  for (j=0; j<solver->N_terms; j++) {
    f_array[j] = (f_array[j] - solver->targets[j]) / solver->sigmas[j];
    if (failed) f_array[j] = mango::NUMBER_FOR_FAILED;
  }

  if (solver->verbose > 0) {
    std::cout << "mango_petsc_residual_function after sigma shift. state_vector:";
    for (j=0; j < solver->N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl;
    std::cout << "residual:";
    for (j=0; j < solver->N_terms; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << f_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  VecRestoreArrayRead(x, &x_array);
  VecRestoreArray(f, &f_array);

  return(0);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

PetscErrorCode mango::Package_petsc::mango_petsc_Jacobian_function(Tao my_tao, Vec x, Mat Jacobian, Mat preconditioner, void* user_context) {
  int j;
  const double* x_array;
  double* Jacobian_array;
  VecGetArrayRead(x, &x_array);
  MatDenseGetArray(Jacobian, &Jacobian_array);
  
  Least_squares_solver* solver = (Least_squares_solver*) user_context;
  // Introduce shorthand:
  int N_parameters = solver->N_parameters;
  int N_terms = solver->N_terms;

  assert(solver->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  if (solver->verbose > 0) {
    std::cout << "mango_petsc_Jacobian_function before sigma shift. state_vector:";
    for (j=0; j < N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  solver->finite_difference_Jacobian(x_array, solver->residuals, Jacobian_array);
  // PETSc does not actually use the residuals computed here, only the Jacobian.


  // PETSc's definition of the residual function does not include sigmas or targets, so scale the Jacobian appropriately.
  // There is probably a faster approach than these explicit loops, but I'll worry about optimizing this later.
  for (int j_parameter = 0; j_parameter < N_parameters; j_parameter++) {
    for (int j_term = 0; j_term < N_terms; j_term++) {
      // MANGO's convention is Jacobian[j_parameter*N_terms+j_term].
      // PETSc's convention for the arrays underlying MatDense is that the Jacobian is in column major order, i.e. Fortran convention.
      // These are consistent.
      Jacobian_array[j_parameter*N_terms+j_term] /= solver->sigmas[j_term];
    }
  }

  VecRestoreArrayRead(x, &x_array);
  MatDenseRestoreArray(Jacobian, &Jacobian_array);

  return(0);
}

#endif
