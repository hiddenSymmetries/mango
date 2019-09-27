program chwirut

  use mango

  implicit none

  include 'mpif.h'
  
  integer :: ierr, N_procs, mpi_rank, data(1)
  logical :: proc0
  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
!  procedure(mango_residual_function_interface) :: residual_function
  integer, parameter :: data_length = 214
  double precision :: y(data_length), t(data_length)

  !---------------------------------------------

  print *,"Hello world from chwirut"

  call init_data(data_length, t, y)

  call mpi_init(ierr)

  call mango_read_namelist(problem,'../input/mango_in.chwirut')

  call mango_mpi_init(problem, MPI_COMM_WORLD)

  problem%output_filename = '../output/mango_out.chwirut'

  ! Set initial condition
  allocate(problem%state_vector(3))
  problem%state_vector(1) = 0.15d+0
  problem%state_vector(2) = 0.008d+0
  problem%state_vector(3) = 0.01d+0
     
  allocate(problem%targets(data_length))
  problem%targets = 0.0d+0
     
  allocate(problem%sigmas(data_length))
  problem%sigmas = 1.0d+0
     
  if (problem%proc0_worker_groups) then
     call mango_optimize_least_squares(problem, residual_function)

     ! Make workers stop
     data = -1
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
     if (ierr .ne. 0) print *,"Error A on proc0!"
  else
     call worker(problem)
  end if

  call mango_mpi_finalize(problem)
  call mpi_finalize(ierr)

  print *,"Good bye!"

contains


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine residual_function(problem, x, f, failed)

  use mango

  implicit none

  include 'mpif.h'

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  double precision, intent(in) :: x(:)
  double precision, intent(out) :: f(:)
  logical, intent(out) :: failed
  !double precision, parameter :: a = 1, b=100
  integer :: data(1), ierr, j

  !---------------------------------------------

  print *,"chwirut/residual_function: size(x)=",size(x),", size(f)=",size(f)

  ! Mobilize the workers in the group with this group leader:
  data = problem%worker_group
  call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
  if (ierr .ne. 0) print *,"Error on proc0 in residual_function!"
  
  do j = 1, data_length
     f(j) = y(j) - exp(-x(1)*t(j))/(x(2)+x(3)*t(j))
  end do

  failed = .false.

  print *,"chwirut/residual_function: x=",x,", f=",f

end subroutine residual_function

end program chwirut


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

subroutine worker(problem)

  use mango

  implicit none

  include 'mpif.h'

  !type(mango_least_squares_problem) :: problem
  type(mango_problem) :: problem
  integer :: ierr, mpi_rank, data(1)

  call mpi_comm_rank(MPI_COMM_WORLD, mpi_rank, ierr)
  do
     call mpi_bcast(data,1,MPI_INTEGER,0,problem%mpi_comm_worker_groups,ierr)
     if (ierr .ne. 0) print *,"Error on proc",mpi_rank," in worker: bcast!"
     if (data(1) < 0) then
        print "(a,i4,a)", "Proc",mpi_rank," is exiting."
        exit
     else
        print "(a,i4,a,i4)", "Proc",mpi_rank," is doing calculation",data(1)
     end if
  end do

end subroutine worker

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
