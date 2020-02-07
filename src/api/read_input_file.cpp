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

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "mango.hpp"

void mango::Problem::read_input_file(std::string filename) {
  std::ifstream file;

  file.open(filename.c_str());
  if (!file.is_open()) {
    std::cerr << "Error! Unable to open file " << filename << std::endl;
    throw std::runtime_error("Error in mango::Problem::read_input_file. Unable to open file.");
  }

  int N_worker_groups;
  std::string algorithm_str;
  file >> N_worker_groups;
  file >> algorithm_str;
  set_algorithm(algorithm_str);
  file.close();

  mpi_partition.set_N_worker_groups(N_worker_groups);
}
