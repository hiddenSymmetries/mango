program rosenbrock

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem

  !---------------------------------------------

  print *,"Hello world from rosenbrock"

  call mpi_init(ierr)

  call mango_problem_create(problem)
  call mango_set_algorithm(problem, 2)
  call mango_mpi_init(problem, MPI_COMM_WORLD)
  call mango_problem_destroy(problem)

  call mpi_finalize(ierr)

  print *,"Good bye!"

end program rosenbrock

