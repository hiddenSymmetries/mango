#include<iostream>
#include "mango.hpp"

#ifdef MANGO_NLOPT_AVAILABLE
#include "nlopt.hpp"
#endif

/* double mango::problem::nlopt_objective_function(unsigned, const double*, double*, void*); */
double nlopt_objective_function(unsigned, const double*, double*, void*); 


void mango::problem::optimize_nlopt() {
#ifdef MANGO_NLOPT_AVAILABLE

  /*  int* f_data = NULL;
      nlopt::opt opt_instance(nlopt::LN_NELDERMEAD, N_parameters); */

  /*  opt_instance.set_min_objective(&nlopt_objective_function, (void*)this);  */
  /*   opt_instance.set_min_objective(&nlopt_objective_function, NULL); */

  /* double f;
     std::vector<double> vectorized_state_vector = std::vector<double>(state_vector, &state_vector[N_parameters]); */
  /* std::cout << "Here comes vectorized_state_vector:" << vectorized_state_vector; */
  /*  nlopt::result nlopt_result = opt_instance.optimize(vectorized_state_vector, &f);  */
  /*  nlopt::result nlopt_result = opt_instance.optimize(state_vector, &f); */

  nlopt::algorithm mango_nlopt_algorithm;
  switch (algorithm) {
  case mango::NLOPT_GN_DIRECT:
    mango_nlopt_algorithm = nlopt::GN_DIRECT;
    break;
  case mango::NLOPT_GN_DIRECT_L:
    mango_nlopt_algorithm = nlopt::GN_DIRECT_L;
    break;
  case mango::NLOPT_GN_DIRECT_L_RAND:
    mango_nlopt_algorithm = nlopt::GN_DIRECT_L_RAND;
    break;
  case mango::NLOPT_GN_DIRECT_NOSCAL:
    mango_nlopt_algorithm = nlopt::GN_DIRECT_NOSCAL;
    break;
  case mango::NLOPT_GN_DIRECT_L_NOSCAL:
    mango_nlopt_algorithm = nlopt::GN_DIRECT_L_NOSCAL;
    break;
  case mango::NLOPT_GN_DIRECT_L_RAND_NOSCAL:
    mango_nlopt_algorithm = nlopt::GN_DIRECT_L_RAND_NOSCAL;
    break;
  case mango::NLOPT_GN_ORIG_DIRECT:
    mango_nlopt_algorithm = nlopt::GN_ORIG_DIRECT;
    break;
  case mango::NLOPT_GN_ORIG_DIRECT_L:
    mango_nlopt_algorithm = nlopt::GN_ORIG_DIRECT_L;
    break;
  case mango::NLOPT_GN_CRS2_LM:
    mango_nlopt_algorithm = nlopt::GN_CRS2_LM;
    break;
  case mango::NLOPT_LN_COBYLA:
    mango_nlopt_algorithm = nlopt::LN_COBYLA;
    break;
  case mango::NLOPT_LN_BOBYQA:
    mango_nlopt_algorithm = nlopt::LN_BOBYQA;
    break;
  case mango::NLOPT_LN_PRAXIS:
    mango_nlopt_algorithm = nlopt::LN_PRAXIS;
    break;
  case mango::NLOPT_LN_NELDERMEAD:
    mango_nlopt_algorithm = nlopt::LN_NELDERMEAD;
    break;
  case mango::NLOPT_LN_SBPLX:
    mango_nlopt_algorithm = nlopt::LN_SBPLX;
    break;
  case mango::NLOPT_LD_LBFGS:
    mango_nlopt_algorithm = nlopt::LD_LBFGS;
    break;
  default:
    std::cout << "Error in optimize_nlopt. Unexpected algorithm!\n";
    exit(1);
  }

  /* I'll use the C interface of nlopt rather than the C++ interface, because the C++ interface requires 
     converting things back and forth between double[] and std::vector<double>.
     However, some nlopt constants like NLOPT_LN_NELDERMEAD conflict with mango's constants. 
     We can get around the latter issue by using the C++ nlopt:: constants and casting them
     from nlopt::algorithm to nlopt_algorithm. */
  nlopt_opt opt = nlopt_create((nlopt_algorithm)mango_nlopt_algorithm, N_parameters);
  nlopt_set_min_objective(opt, &nlopt_objective_function, (void*)this);
  double final_objective_function;
  nlopt_result result = nlopt_optimize(opt, state_vector, &final_objective_function);

  switch (result) {
  case nlopt::SUCCESS:
    std::cout << "nlopt generic success\n";
    break;
  case nlopt::STOPVAL_REACHED:
    std::cout << "nlopt success: stopval reached.\n";
    break;
  case nlopt::FTOL_REACHED:
    std::cout << "nlopt success: ftol reached.\n";
    break;
  case nlopt::XTOL_REACHED:
    std::cout << "nlopt success: xtol reached.\n";
    break;
  case nlopt::MAXEVAL_REACHED:
    std::cout << "nlopt: maxeval reached\n";
    break;
  case nlopt::MAXTIME_REACHED:
    std::cout << "nlopt: maxtime reached.\n";
    break;
  case nlopt::FAILURE:
    std::cout << "WARNING!!! NLOPT reported a generic failure. Results may or may not make sense.\n";
    break;
  case nlopt::INVALID_ARGS:
    std::cout << "nlopt failure: invalid arguments!\n";
    exit(1);
    break;
  case nlopt::OUT_OF_MEMORY:
    std::cout << "nlopt out of memory!\n";
    exit(1);
    break;
  case nlopt::ROUNDOFF_LIMITED:
    std::cout << "nlopt: WARNING! Limited by roundoff. Results may or may not make sense.\n";
    break;
  case nlopt::FORCED_STOP:
    std::cout << "nlopt forced stop!\n";
    exit(1);
    break;
  default:
    std::cout << "nlopt unexpected return value!\n";
    exit(1);
  }

  nlopt_destroy(opt);

#else
  std::cout << "Error! A NLOPT algorithm was requested, but Mango was compiled without NLOPT support.\n";
  exit(1);
#endif
}

/*double mango::problem::nlopt_objective_function(unsigned n, const double* x, double* grad, void* f_data) { */
double nlopt_objective_function(unsigned n, const double* x, double* grad, void* f_data) { 
  mango::problem* this_problem = (mango::problem*) f_data; 
  bool failed;
  double f;

  if (grad == NULL) {
    /* Gradient is not required. */
    this_problem->objective_function_wrapper(x, &f, &failed);
    /* objective_function_wrapper(x, &f, &failed); */
  } else {
    /* Gradient is required. */
    if (this_problem->is_least_squares()) {
      /* mango_finite_difference_Jacobian_to_gradient(problem, residual_function, x, f, grad) goes here. */
    } else {
      this_problem->finite_difference_gradient(x, &f, grad);
    }
  }

  if (failed) f = mango::NUMBER_FOR_FAILED;

  return f;
}

