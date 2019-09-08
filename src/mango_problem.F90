module mango_problem_mod

  implicit none

!  include mpi.f

  abstract interface
  subroutine objective_function_interface(x,f,failed)
    double precision, intent(in) :: x(2)
    double precision, intent(out) :: f
    logical, intent(out) :: failed
  end subroutine objective_function_interface
  end interface

  type :: mango_problem
     integer :: mpi_comm
     logical :: least_squares
     integer :: N_parameters
     integer :: N_proc_groups
     integer, allocatable :: mpi_sub_comms(:)
     character(100) :: algorithm
     procedure(objective_function_interface), pointer, nopass :: objective_function
  end type mango_problem


  character(len=*), parameter :: &
       mango_algorithm_petsc_pounders = "petsc_pounders", &
       mango_algorithm_petsc_nm = "petsc_nm", &
       mango_algorithm_hopspack = "hopspack", &
       mango_algorithm_nlopt_ln_cobyla = "nlopt_ln_cobyla"

  double precision, parameter :: mango_huge = 1.0d+12

end module mango_problem_mod
