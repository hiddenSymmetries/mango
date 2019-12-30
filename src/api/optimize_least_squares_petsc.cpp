#include<iostream>
#include<iomanip>
#include<stdexcept>
#include<cassert>
#include "mango.hpp"
#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

static  char help[]="";

void mango::problem::optimize_least_squares_petsc() {
#ifdef MANGO_PETSC_AVAILABLE

  // The need for this line is described on https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Sys/PetscInitialize.html
  PETSC_COMM_WORLD = MPI_COMM_SELF;

  int ierr;
  ierr = PetscInitialize(&argc,&argv,(char *)0,help);
  if (ierr) throw std::runtime_error("Error in PetscInitialize in mango::problem::optimize_least_squares_petsc().");
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
  memcpy(temp_array, state_vector, N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);
  if (verbose > 0) {
    std::cout << "Here comes petsc vec for initial condition:" << std::endl;
    VecView(tao_state_vec, PETSC_VIEWER_STDOUT_SELF);
  }
  TaoSetInitialVector(my_tao, tao_state_vec);

  if (verbose > 0) std::cout << "PETSc has been initialized." << std::endl;

#if (PETSC_VERSION_MAJOR < 3 || (PETSC_VERSION_MAJOR==3 && PETSC_VERSION_MINOR < 10))
  TaoSetSeparableObjectiveRoutine(my_tao, tao_residual_vec, &mango_petsc_residual_function, (void*)this);
#else
  TaoSetResidualRoutine(my_tao, tao_residual_vec, &mango_petsc_residual_function, (void*)this);
#endif

  switch (algorithm) {
  case PETSC_POUNDERS:
    TaoSetType(my_tao, TAOPOUNDERS);
    break;
  case PETSC_BRGN:
#if (PETSC_VERSION_MAJOR < 3 || (PETSC_VERSION_MAJOR==3 && PETSC_VERSION_MINOR < 11))
    throw std::runtime_error("The petsc_brgn algorithm requires PETSc version 3.11 or newer.");
#else
    TaoSetJacobianResidualRoutine(my_tao, petsc_Jacobian, petsc_Jacobian, &mango_petsc_Jacobian_function, (void*)this);
    TaoSetType(my_tao, TAOBRGN);
#endif
  default:
    std::cerr << "Should not get here! algorithm = " << algorithm << " i.e. " << algorithms[algorithm].name << std::endl;
    throw std::runtime_error("Error in mango::problem::optimize_least_squares_petsc()");
  }

  // TaoSetTolerances(my_tao, 1e-30, 1e-30, 1e-30);
  TaoSetFromOptions(my_tao);
  TaoSolve(my_tao);
  if (verbose > 0) TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF);
  //TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF);

  // Copy PETSc solution to the mango state vector.
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(state_vector, temp_array, N_parameters * sizeof(double));
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
PetscErrorCode mango::problem::mango_petsc_residual_function(Tao my_tao, Vec x, Vec f, void* user_context) {

  int j;
  double* x_array;
  double* f_array;
  VecGetArray(x, &x_array);
  VecGetArray(f, &f_array);
  
  mango::problem* this_problem = (mango::problem*) user_context;
  assert(this_problem->mpi_partition.get_proc0_world()); // This subroutine should only ever be called by proc 0.

  bool failed;
  this_problem->residual_function_wrapper(x_array, f_array, &failed);

  if (this_problem->verbose > 0) {
    std::cout << "mango_petsc_residual_function before sigma shift. state_vector:";
    for (j=0; j < this_problem->get_N_parameters(); j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl;
    std::cout << "residual:";
    for (j=0; j < this_problem->get_N_terms(); j++) {
      std::cout << std::setw(24) << std::setprecision(15) << f_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  // PETSc's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately:
  for (j=0; j<this_problem->get_N_terms(); j++) {
    f_array[j] = (f_array[j] - this_problem->targets[j]) / this_problem->sigmas[j];
    if (failed) f_array[j] = mango::NUMBER_FOR_FAILED;
  }

  if (this_problem->verbose > 0) {
    std::cout << "mango_petsc_residual_function after sigma shift. state_vector:";
    for (j=0; j < this_problem->get_N_parameters(); j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl;
    std::cout << "residual:";
    for (j=0; j < this_problem->get_N_terms(); j++) {
      std::cout << std::setw(24) << std::setprecision(15) << f_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  VecRestoreArray(x, &x_array);
  VecRestoreArray(f, &f_array);

  return(0);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

PetscErrorCode mango::problem::mango_petsc_Jacobian_function(Tao my_tao, Vec x, Mat Jacobian, Mat preconditioner, void* user_context) {
  int j;
  double* x_array;
  double* Jacobian_array;
  VecGetArray(x, &x_array);
  MatDenseGetArray(Jacobian, &Jacobian_array);
  
  mango::problem* this_problem = (mango::problem*) user_context;

  assert(this_problem->mpi_partition.get_proc0_world()); // This subroutine should only ever be called by proc 0.

  if (this_problem->verbose > 0) {
    std::cout << "mango_petsc_Jacobian_function before sigma shift. state_vector:";
    for (j=0; j < this_problem->get_N_parameters(); j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  this_problem->finite_difference_Jacobian(x_array, this_problem->residuals, Jacobian_array);
  // PETSc does not actually use the residuals computed here, only the Jacobian.

  // Introduce shorthand:
  int N_parameters = this_problem->N_parameters;
  int N_terms = this_problem->N_terms;

  // PETSc's definition of the residual function does not include sigmas or targets, so scale the Jacobian appropriately.
  // There is probably a faster approach than these explicit loops, but I'll worry about optimizing this later.
  for (int j_parameter = 0; j_parameter < N_parameters; j_parameter++) {
    for (int j_term = 0; j_term < N_terms; j_term++) {
      // MANGO's convention is Jacobian[j_parameter*N_terms+j_term].
      // PETSc's convention for the arrays underlying MatDense is that the Jacobian is in column major order, i.e. Fortran convention.
      // These are consistent.
      Jacobian_array[j_parameter*N_terms+j_term] /= this_problem->sigmas[j_term];
    }
  }

  VecRestoreArray(x, &x_array);
  MatDenseRestoreArray(Jacobian, &Jacobian_array);

  return(0);
}

#endif
