module mango_problem_mod

  implicit none

  type :: mango_problem
     logical :: least_squares
     integer :: N_parameters
     integer :: N_proc_groups
     character(100) :: algorithm
  end type mango_problem


end module mango_problem_mod
