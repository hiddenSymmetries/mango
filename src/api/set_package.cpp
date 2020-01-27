#include <stdexcept>
#include "mango.hpp"
#include "Problem_data.hpp"
#include "Package_nlopt.hpp"
#include "Package_gsl.hpp"
#include "Package_petsc.hpp"

// This file is the only place where the core of mango (specifically Problem_data) knows
// anything about the concrete Packages available.

void mango::Problem_data::set_package() {
  switch (algorithms[algorithm].package) {
  case PACKAGE_NLOPT:
    package = new Package_nlopt();
    break;
  case PACKAGE_GSL:
    package = new Package_gsl();
    break;
  case PACKAGE_PETSC:
    package = new Package_petsc();
    break;
  default:
    throw std::runtime_error("Unknown package in mango::Problem_data::set_package");
  }
}
