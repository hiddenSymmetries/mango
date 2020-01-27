#ifndef MANGO_PACKAGE_NLOPT_H
#define MANGO_PACKAGE_NLOPT_H

namespace mango {

  class Problem_data;
  class Least_squares_data;

  class Package_nlopt : public Package {
  private:
    static double nlopt_objective_function(unsigned, const double*, double*, void*); 

  public:
    void optimize(Problem_data*);
    void optimize_least_squares(Problem_data*, Least_squares_data*);
  };
}

#endif
