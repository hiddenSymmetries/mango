#ifndef MANGO_RECORDER_H
#define MANGO_RECORDER_H

#include <ctime>

namespace mango {

class Recorder {
public:
  virtual void init() = 0;
  virtual void record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f) = 0;
  virtual void finalize() = 0;
};

}

#endif
