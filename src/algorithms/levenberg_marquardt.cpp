#include <iostream>
#include <Package_mango.hpp>
#include "levenberg_marquardt.hpp"

#ifdef MANGO_EIGEN_AVAILABLE
#include <Eigen/Dense>
#endif

//void mango::Package_mango::levenberg_marquardt(Least_squares_solver* solver) {
void mango::levenberg_marquardt::solve(Least_squares_solver* solver) {
#ifdef MANGO_EIGEN_AVAILABLE
  std::cout << "Hello from levenberg_marquardt" << std::endl;

  Eigen::MatrixXf A = Eigen::MatrixXf::Random(3, 2);
  std::cout << "Here is the matrix A:\n" << A << std::endl;
  Eigen::VectorXf b = Eigen::VectorXf::Random(3);
  std::cout << "Here is the right hand side b:\n" << b << std::endl;
  std::cout << "The least-squares solution is:\n"
	    << A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b) << std::endl;

  fun1();
#else
  throw std::runtime_error("ERROR: The algorithm mango_levenberg_marquardt was selected. This algorithm requires Eigen, but MANGO was built without Eigen.");
#endif
}

int mango::levenberg_marquardt::fun1() {
  std::cout << "Hello from levenberg_marquardt helper function" << std::endl;
  return 17;
}
