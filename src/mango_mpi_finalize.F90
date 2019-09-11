subroutine mango_mpi_finalize(problem)

  use mango

  implicit none

  type(mango_problem) :: problem
  integer :: ierr

  !-----------------------------------------------------------------

  call mpi_comm_free(problem%mpi_comm_worker_groups,ierr)
  if (problem%proc0_worker_groups) call mpi_comm_free(problem%mpi_comm_group_leaders,ierr)
 
end subroutine mango_mpi_finalize
