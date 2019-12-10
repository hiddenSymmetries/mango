#include<iostream>
#include<iomanip>
#include<sstream>
#include<cstring>
#include<ctime>
#include "mango.hpp"


void mango::problem::write_function_evaluations() {
  // This subroutine formats and writes only the first entry in the output file lines: the # of function evaluations.
  output_file << std::setw(6) << std::right << function_evaluations << ",";
}


void mango::problem::compose_time_x_f_string(std::string & file_string, clock_t print_time, const double* x, double f) {
  // This subroutine formats (but does not write) several of the entries in the output file lines: the elapsed time, the state vector x, and the objective function f.
  double elapsed_time = ((float)(print_time - start_time)) / CLOCKS_PER_SEC;

  std::ostringstream string_stream;
  string_stream << std::right << std::setw(12) << std::setprecision(4) << std::scientific << elapsed_time;
  for (int j=0; j<N_parameters; j++) {
    string_stream << "," << std::setw(24) << std::setprecision(16) << std::scientific << x[j];
  }
  string_stream << "," << std::setw(24) << f;
  // Finally, convert the stream to a std::string:
  file_string = string_stream.str();
}


void mango::problem::compose_residuals_string(std::string & file_string, double* residuals) {
  // This subroutine formats (but does not write) the entries in the output file lines for the residuals in a least-squares problem.

  std::ostringstream string_stream;
  for (int j=0; j<N_terms; j++) {
    string_stream << "," << std::setw(24) << std::setprecision(16) << std::scientific << residuals[j];
  }

  // Finally, convert the stream to a std::string:
  file_string = string_stream.str();
}


void mango::problem::write_file_line(clock_t print_time, const double* x, double f) {
  // This subroutine writes a line in the output file for non-least-squares problems.
  write_function_evaluations();
  std::string file_string;
  compose_time_x_f_string(file_string, print_time, x, f);
  output_file << file_string << std::endl << std::flush;
}


void mango::problem::write_least_squares_file_line(clock_t print_time, const double* x, double f, double* residuals) {
  // This subroutine assumes the total objective function f has been computed from the residuals using mango::problem::residuals_to_single_objective.

  // Prepare the parts of the output file line:
  std::string time_x_f_string, residuals_string;
  compose_time_x_f_string(time_x_f_string, print_time, x, f);
  compose_residuals_string(residuals_string, residuals);

  // Now actually write the line of the output file.
  write_function_evaluations();
  output_file << time_x_f_string << residuals_string << std::endl << std::flush;
}


void mango::problem::write_hopspack_line_to_file(std::string line) {
  // 'line' is almost all of the line of the output file, except that the global # of function evaluations (the first element of the line)
  // is missing. This is because the line was generated on a proc >0, but only proc 0 knows the global # of function evaluations.

  function_evaluations += 1; // This line is how proc 0 keeps track of the total number of function evaluations.
  write_function_evaluations();
  output_file << line << std::endl << std::flush;
}
