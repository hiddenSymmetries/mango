module mango

  implicit none

!  include mpi.f

  abstract interface
  !interface
  subroutine mango_objective_function_interface(x,f,failed)
    double precision, intent(in) :: x(:)
    double precision, intent(out) :: f
    logical, intent(out) :: failed
  end subroutine mango_objective_function_interface
  subroutine mango_residual_function_interface(x,f,failed)
    double precision, intent(in) :: x(:)
    double precision, intent(out) :: f(:)
    logical, intent(out) :: failed
  end subroutine mango_residual_function_interface
  end interface

  type :: mango_problem
     integer :: mpi_comm
     !logical :: least_squares
     integer :: N_parameters
     integer :: N_proc_groups
     double precision, allocatable :: state_vector(:)
     integer, allocatable :: mpi_sub_comms(:)
     character(100) :: algorithm
     !procedure(objective_function_interface), pointer, nopass :: objective_function
  end type mango_problem

  type, extends(mango_problem) :: mango_least_squares_problem
     double precision, allocatable :: targets(:), sigmas(:)
     integer :: N_terms
  end type mango_least_squares_problem

  character(len=*), parameter :: &
       mango_algorithm_petsc_pounders = "petsc_pounders", &
       mango_algorithm_petsc_nm = "petsc_nm", &
       mango_algorithm_hopspack = "hopspack", &
       mango_algorithm_nlopt_gn_direct = "nlopt_gn_direct", &
       mango_algorithm_nlopt_gn_direct_l = "nlopt_gn_direct_l", &
       mango_algorithm_nlopt_gn_direct_l_rand = "nlopt_gn_direct_l_rand", &
       mango_algorithm_nlopt_gn_direct_noscal = "nlopt_gn_direct_noscal", &
       mango_algorithm_nlopt_gn_direct_l_noscal = "nlopt_gn_direct_l_noscal", &
       mango_algorithm_nlopt_gn_direct_l_rand_noscal = "nlopt_gn_direct_l_rand_noscal", &
       mango_algorithm_nlopt_gn_orig_direct = "nlopt_gn_orig_direct", &
       mango_algorithm_nlopt_gn_orig_direct_l = "nlopt_gn_orig_direct_l", &
       mango_algorithm_nlopt_gn_crs2_lm = "nlopt_gn_crs2_lm", &
       mango_algorithm_nlopt_ln_cobyla = "nlopt_ln_cobyla", &
       mango_algorithm_nlopt_ln_bobyqa = "nlopt_ln_bobyqa", &
       mango_algorithm_nlopt_ln_praxis = "nlopt_ln_praxis", &
       mango_algorithm_nlopt_ln_neldermead = "nlopt_ln_neldermead", &
       mango_algorithm_nlopt_ln_sbplx = "nlopt_ln_sbplx"

  double precision, parameter :: mango_huge = 1.0d+12

end module mango
