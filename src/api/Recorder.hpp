// Copyright 2019, University of Maryland and the MANGO development team.
//
// This file is part of MANGO.
//
// MANGO is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// MANGO is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with MANGO.  If not, see
// <https://www.gnu.org/licenses/>.

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
