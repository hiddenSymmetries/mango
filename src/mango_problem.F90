module mango_problem_mod

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
       mango_algorithm_nlopt_ln_cobyla = "nlopt_ln_cobyla"

  double precision, parameter :: mango_huge = 1.0d+12

end module mango_problem_mod
