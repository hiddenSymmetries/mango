#ifndef MANGO_PACKAGE_H
#define MANGO_PACKAGE_H

namespace mango {

  class Solver;
  class Least_squares_solver;

  //////////////////////////////////////////////////////////////////////////////////////
  // Abstract class for optimization packages
  class Package {
  public:
    virtual void optimize(Solver*) = 0;
    virtual void optimize_least_squares(Least_squares_solver*) = 0;
  };
}

#endif
