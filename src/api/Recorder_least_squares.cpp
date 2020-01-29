#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <ctime>
#include "Recorder.hpp"
#include "Recorder_least_squares.hpp"
#include "Problem_data.hpp"
#include "Least_squares_data.hpp"

mango::Recorder_least_squares::Recorder_least_squares(Problem_data* problem_data_in, Least_squares_data* least_squares_data_in) {
  problem_data = problem_data_in;
  least_squares_data = least_squares_data_in;
}

void mango::Recorder_least_squares::init() {
  int j;
  
  // Open output file
  output_file.open(problem_data->output_filename.c_str());
  if (!output_file.is_open()) {
    std::cerr << "output file: " << problem_data->output_filename << std::endl;
    throw std::runtime_error("Error! Unable to open output file.");
  }
  // Write header lines of output file
  output_file << "Recorder type:" << std::endl << "least_squares" << std::endl << "N_parameters:" << std::endl << problem_data->N_parameters << std::endl << "function_evaluation,seconds";
  for (j=0; j<problem_data->N_parameters; j++) {
    output_file << ",x(" << j+1 << ")";
  }
  output_file << ",objective_function";
  if (least_squares_data->print_residuals_in_output_file) {
    for (j=0; j<least_squares_data->N_terms; j++) {
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
  double elapsed_time = ((float)(print_time - problem_data->start_time)) / CLOCKS_PER_SEC;
  output_file << std::setw(6) << std::right << function_evaluations << "," << std::setw(12) << std::setprecision(4) << std::scientific << elapsed_time;
  for (int j=0; j<problem_data->N_parameters; j++) {
    output_file << "," << std::setw(24) << std::setprecision(16) << std::scientific << x[j];
  }
  output_file << "," << std::setw(24) << f;
  if (least_squares_data->print_residuals_in_output_file) {
    for (int j=0; j<least_squares_data->N_terms; j++) {
      output_file << "," << std::setw(24) << std::setprecision(16) << std::scientific << residuals[j];
    }
  }
  output_file << std::endl << std::flush;
}


void mango::Recorder_least_squares::record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f) {
  write_file_line(function_evaluations, print_time, x, f, least_squares_data->current_residuals);
}


void mango::Recorder_least_squares::finalize() {
  // Copy the line corresponding to the optimum to the bottom of the output file.
  write_file_line(problem_data->best_function_evaluation, problem_data->best_time, problem_data->state_vector, problem_data->best_objective_function, least_squares_data->best_residual_function);

  output_file.close();
}
