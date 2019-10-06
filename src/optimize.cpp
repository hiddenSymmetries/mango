#include<iostream>
#include "mango.h"

void mango::problem::optimize() {
  least_squares = false;

  if (! proc0_worker_groups) {
    std::cout << "Error! The mango_optimize() subroutine should only be called by group leaders, not by all workers.\n";
    exit(1);
  }
}
