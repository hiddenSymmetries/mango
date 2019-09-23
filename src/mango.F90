module mango

  implicit none

  !include 'mpif.h'

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
       mango_algorithm_nlopt_ln_sbplx = "nlopt_ln_sbplx", &
       mango_algorithm_nlopt_ld_lbfgs = "nlopt_ld_lbfgs"

  double precision, parameter :: mango_huge = 1.0d+12

  type :: mango_problem
     ! Should I separate this into mpi info vs numerics info?
     integer :: mpi_comm_world
     integer :: mpi_comm_worker_groups, mpi_comm_group_leaders
     logical :: proc0_world, proc0_worker_groups
     integer :: mpi_rank_world, mpi_rank_worker_groups, mpi_rank_group_leaders
     integer :: N_procs_world, N_procs_worker_groups, N_procs_group_leaders
     integer :: N_worker_groups = -1
     integer :: worker_group
     integer :: N_parameters
     double precision, allocatable :: state_vector(:)
     character(len=100) :: algorithm = mango_algorithm_nlopt_ln_neldermead
     character(len=200) :: output_filename = 'mango_out'
     integer :: output_unit = 11
     integer :: function_evaluations = 0
     double precision :: finite_difference_step_size = 1.0d-8
     logical :: centered_differences = .false.
     !procedure(objective_function_interface), pointer, nopass :: objective_function
     double precision, allocatable :: targets(:), sigmas(:)
     integer :: N_terms
     logical :: least_squares = .false.
  end type mango_problem

!!$  type, extends(mango_problem) :: mango_least_squares_problem
!!$     double precision, allocatable :: targets(:), sigmas(:)
!!$     integer :: N_terms
!!$  end type mango_least_squares_problem

  abstract interface
  !interface

  subroutine mango_objective_function_interface(problem,x,f,failed)
    import :: mango_problem
    type(mango_problem) :: problem
    double precision, intent(in) :: x(:)
    double precision, intent(out) :: f
    logical, intent(out) :: failed
  end subroutine mango_objective_function_interface

  subroutine mango_residual_function_interface(problem,x,f,failed)
    !import :: mango_least_squares_problem
    !type(mango_least_squares_problem) :: problem
    import :: mango_problem
    type(mango_problem) :: problem
    double precision, intent(in) :: x(:)
    double precision, intent(out) :: f(:)
    logical, intent(out) :: failed
  end subroutine mango_residual_function_interface

  end interface

end module mango
