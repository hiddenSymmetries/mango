program rosenbrock

  use mango
  !use iso_c_binding

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, dimension(2) :: state_vector = (/ 0.0d+0, 0.0d+0 /)
  double precision, dimension(2) :: targets      = (/ 1.0d+0, 0.0d+0 /)
  double precision, dimension(2) :: sigmas       = (/ 1.0d+0, 0.1d+0 /)
  double precision :: best_residual_function(2), best_objective_function
  !external objective_function
  !procedure(objective_function_interface), pointer :: objective_function

  !---------------------------------------------

  print *,"Hello world from rosenbrock_f"
  call mpi_init(ierr)

  !call mango_problem_create(problem,2,state_vector,dummy,objective_function)
  call mango_problem_create_least_squares(problem, 2, state_vector, 2, targets, sigmas, best_residual_function, residual_function)
  print *,"Here comes state vector:",state_vector
  !call mango_set_algorithm(problem, 2)
  !call mango_set_algorithm_from_string(problem, "nlopt_ln_praxis")
  call mango_read_input_file(problem, "../input/mango_in.rosenbrock_f")
  call mango_set_output_filename(problem, "../output/mango_out.rosenbrock_f")
  call mango_mpi_init(problem, MPI_COMM_WORLD)
  call mango_set_max_function_evaluations(problem, 2000)

  if (mango_is_proc0_worker_groups(problem)) then
     best_objective_function = mango_optimize(problem)

     print *,"Best state vector:",state_vector
     print *,"Best objective function: ",best_objective_function
     print *,"Best residual function: ",best_residual_function
     print *,"Best function evaluation was ",mango_get_best_function_evaluation(problem)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,mango_get_mpi_comm_worker_groups(problem),ierr)
     if (ierr .ne. 0) print *,"Error A on proc0!"
  else
     call worker(problem)
  end if

  call mango_problem_destroy(problem)

  call mpi_finalize(ierr)

  print *,"Good bye!"

contains

!subroutine objective_function(N, x, f)
!  implicit none
!  integer, intent(in) :: N
!  double precision, intent(in) :: x(N)
!  double precision, intent(out) :: f
!
!  print *,"Hi from fortran. N=",N," size(x)=",size(x)
!  f = sum((x-2)*(x-2))
!  print *,"In fortran, x=",x,", f=",f
!
!end subroutine objective_function

subroutine residual_function(N_parameters, x, N_terms, f, failed, problem)
  use iso_c_binding
  implicit none
  integer(C_int), intent(in) :: N_parameters
  real(C_double), intent(in) :: x(N_parameters)
  integer(C_int), intent(in) :: N_terms
  real(C_double), intent(out) :: f(N_terms)
  integer(C_int), intent(out) :: failed
  type(mango_problem), value, intent(in) :: problem

  print *,"Hi from fortran. N_parameters=",N_parameters," size(x)=",size(x)
  !f = (x(1) - 1) * (x(1) - 1) + 100 * (x(2) - x(1)*x(1)) * (x(2) - x(1)*x(1))
  f(1) = x(1)
  f(2) = x(2) - x(1) * x(1)
  failed = 0

end subroutine residual_function

end program rosenbrock

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem
  integer :: ierr, data(1)

  do
     call mpi_bcast(data,1,MPI_INTEGER,0,mango_get_mpi_comm_worker_groups(problem),ierr)
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",mango_get_mpi_rank_world(problem)," is exiting."
        exit
     else
        print "(a,i4,a,i4)", "Proc",mango_get_mpi_rank_world(problem)," is doing calculation",data(1)
     end if
  end do

end subroutine worker
