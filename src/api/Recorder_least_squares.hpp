#ifndef MANGO_RECORDER_LEAST_SQUARES_H
#define MANGO_RECORDER_LEAST_SQUARES_H

#include <fstream>
#include "Problem_data.hpp"
#include "Least_squares_data.hpp"
#include "Recorder.hpp"

namespace mango {

class Recorder_least_squares : public Recorder {
private:
  Problem_data* problem_data;
  Least_squares_data* least_squares_data;
  std::ofstream output_file;
  void write_file_line(int function_evaluations, clock_t print_time, const double* x, double f, double* residuals);

public:
  Recorder_least_squares(Problem_data*, Least_squares_data*);
  void init();
  void record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f);
  void finalize();
};

}

#endif
