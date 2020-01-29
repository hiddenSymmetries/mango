#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cassert>
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Least_squares_data.hpp"
#include "Package_petsc.hpp"

#ifdef MANGO_PETSC_AVAILABLE
#include <petsctao.h>
#endif

static  char help[]="";

void mango::Package_petsc::optimize_least_squares(Problem_data* problem_data, Least_squares_data* least_squares_data) {
#ifdef MANGO_PETSC_AVAILABLE

  int N_parameters = problem_data->N_parameters;
  int N_terms = least_squares_data->N_terms;

  // The need for this line is described on https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/Sys/PetscInitialize.html
  PETSC_COMM_WORLD = MPI_COMM_SELF;

  int ierr;
  ierr = PetscInitialize(&(problem_data->argc),&(problem_data->argv),(char *)0,help);
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
  memcpy(temp_array, problem_data->state_vector, N_parameters * sizeof(double));
  VecRestoreArray(tao_state_vec, &temp_array);
  if (problem_data->verbose > 0) {
    std::cout << "Here comes petsc vec for initial condition:" << std::endl;
    VecView(tao_state_vec, PETSC_VIEWER_STDOUT_SELF);
  }
  TaoSetInitialVector(my_tao, tao_state_vec);

  if (problem_data->verbose > 0) std::cout << "PETSc has been initialized." << std::endl;

#if (PETSC_VERSION_MAJOR < 3 || (PETSC_VERSION_MAJOR==3 && PETSC_VERSION_MINOR < 10))
  TaoSetSeparableObjectiveRoutine(my_tao, tao_residual_vec, &mango_petsc_residual_function, (void*)least_squares_data);
#else
  TaoSetResidualRoutine(my_tao, tao_residual_vec, &mango_petsc_residual_function, (void*)least_squares_data);
#endif

  switch (problem_data->algorithm) {
  case PETSC_POUNDERS:
    TaoSetType(my_tao, TAOPOUNDERS);
    break;
  case PETSC_BRGN:
#if (PETSC_VERSION_MAJOR < 3 || (PETSC_VERSION_MAJOR==3 && PETSC_VERSION_MINOR < 11))
    throw std::runtime_error("The petsc_brgn algorithm requires PETSc version 3.11 or newer.");
#else
    TaoSetJacobianResidualRoutine(my_tao, petsc_Jacobian, petsc_Jacobian, &mango_petsc_Jacobian_function, (void*)least_squares_data);
    TaoSetType(my_tao, TAOBRGN);
    break;
#endif
  default:
    std::cerr << "Should not get here! algorithm = " << problem_data->algorithm << " i.e. " << algorithms[problem_data->algorithm].name << std::endl;
    throw std::runtime_error("Error in mango::problem::optimize_least_squares_petsc()");
  }

  TaoSetMaximumFunctionEvaluations(my_tao, (PetscInt) problem_data->max_function_and_gradient_evaluations);

  Vec lower_bounds_vec, upper_bounds_vec;
  if (problem_data->bound_constraints_set) {
    VecCreateSeq(PETSC_COMM_SELF, N_parameters, &lower_bounds_vec);
    VecCreateSeq(PETSC_COMM_SELF, N_parameters, &upper_bounds_vec);

    for (int j=0; j<N_parameters; j++) {
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
  //TaoView(my_tao, PETSC_VIEWER_STDOUT_SELF);

  // Copy PETSc solution to the mango state vector.
  VecGetArray(tao_state_vec, &temp_array);
  memcpy(problem_data->state_vector, temp_array, N_parameters * sizeof(double));
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
  
  Least_squares_data* least_squares_data = (Least_squares_data*) user_context;
  Problem_data* problem_data = least_squares_data->problem_data;

  assert(problem_data->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  bool failed;
  least_squares_data->residual_function_wrapper(x_array, f_array, &failed);

  if (problem_data->verbose > 0) {
    std::cout << "mango_petsc_residual_function before sigma shift. state_vector:";
    for (j=0; j < problem_data->N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl;
    std::cout << "residual:";
    for (j=0; j < least_squares_data->N_terms; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << f_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  // PETSc's definition of the residual function does not include sigmas or targets, so shift and scale the mango residuals appropriately:
  for (j=0; j<least_squares_data->N_terms; j++) {
    f_array[j] = (f_array[j] - least_squares_data->targets[j]) / least_squares_data->sigmas[j];
    if (failed) f_array[j] = mango::NUMBER_FOR_FAILED;
  }

  if (problem_data->verbose > 0) {
    std::cout << "mango_petsc_residual_function after sigma shift. state_vector:";
    for (j=0; j < problem_data->N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl;
    std::cout << "residual:";
    for (j=0; j < least_squares_data->N_terms; j++) {
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
  
  Least_squares_data* least_squares_data = (Least_squares_data*) user_context;
  Problem_data* problem_data = least_squares_data->problem_data;
  // Introduce shorthand:
  int N_parameters = problem_data->N_parameters;
  int N_terms = least_squares_data->N_terms;

  assert(problem_data->mpi_partition->get_proc0_world()); // This subroutine should only ever be called by proc 0.

  if (problem_data->verbose > 0) {
    std::cout << "mango_petsc_Jacobian_function before sigma shift. state_vector:";
    for (j=0; j < N_parameters; j++) {
      std::cout << std::setw(24) << std::setprecision(15) << x_array[j];
    }
    std::cout << std::endl << std::flush;
  }

  least_squares_data->finite_difference_Jacobian(x_array, least_squares_data->residuals, Jacobian_array);
  // PETSc does not actually use the residuals computed here, only the Jacobian.


  // PETSc's definition of the residual function does not include sigmas or targets, so scale the Jacobian appropriately.
  // There is probably a faster approach than these explicit loops, but I'll worry about optimizing this later.
  for (int j_parameter = 0; j_parameter < N_parameters; j_parameter++) {
    for (int j_term = 0; j_term < N_terms; j_term++) {
      // MANGO's convention is Jacobian[j_parameter*N_terms+j_term].
      // PETSc's convention for the arrays underlying MatDense is that the Jacobian is in column major order, i.e. Fortran convention.
      // These are consistent.
      Jacobian_array[j_parameter*N_terms+j_term] /= least_squares_data->sigmas[j_term];
    }
  }

  VecRestoreArrayRead(x, &x_array);
  MatDenseRestoreArray(Jacobian, &Jacobian_array);

  return(0);
}

#endif
