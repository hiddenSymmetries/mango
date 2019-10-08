#include<string>
#include<fstream>
#include<iostream>
#include "mango.hpp"

void mango::problem::read_input_file(std::string filename) {
  std::ifstream file;
  std::string algorithm_str;

  file.open(filename);
  if (!file.is_open()) {
    std::cout << "Error! Unable to open file " << filename << "\n";
    exit(1);
  }

  file >> N_worker_groups;
  file >> algorithm_str;
  set_algorithm(algorithm_str);
  file.close();
}
