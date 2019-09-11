subroutine mango_optimize_nlopt(problem, objective_function)

  use mango

  implicit none

#if defined(MANGO_NLOPT_AVAILABLE)
  include 'nlopt.f'
#endif

  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function

#ifdef MANGO_NLOPT_AVAILABLE
  integer*8 opt
  integer :: ires
  double precision :: minimum_value
  integer :: f_data

  !-------------------------------------------


  print *,"Hello world from mango_optimize_nlopt."

  opt = 0
  ires = 0

  select case (trim(problem%algorithm))
     ! Global, derivative-free:
  case (mango_algorithm_nlopt_gn_direct)
     call nlo_create(opt, NLOPT_GN_DIRECT, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_direct_l)
     call nlo_create(opt, NLOPT_GN_DIRECT_L, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_direct_l_rand)
     call nlo_create(opt, NLOPT_GN_DIRECT_L_RAND, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_direct_noscal)
     call nlo_create(opt, NLOPT_GN_DIRECT_NOSCAL, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_direct_l_noscal)
     call nlo_create(opt, NLOPT_GN_DIRECT_L_NOSCAL, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_direct_l_rand_noscal)
     call nlo_create(opt, NLOPT_GN_DIRECT_L_RAND_NOSCAL, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_orig_direct)
     call nlo_create(opt, NLOPT_GN_ORIG_DIRECT, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_orig_direct_l)
     call nlo_create(opt, NLOPT_GN_ORIG_DIRECT_L, problem%N_parameters)
  case (mango_algorithm_nlopt_gn_crs2_lm)
     call nlo_create(opt, NLOPT_GN_CRS2_LM, problem%N_parameters)
     ! Local, derivative-free:
  case (mango_algorithm_nlopt_ln_cobyla)
     call nlo_create(opt, NLOPT_LN_COBYLA, problem%N_parameters)
  case (mango_algorithm_nlopt_ln_bobyqa)
     call nlo_create(opt, NLOPT_LN_BOBYQA, problem%N_parameters)
  case (mango_algorithm_nlopt_ln_praxis)
     call nlo_create(opt, NLOPT_LN_PRAXIS, problem%N_parameters)
  case (mango_algorithm_nlopt_ln_neldermead)
     call nlo_create(opt, NLOPT_LN_NELDERMEAD, problem%N_parameters)
  case (mango_algorithm_nlopt_ln_sbplx)
     call nlo_create(opt, NLOPT_LN_SBPLX, problem%N_parameters)
     ! Local, derivative-based
  case (mango_algorithm_nlopt_lD_LBFGS)
     call nlo_create(opt, NLOPT_LD_LBFGS, problem%N_parameters)
  case default
     print "(a)","Error! Should not get here."
  end select

  if (opt==0) stop "NLOpt error: nlo_create failed."

  call nlo_set_min_objective(ires, opt, mango_nlopt_objective_function, f_data)
  if (ires <= 0) then
     print *,"Error in nlo_set_min_objective. ires=",ires
     stop
  end if

  call nlo_optimize(ires, opt, problem%state_vector, minimum_value)
  if (ires == -4) then
     print *,"NLOpt exited with ires=-4, meaning the optimization halted due to roundoff errors. Result may or may not be meaningful."
  elseif (ires <= 0) then
     print *,"Error in nlo_optimize. ires=",ires
     stop
  else
     print *,"NLOpt succeeded. ires=",ires
     select case (ires)
     case (1)
        print *,"  Generic success"
     case (2)
        print *,"  stopval reached."
     case (3)
        print *,"  ftol_rel or ftol_abs reached."
     case (4)
        print *,"  xtol_rel or xtol_abs reached."
     case (5)
        print *,"  maxeval reached."
     case (6)
        print *,"  maxtime reached."
     case default
        print *,"  Unknown ires value."
     end select
  end if

  call nlo_destroy(opt)

contains

  subroutine mango_nlopt_objective_function(f, n, x, grad, need_gradient, f_data)

    implicit none

    integer, intent(in) :: need_gradient, n
    double precision, intent(in) :: x(n)
    double precision :: f, grad(n)
    integer :: f_data
    logical :: failed

    print *,"mango_nlopt_objective_function: size(x)=",size(x)
    print *,"x=",x
    if (need_gradient.ne.0) then
       !print *,"mango_nlopt_objective_function: need_gradient is not yet supported in mango. need_gradient=",need_gradient
       !stop
       call mango_finite_difference_gradient(problem, objective_function, x, f, grad)
    else
       call mango_objective_function_wrapper(problem, objective_function, x, f, failed)
       if (failed) f = mango_huge
    end if

  end subroutine mango_nlopt_objective_function

#else
  stop "Error! A NLOpt algorithm was requested, but Mango was compiled without NLOpt support"
#endif

end subroutine mango_optimize_nlopt
