#ifndef MANGO_PACKAGE_H
#define MANGO_PACKAGE_H

namespace mango {

  class Problem_data;

  //////////////////////////////////////////////////////////////////////////////////////
  // Abstract class for optimization packages
  class Package {
  public:
    virtual void optimize(Problem_data*) = 0;
    virtual void optimize_least_squares(Problem_data*) = 0;
  };
}

#endif
