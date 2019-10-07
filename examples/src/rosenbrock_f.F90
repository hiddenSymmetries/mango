program rosenbrock

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, dimension(2) :: state_vector = (/ 3.0, 4.0 /)

  !---------------------------------------------

  print *,"Hello world from rosenbrock_f"

  call mpi_init(ierr)

  call mango_problem_create(problem,2,state_vector)
  print *,"Here comes state vector:",state_vector
  !call mango_set_algorithm(problem, 2)
  !call mango_set_algorithm_from_string(problem, "nlopt_ln_praxis")
  call mango_read_input_file(problem, "../input/mango_in.rosenbrock_f")
  call mango_mpi_init(problem, MPI_COMM_WORLD)
  call mango_problem_destroy(problem)

  call mpi_finalize(ierr)

  print *,"Good bye!"

end program rosenbrock

