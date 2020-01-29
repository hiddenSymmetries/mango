#ifndef MANGO_RECORDER_STANDARD_H
#define MANGO_RECORDER_STANDARD_H

#include <fstream>
#include "Problem_data.hpp"
#include "Recorder.hpp"

namespace mango {

class Recorder_standard : public Recorder {
private:
  Problem_data* problem_data;
  std::ofstream output_file;
  void write_file_line(int function_evaluations, clock_t print_time, const double* x, double f);

public:
  Recorder_standard(Problem_data*);
  void init();
  void record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f);
  void finalize();
};

}

#endif
