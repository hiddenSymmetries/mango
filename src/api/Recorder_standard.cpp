#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include "Recorder.hpp"
#include "Recorder_standard.hpp"

mango::Recorder_standard::Recorder_standard(Solver* solver_in) {
  solver = solver_in;
}

void mango::Recorder_standard::init() {
  if (!solver->mpi_partition->get_proc0_world()) return; // Proceed only on proc0_world.

  // Open output file
  output_file.open(solver->output_filename.c_str());
  if (!output_file.is_open()) {
    std::cerr << "output file: " << solver->output_filename << std::endl;
    throw std::runtime_error("Error! Unable to open output file.");
  }
  // Write header lines of output file
  output_file << "Recorder type:" << std::endl << "standard" << std::endl << "N_parameters:" << std::endl << solver->N_parameters << std::endl << "function_evaluation,seconds";
  for (int j=0; j<solver->N_parameters; j++) {
    output_file << ",x(" << j+1 << ")";
  }
  output_file << ",objective_function" << std::endl;  
}


void mango::Recorder_standard::write_file_line(int function_evaluations, clock_t print_time, const double* x, double f) {
  // This subroutine writes a line in the output file for non-least-squares problems.

  /*
  write_function_evaluation_and_time(print_time);
  std::string file_string;
  compose_x_f_string(file_string, x, f);
  output_file << file_string << std::endl << std::flush;
  */
  double elapsed_time = ((float)(print_time - solver->start_time)) / CLOCKS_PER_SEC;
  output_file << std::setw(6) << std::right << function_evaluations << "," << std::setw(12) << std::setprecision(4) << std::scientific << elapsed_time;
  for (int j=0; j<solver->N_parameters; j++) {
    output_file << "," << std::setw(24) << std::setprecision(16) << std::scientific << x[j];
  }
  output_file << "," << std::setw(24) << f << std::endl << std::flush;
}


void mango::Recorder_standard::record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f) {
  if (!solver->mpi_partition->get_proc0_world()) return; // Proceed only on proc0_world.
  write_file_line(function_evaluations, print_time, x, f);
}


void mango::Recorder_standard::finalize() {
  // Copy the line corresponding to the optimum to the bottom of the output file.

  if (!solver->mpi_partition->get_proc0_world()) return; // Proceed only on proc0_world.

  write_file_line(solver->best_function_evaluation, solver->best_time, solver->state_vector, solver->best_objective_function);

  output_file.close();
}
