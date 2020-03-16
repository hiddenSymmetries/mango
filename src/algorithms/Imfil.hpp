#include "Package_mango.hpp"
#include "Solver.hpp"

namespace mango {
  class Imfil : public Algorithm {

  public:
    Imfil(Solver*);
    void solve();
  };
}

