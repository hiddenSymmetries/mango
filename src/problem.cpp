#include<iostream>
#include "mango.h"

/* Constructor */
mango::problem::problem() {
  N_worker_groups = -1;
  algorithm = PETSC_NM;
}

/* Destructor */
mango::problem::~problem() {
  std::cout << "Mango problem is being destroyed.\n";
}

