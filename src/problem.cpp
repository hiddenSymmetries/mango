#include<iostream>
#include "mango.h"

/* Constructor for non-least-squares problems */
mango::problem::problem(int N_parameters_in, double* state_vector_in) {
  N_worker_groups = -1;
  algorithm = PETSC_NM;
  N_parameters = N_parameters_in;

  least_squares = false;
  /*  state_vector = new double[N_parameters];   */
  state_vector = state_vector_in;
  targets = NULL;
  sigmas = NULL;
}

/* Constructor for least-squares problems */
mango::problem::problem(int N_parameters_in, double* state_vector_in, int N_terms_in, double* targets_in, double* sigmas_in) {
  N_worker_groups = -1;
  algorithm = PETSC_POUNDERS;
  N_parameters = N_parameters_in;
  N_terms = N_terms_in;

  least_squares = true;
  /*
  state_vector = new double[N_parameters];
  targets = new double[N_terms];
  sigmas = new double[N_terms];
  */
  state_vector = state_vector_in;
  targets = targets_in;
  sigmas = sigmas_in;
}

/* Destructor */
mango::problem::~problem() {
  std::cout << "Mango problem is being destroyed.\n";
  /*
  delete state_vector;
  if (least_squares) {
    delete targets;
    delete sigmas;
  }
  */
}

