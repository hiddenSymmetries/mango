subroutine mango_group_leaders_loop(problem, objective_function)

  use mango

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem
  procedure(mango_objective_function_interface) :: objective_function
  double precision :: f
  double precision, allocatable :: state_vector(:), gradient(:)
  integer :: ierr, data(1)

  !----------------------------------------------

  allocate(state_vector(problem%N_parameters))
  allocate(gradient(problem%N_parameters))

  do
     ! Wait for proc0_world to send us a message.
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_group_leaders,ierr)
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",problem%mpi_rank_world," (a group leader) is exiting."
        exit
     else
        print "(a,i4,a)", "Proc",problem%mpi_rank_world," (a group leader) is starting finite-difference gradient calculation."
        call mango_finite_difference_gradient(problem, objective_function, state_vector, f, gradient)
     end if
  end do

  deallocate(state_vector, gradient)

end subroutine mango_group_leaders_loop
