#ifndef MANGO_RECORDER_LEAST_SQUARES_H
#define MANGO_RECORDER_LEAST_SQUARES_H

#include <fstream>
#include "Least_squares_solver.hpp"
#include "Recorder.hpp"

namespace mango {

class Recorder_least_squares : public Recorder {
private:
  Least_squares_solver* solver;
  std::ofstream output_file;
  void write_file_line(int function_evaluations, clock_t print_time, const double* x, double f, double* residuals);

public:
  Recorder_least_squares(Least_squares_solver*);
  void init();
  void record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f);
  void finalize();
};

}

#endif
