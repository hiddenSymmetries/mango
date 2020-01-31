#ifndef MANGO_PACKAGE_HOPSPACK_H
#define MANGO_PACKAGE_HOPSPACK_H

#ifdef MANGO_HOPSPACK_AVAILABLE
#endif

namespace mango {

  class Solver;
  class Least_squares_solver;

  class Package_hopspack : public Package {
  private:
    Solver* solver;

#ifdef MANGO_HOPSPACK_AVAILABLE
#endif

  public:
    void optimize(Solver*);
    void optimize_least_squares(Least_squares_solver*);
  };
}

#endif
