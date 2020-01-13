#include <stdexcept>
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Package_petsc.hpp"
#include "Package_nlopt.hpp"

// This file is the only place where the core of mango (specifically Problem_data) knows
// anything about the concrete Packages available.

void mango::Problem_data::set_package() {
  switch (algorithm.package) {
  case PACKAGE_NLOPT:
    package = new Package_nlopt();
    break;
  case PACKAGE_PETSC:
    package = new Package_petsc();
    break;
  default:
    throw std::runtime_error("Unknown package in mango::Problem_data::set_package");
  }
}
