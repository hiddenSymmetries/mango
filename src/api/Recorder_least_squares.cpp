#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include "Recorder.hpp"
#include "Recorder_least_squares.hpp"

mango::Recorder_least_squares::Recorder_least_squares(Least_squares_solver* solver_in) {
  solver = solver_in;
}

void mango::Recorder_least_squares::init() {
  if (!solver->mpi_partition->get_proc0_world()) return; // Proceed only on proc0_world.

  int j;
  
  // Open output file
  output_file.open(solver->output_filename.c_str());
  if (!output_file.is_open()) {
    std::cerr << "output file: " << solver->output_filename << std::endl;
    throw std::runtime_error("Error! Unable to open output file.");
  }
  // Write header lines of output file
  output_file << "Recorder type:" << std::endl << "least_squares" << std::endl << "N_parameters:" << std::endl << solver->N_parameters << std::endl << "function_evaluation,seconds";
  for (j=0; j<solver->N_parameters; j++) {
    output_file << ",x(" << j+1 << ")";
  }
  output_file << ",objective_function";
  if (solver->print_residuals_in_output_file) {
    for (j=0; j<solver->N_terms; j++) {
      output_file << ",F(" << j+1 << ")";
    }
  }
  output_file << std::endl << std::flush;
}


void mango::Recorder_least_squares::write_file_line(int function_evaluations, clock_t print_time, const double* x, double f, double* residuals) {
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
  output_file << "," << std::setw(24) << f;
  if (solver->print_residuals_in_output_file) {
    for (int j=0; j<solver->N_terms; j++) {
      output_file << "," << std::setw(24) << std::setprecision(16) << std::scientific << residuals[j];
    }
  }
  output_file << std::endl << std::flush;
}


void mango::Recorder_least_squares::record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f) {
  if (!solver->mpi_partition->get_proc0_world()) return; // Proceed only on proc0_world.

  write_file_line(function_evaluations, print_time, x, f, solver->current_residuals);
}


void mango::Recorder_least_squares::finalize() {
  // Copy the line corresponding to the optimum to the bottom of the output file.

  if (!solver->mpi_partition->get_proc0_world()) return; // Proceed only on proc0_world.

  write_file_line(solver->best_function_evaluation, solver->best_time, solver->state_vector, solver->best_objective_function, solver->best_residual_function);

  output_file.close();
}
