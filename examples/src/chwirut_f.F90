! Copyright 2019, University of Maryland and the MANGO development team.
!
! This file is part of MANGO.
!
! MANGO is free software: you can redistribute it and/or modify it
! under the terms of the GNU Lesser General Public License as
! published by the Free Software Foundation, either version 3 of the
! License, or (at your option) any later version.
!
! MANGO is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
! Lesser General Public License for more details.
!
! You should have received a copy of the GNU Lesser General Public
! License along with MANGO.  If not, see
! <https://www.gnu.org/licenses/>.
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! This example is based on the PETSc example
! src/tao/leastsquares/examples/tutorials/chwirut*

#define verbose_level 0

program chwirut

  use mango_mod
  !use iso_c_binding

  implicit none

  include 'mpif.h'
  
  character(len=*), parameter :: extension = "chwirut_f"
  integer :: ierr, N_procs, mpi_rank
  logical :: proc0
  type(mango_problem) :: problem
  integer, parameter :: N_terms = 214
  integer, parameter :: N_parameters = 3
  double precision :: y(N_terms), t(N_terms)
  double precision, dimension(N_parameters) :: state_vector = (/ 0.15d+0, 0.008d+0, 0.01d+0 /)
  double precision :: targets(N_terms), sigmas(N_terms), best_residual_function(N_terms), best_objective_function

  !---------------------------------------------

  if (verbose_level > 0) print *,"Hello world from chwirut_f"
  call mpi_init(ierr)

  targets = 0
  sigmas = 1
  call init_data(N_terms, t, y)

  call mango_problem_create_least_squares(problem, N_parameters, state_vector, N_terms, targets, sigmas, best_residual_function, residual_function)
  if (verbose_level > 0) print *,"Here comes state vector:",state_vector
  !call mango_set_algorithm(problem, 2)
  !call mango_set_algorithm_from_string(problem, "nlopt_ln_praxis")
  call mango_set_verbose(problem, verbose_level)
  call mango_read_input_file(problem, "../input/mango_in." // extension)
  call mango_set_output_filename(problem, "../output/mango_out." // extension)
  call mango_mpi_init(problem, MPI_COMM_WORLD)
  call mango_mpi_partition_write(problem, "../output/mango_mpi." // extension)
  call mango_set_max_function_evaluations(problem, 2000)
  call mango_set_print_residuals_in_output_file(problem, .false.)
  call mango_set_N_line_search(problem, 3) ! To make results independent of the # of MPI processes, N_line_search must be set to any positive integer.

  if (mango_get_proc0_worker_groups(problem)) then
     best_objective_function = mango_optimize(problem)
     call mango_stop_workers(problem)
  else
     call worker(problem)
  end if

  if (mango_get_proc0_world(problem) .and. (verbose_level > 0)) then
     print *,"Best state vector:",state_vector
     print *,"Best objective function: ",best_objective_function
     print *,"Best residual function: ",best_residual_function
     print *,"Best function evaluation was ",mango_get_best_function_evaluation(problem)
  end if

  call mango_problem_destroy(problem)

  call mpi_finalize(ierr)

  if (verbose_level > 0) print *,"Good bye!"

contains

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine do_work(N_parameters, x, N_terms, f, start_index, stop_index)

  integer, intent(in) :: N_parameters, N_terms, start_index, stop_index
  double precision, intent(in) :: x(N_parameters)
  double precision, intent(out) :: f(N_terms)
  integer :: j

  do j = start_index, stop_index
     f(j) = y(j) - exp(-x(1)*t(j))/(x(2)+x(3)*t(j))
  end do

end subroutine do_work

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine residual_function(N_parameters, x, N_terms, f, failed, problem, user_data)
  use iso_c_binding
  implicit none
  integer(C_int), intent(in) :: N_parameters
  real(C_double), intent(in) :: x(N_parameters)
  integer(C_int), intent(in) :: N_terms
  real(C_double), intent(out) :: f(N_terms)
  integer(C_int), intent(out) :: failed
  type(mango_problem), value, intent(in) :: problem
  type(C_ptr), value, intent(in) :: user_data
  integer :: ierr, j, start_index, stop_index, N_procs_worker_groups
  integer :: mpi_status(MPI_STATUS_SIZE)
  integer :: mpi_comm_worker_groups

  if (verbose_level > 0) print *,"Hi from fortran. N_parameters=",N_parameters," size(x)=",size(x)
  N_procs_worker_groups = mango_get_N_procs_worker_groups(problem)
  mpi_comm_worker_groups = mango_get_mpi_comm_worker_groups(problem)

  ! Mobilize the workers in the group with this group leader:
  call mango_mobilize_workers(problem)

  ! Send the state vector to all the other procs:
  call mpi_bcast(x, N_parameters, MPI_DOUBLE, 0, mpi_comm_worker_groups, ierr)

  ! Compute the residual terms on this proc, or receive the terms from the worker procs.
  do j = 0, (N_procs_worker_groups-1)
     call partition_work(N_terms, j, N_procs_worker_groups, start_index, stop_index)
     !print "(4(a,i4))","residual function, j=",j," of",N_procs_worker_groups,", start_index=",start_index,", stop_index=",stop_index
     if (j==0) then
        call do_work(N_parameters, x, N_terms, f, start_index, stop_index)
     else
        call mpi_recv(f(start_index:stop_index), stop_index - start_index + 1, MPI_DOUBLE, j, j, mpi_comm_worker_groups,mpi_status,ierr)
     end if
  end do

  failed = 0

end subroutine residual_function

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango_mod

  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem
  integer :: ierr
  double precision, allocatable :: f(:), x(:)
  integer :: start_index, stop_index, N_terms, mpi_rank_worker_groups, N_parameters
  integer :: mpi_comm_worker_groups

  mpi_comm_worker_groups = mango_get_mpi_comm_worker_groups(problem)
  N_terms = mango_get_N_terms(problem)
  N_parameters = mango_get_N_parameters(problem)
  mpi_rank_worker_groups = mango_get_mpi_rank_worker_groups(problem)
  allocate(f(N_terms))
  allocate(x(N_parameters))
  call partition_work(N_terms, mpi_rank_worker_groups, mango_get_N_procs_worker_groups(problem), start_index, stop_index)

  do while (mango_continue_worker_loop(problem))
     if (verbose_level > 0) print "(a,i4,a,i4,a,i4)", "Proc",mango_get_mpi_rank_world(problem)," is processing indices",start_index," to",stop_index

     ! Get the state vector
     call mpi_bcast(x, N_parameters, MPI_DOUBLE, 0, mpi_comm_worker_groups, ierr)
       
     call do_work(N_parameters, x, N_terms, f, start_index, stop_index)

     ! Send my terms of the residual back to the master proc.
     call mpi_send(f(start_index:stop_index), stop_index - start_index + 1, MPI_DOUBLE, 0,mpi_rank_worker_groups,mpi_comm_worker_groups,ierr)
  end do

  if (verbose_level > 0) print "(a,i4,a)", "Proc",mango_get_mpi_rank_world(problem)," is exiting."
  deallocate(f,x)

end subroutine worker

end program chwirut

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine init_data(data_length, t, y)

  implicit none

  integer, intent(in) :: data_length
  double precision, intent(out) :: t(data_length), y(data_length)

  integer ::  i

      i=1
      y(i) =    92.9000d+0;  t(i) =  0.5000d+0; i=i+1
      y(i) =    78.7000d+0;  t(i) =   0.6250d+0; i=i+1
      y(i) =    64.2000d+0;  t(i) =   0.7500d+0; i=i+1
      y(i) =    64.9000d+0;  t(i) =   0.8750d+0; i=i+1
      y(i) =    57.1000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    43.3000d+0;  t(i) =   1.2500d+0; i=i+1
      y(i) =    31.1000d+0;  t(i) =  1.7500d+0; i=i+1
      y(i) =    23.6000d+0;  t(i) =  2.2500d+0; i=i+1
      y(i) =    31.0500d+0;  t(i) =  1.7500d+0; i=i+1
      y(i) =    23.7750d+0;  t(i) =  2.2500d+0; i=i+1
      y(i) =    17.7375d+0;  t(i) =  2.7500d+0; i=i+1
      y(i) =    13.8000d+0;  t(i) =  3.2500d+0; i=i+1
      y(i) =    11.5875d+0;  t(i) =  3.7500d+0; i=i+1
      y(i) =     9.4125d+0;  t(i) =  4.2500d+0; i=i+1
      y(i) =     7.7250d+0;  t(i) =  4.7500d+0; i=i+1
      y(i) =     7.3500d+0;  t(i) =  5.2500d+0; i=i+1
      y(i) =     8.0250d+0;  t(i) =  5.7500d+0; i=i+1
      y(i) =    90.6000d+0;  t(i) =  0.5000d+0; i=i+1
      y(i) =    76.9000d+0;  t(i) =  0.6250d+0; i=i+1
      y(i) =    71.6000d+0;  t(i) = 0.7500d+0; i=i+1
      y(i) =    63.6000d+0;  t(i) =  0.8750d+0; i=i+1
      y(i) =    54.0000d+0;  t(i) =  1.0000d+0; i=i+1
      y(i) =    39.2000d+0;  t(i) =  1.2500d+0; i=i+1
      y(i) =    29.3000d+0;  t(i) = 1.7500d+0; i=i+1
      y(i) =    21.4000d+0;  t(i) =  2.2500d+0; i=i+1
      y(i) =    29.1750d+0;  t(i) =  1.7500d+0; i=i+1
      y(i) =    22.1250d+0;  t(i) =  2.2500d+0; i=i+1
      y(i) =    17.5125d+0;  t(i) =  2.7500d+0; i=i+1
      y(i) =    14.2500d+0;  t(i) =  3.2500d+0; i=i+1
      y(i) =     9.4500d+0;  t(i) =  3.7500d+0; i=i+1
      y(i) =     9.1500d+0;  t(i) =  4.2500d+0; i=i+1
      y(i) =     7.9125d+0;  t(i) =  4.7500d+0; i=i+1
      y(i) =     8.4750d+0;  t(i) =  5.2500d+0; i=i+1
      y(i) =     6.1125d+0;  t(i) =  5.7500d+0; i=i+1
      y(i) =    80.0000d+0;  t(i) =  0.5000d+0; i=i+1
      y(i) =    79.0000d+0;  t(i) =  0.6250d+0; i=i+1
      y(i) =    63.8000d+0;  t(i) =  0.7500d+0; i=i+1
      y(i) =    57.2000d+0;  t(i) =  0.8750d+0; i=i+1
      y(i) =    53.2000d+0;  t(i) =  1.0000d+0; i=i+1
      y(i) =    42.5000d+0;  t(i) =  1.2500d+0; i=i+1
      y(i) =    26.8000d+0;  t(i) =  1.7500d+0; i=i+1
      y(i) =    20.4000d+0;  t(i) =  2.2500d+0; i=i+1
      y(i) =    26.8500d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    21.0000d+0;  t(i) =   2.2500d+0; i=i+1
      y(i) =    16.4625d+0;  t(i) =   2.7500d+0; i=i+1
      y(i) =    12.5250d+0;  t(i) =   3.2500d+0; i=i+1
      y(i) =    10.5375d+0;  t(i) =   3.7500d+0; i=i+1
      y(i) =     8.5875d+0;  t(i) =   4.2500d+0; i=i+1
      y(i) =     7.1250d+0;  t(i) =   4.7500d+0; i=i+1
      y(i) =     6.1125d+0;  t(i) =   5.2500d+0; i=i+1
      y(i) =     5.9625d+0;  t(i) =   5.7500d+0; i=i+1
      y(i) =    74.1000d+0;  t(i) =   0.5000d+0; i=i+1
      y(i) =    67.3000d+0;  t(i) =   0.6250d+0; i=i+1
      y(i) =    60.8000d+0;  t(i) =   0.7500d+0; i=i+1
      y(i) =    55.5000d+0;  t(i) =   0.8750d+0; i=i+1
      y(i) =    50.3000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    41.0000d+0;  t(i) =   1.2500d+0; i=i+1
      y(i) =    29.4000d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    20.4000d+0;  t(i) =   2.2500d+0; i=i+1
      y(i) =    29.3625d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    21.1500d+0;  t(i) =   2.2500d+0; i=i+1
      y(i) =    16.7625d+0;  t(i) =   2.7500d+0; i=i+1
      y(i) =    13.2000d+0;  t(i) =   3.2500d+0; i=i+1
      y(i) =    10.8750d+0;  t(i) =   3.7500d+0; i=i+1
      y(i) =     8.1750d+0;  t(i) =   4.2500d+0; i=i+1
      y(i) =     7.3500d+0;  t(i) =   4.7500d+0; i=i+1
      y(i) =     5.9625d+0;  t(i) =  5.2500d+0; i=i+1
      y(i) =     5.6250d+0;  t(i) =   5.7500d+0; i=i+1
      y(i) =    81.5000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    62.4000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    32.5000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    12.4100d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    13.1200d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    15.5600d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     5.6300d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    78.0000d+0;  t(i) =   .5000d+0; i=i+1
      y(i) =    59.9000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    33.2000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    13.8400d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    12.7500d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    14.6200d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     3.9400d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    76.8000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    61.0000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    32.9000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    13.8700d+0;  t(i) = 3.0000d+0; i=i+1
      y(i) =    11.8100d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    13.3100d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     5.4400d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    78.0000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    63.5000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    33.8000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    12.5600d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     5.6300d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    12.7500d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    13.1200d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     5.4400d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    76.8000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    60.0000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    47.8000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    32.0000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    22.2000d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    22.5700d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    18.8200d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    13.9500d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    11.2500d+0;  t(i) =   4.0000d+0; i=i+1
      y(i) =     9.0000d+0;  t(i) =   5.0000d+0; i=i+1
      y(i) =     6.6700d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    75.8000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    62.0000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    48.8000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    35.2000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    20.0000d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    20.3200d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    19.3100d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    12.7500d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    10.4200d+0;  t(i) =   4.0000d+0; i=i+1
      y(i) =     7.3100d+0;  t(i) =   5.0000d+0; i=i+1
      y(i) =     7.4200d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    70.5000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    59.5000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    48.5000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    35.8000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    21.0000d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    21.6700d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    21.0000d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    15.6400d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     8.1700d+0;  t(i) =   4.0000d+0; i=i+1
      y(i) =     8.5500d+0;  t(i) =   5.0000d+0; i=i+1
      y(i) =    10.1200d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    78.0000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    66.0000d+0;  t(i) =    .6250d+0; i=i+1
      y(i) =    62.0000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    58.0000d+0;  t(i) =    .8750d+0; i=i+1
      y(i) =    47.7000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    37.8000d+0;  t(i) =   1.2500d+0; i=i+1
      y(i) =    20.2000d+0;  t(i) =   2.2500d+0; i=i+1
      y(i) =    21.0700d+0;  t(i) =   2.2500d+0; i=i+1
      y(i) =    13.8700d+0;  t(i) =   2.7500d+0; i=i+1
      y(i) =     9.6700d+0;  t(i) =   3.2500d+0; i=i+1
      y(i) =     7.7600d+0;  t(i) =   3.7500d+0; i=i+1
      y(i) =     5.4400d+0;  t(i) =  4.2500d+0; i=i+1
      y(i) =     4.8700d+0;  t(i) =  4.7500d+0; i=i+1
      y(i) =     4.0100d+0;  t(i) =   5.2500d+0; i=i+1
      y(i) =     3.7500d+0;  t(i) =   5.7500d+0; i=i+1
      y(i) =    24.1900d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    25.7600d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    18.0700d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    11.8100d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    12.0700d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    16.1200d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    70.8000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    54.7000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    48.0000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    39.8000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    29.8000d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    23.7000d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    29.6200d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    23.8100d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    17.7000d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    11.5500d+0;  t(i) =   4.0000d+0; i=i+1
      y(i) =    12.0700d+0;  t(i) =   5.0000d+0; i=i+1
      y(i) =     8.7400d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    80.7000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    61.3000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    47.5000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    29.0000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    24.0000d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    17.7000d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    24.5600d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    18.6700d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    16.2400d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     8.7400d+0;  t(i) =   4.0000d+0; i=i+1
      y(i) =     7.8700d+0;  t(i) =   5.0000d+0; i=i+1
      y(i) =     8.5100d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    66.7000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    59.2000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    40.8000d+0;  t(i) =   1.0000d+0; i=i+1
      y(i) =    30.7000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    25.7000d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    16.3000d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    25.9900d+0;  t(i) =   2.0000d+0; i=i+1
      y(i) =    16.9500d+0;  t(i) =   2.5000d+0; i=i+1
      y(i) =    13.3500d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     8.6200d+0;  t(i) =   4.0000d+0; i=i+1
      y(i) =     7.2000d+0;  t(i) =   5.0000d+0; i=i+1
      y(i) =     6.6400d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    13.6900d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    81.0000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    64.5000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    35.5000d+0;  t(i) =   1.5000d+0; i=i+1
      y(i) =    13.3100d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     4.8700d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    12.9400d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =     5.0600d+0;  t(i) =   6.0000d+0; i=i+1
      y(i) =    15.1900d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    14.6200d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    15.6400d+0;  t(i) =   3.0000d+0; i=i+1
      y(i) =    25.5000d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    25.9500d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    81.7000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    61.6000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    29.8000d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    29.8100d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    17.1700d+0;  t(i) =   2.7500d+0; i=i+1
      y(i) =    10.3900d+0;  t(i) =   3.7500d+0; i=i+1
      y(i) =    28.4000d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    28.6900d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    81.3000d+0;  t(i) =    .5000d+0; i=i+1
      y(i) =    60.9000d+0;  t(i) =    .7500d+0; i=i+1
      y(i) =    16.6500d+0;  t(i) =   2.7500d+0; i=i+1
      y(i) =    10.0500d+0;  t(i) =   3.7500d+0; i=i+1
      y(i) =    28.9000d+0;  t(i) =   1.7500d+0; i=i+1
      y(i) =    28.9500d+0;  t(i) =   1.7500d+0

      if (i .ne. data_length) then
         print *,"Error! data_length=",data_length," but i=",i
         stop
      end if

end subroutine init_data

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine partition_work(N_terms, j, N_procs, start_index, stop_index)
  ! j is the 0-based processor index, as in MPI
  ! However, start_index and stop_index are 1-based, as in fortran arrays.
  integer, intent(in) :: N_terms, j, N_procs
  integer, intent(out) :: start_index, stop_index

  start_index = floor(N_terms*(j+0.0d+0)/N_procs) + 1
  stop_index  = floor(N_terms*(j+1.0d+0)/N_procs)
end subroutine partition_work
