#ifndef MANGO_PACKAGE_NLOPT_H
#define MANGO_PACKAGE_NLOPT_H

namespace mango {

  class Solver;
  class Least_squares_solver;

  class Package_nlopt : public Package {
  private:
    static double nlopt_objective_function(unsigned, const double*, double*, void*); 

  public:
    void optimize(Solver*);
    void optimize_least_squares(Least_squares_solver*);
  };
}

#endif
