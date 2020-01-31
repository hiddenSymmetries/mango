#ifndef MANGO_RECORDER_H
#define MANGO_RECORDER_H

#include <ctime>

namespace mango {

  // All methods of this parent class are empty. Therefore this base version of Recorder does nothing.
  class Recorder {
  public:
    virtual void init() {};
    virtual void record_function_evaluation(int function_evaluations, clock_t print_time, const double* x, double f) {};
    virtual void finalize() {};
  };

}

#endif
