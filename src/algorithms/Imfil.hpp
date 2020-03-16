#ifndef MANGO_IMFIL_H
#define MANGO_IMFIL_H

#include "Package_mango.hpp"
#include "Solver.hpp"

namespace mango {
  class Imfil : public Algorithm {

  public:
    Imfil(Solver*);
    void solve();
  };
}

#endif
