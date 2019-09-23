subroutine mango_group_leaders_least_squares_loop(problem, residual_function)

  use mango

  implicit none

  include 'mpif.h'

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  procedure(mango_residual_function_interface) :: residual_function
  double precision, allocatable :: state_vector(:), Jacobian(:,:), f(:)
  integer :: ierr, data(1)

  !----------------------------------------------

  allocate(state_vector(problem%N_parameters))
  allocate(f(problem%N_terms))
  allocate(Jacobian(problem%N_parameters,problem%N_terms))

  do
     ! Wait for proc0_world to send us a message.
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_group_leaders,ierr)
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",problem%mpi_rank_world," (a group leader) is exiting."
        exit
     else
        print "(a,i4,a)", "Proc",problem%mpi_rank_world," (a group leader) is starting finite-difference Jacobian calculation."
        call mango_finite_difference_Jacobian(problem, residual_function, state_vector, f, Jacobian)
     end if
  end do

  deallocate(state_vector, f, Jacobian)

end subroutine mango_group_leaders_least_squares_loop
