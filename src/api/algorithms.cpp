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
#include <string>
#include <stdexcept>
#include "mango.hpp"
#include "Solver.hpp"

void mango::Problem::set_algorithm(algorithm_type algorithm_in) {
  if (algorithm_in < 0) throw std::runtime_error("Error in mango::Problem::set_algorithm. Algorithm cannot be negative.");
  if (algorithm_in >= NUM_ALGORITHMS) throw std::runtime_error("Error in mango::Problem::set_algorithm. Algorithm is too large.");

  solver->algorithm = algorithm_in;

  if (solver->verbose > 0) std::cout << "Algorithm set (by integer) to " << algorithm_in << ", a.k.a. " << algorithms[algorithm_in].name << std::endl;
}

///////////////////////////////////////////////////////////////////////////

void mango::Problem::set_algorithm(std::string str) {
  algorithm_type algorithm;
  bool found_match = get_algorithm(str, &algorithm);
  if (!found_match) {
    std::cerr << "Error in mango::Problem::set_algorithm. The following algorithm name was requested but not found: " << str << std::endl;
    throw std::runtime_error("Error in mango::Problem::set_algorithm: The requested algorithm name was not found.");
  }
  solver->algorithm = algorithm;
  if (solver->verbose > 0) std::cout << "Algorithm set (by string) to " << algorithm << ", a.k.a. " << algorithms[algorithm].name << std::endl;
}

///////////////////////////////////////////////////////////////////////////

bool mango::get_algorithm(std::string str, algorithm_type* code) {
  // The return value of this function indicates whether an algorithm with the requested name str was found.
  // If a match is found, the corresponding index into the algorithms[] database is returned in the parameter code.

  bool found_match = false;
  int j;
  for (j = 0; j < NUM_ALGORITHMS; j++) {
    if (algorithms[j].name.compare(str) == 0) {
      found_match = true;
      break;
    }
  }

  if (found_match) {
    *code = (algorithm_type) j;
    return(true);
  } else {
    return(false);
  }
}

///////////////////////////////////////////////////////////////////////////

bool mango::does_algorithm_exist(std::string str) {
  algorithm_type algorithm_code;
  return(get_algorithm(str,&algorithm_code));
}
