subroutine mango_mpi_init(problem, mpi_comm)

  ! This subroutine assumes the following variables have been set on all procs of problem%mpi_comm_world:
  !   problem%mpi_comm_world (default = MPI_COMM_WORLD)
  !   problem%N_worker_groups (default = -1)

  use mango
  
  implicit none

  include 'mpif.h'

  type(mango_problem) :: problem
  integer, intent(in) :: mpi_comm
  integer :: ierr, mpi_rank, N_procs
  integer, parameter :: buffer_length = 1000
  character(len=buffer_length) :: proc_assignments_string
  integer :: i, tag, color
  integer :: mpi_status(MPI_STATUS_SIZE)
  !-----------------------------------------------------------------

  ierr = 0
  !problem%mpi_comm_world = mpi_comm
  call mpi_comm_dup(mpi_comm, problem%mpi_comm_world, ierr)

  call mpi_comm_rank(problem%mpi_comm_world, problem%mpi_rank_world, ierr)
  call mpi_comm_size(problem%mpi_comm_world, problem%N_procs_world, ierr)
  problem%proc0_world = (problem%mpi_rank_world==0)

  ! Ensure N_worker_groups is within the range [1, N_procs_world]
  if (problem%N_worker_groups < 1) problem%N_worker_groups = problem%N_procs_world
  if (problem%N_worker_groups > problem%N_procs_world) problem%N_worker_groups = problem%N_procs_world

  select case (trim(problem%algorithm))
     ! Algorithms that cannot use >1 worker groups:
  case (mango_algorithm_petsc_pounders, &
       mango_algorithm_petsc_nm, &
       mango_algorithm_nlopt_gn_direct, &
       mango_algorithm_nlopt_gn_direct_l, &
       mango_algorithm_nlopt_gn_direct_l_rand, &
       mango_algorithm_nlopt_gn_direct_noscal, &
       mango_algorithm_nlopt_gn_direct_l_noscal, &
       mango_algorithm_nlopt_gn_direct_l_rand_noscal, &
       mango_algorithm_nlopt_gn_orig_direct, &
       mango_algorithm_nlopt_gn_orig_direct_l, &
       mango_algorithm_nlopt_gn_crs2_lm, &
       mango_algorithm_nlopt_ln_cobyla, &
       mango_algorithm_nlopt_ln_bobyqa, &
       mango_algorithm_nlopt_ln_praxis, &
       mango_algorithm_nlopt_ln_neldermead, &
       mango_algorithm_nlopt_ln_sbplx)

     problem%N_worker_groups = 1 ! There is no point having >1 worker groups with these algorithms.

     ! Algorithms that can use >1 worker groups:
  case (mango_algorithm_nlopt_ld_lbfgs, &
       mango_algorithm_hopspack)

     if (problem%N_procs_world>1 .and. problem%N_worker_groups == 1 .and. problem%proc0_world) then
#define bold_line "*******************************************************************************************"
        print "(a)", bold_line
        print "(a)", bold_line
        print "(a)","WARNING!!! You have chosen an algorithm that can exploit concurrent function evaluations"
        print "(a)","but you have set N_worker_groups=1. You probably want a larger value."
        print "(a)", bold_line
        print "(a)", bold_line
     end if

  case default
     print "(a,a)","Error! Unrecognized algorithm: ",trim(problem%algorithm)
     stop
  end select

  if (problem%proc0_world) print "(a,i0)","Number of worker groups: ",problem%N_worker_groups

  problem%worker_group = (problem%mpi_rank_world * problem%N_worker_groups) / problem%N_procs_world ! Note integer division, so there is an implied floor()

  ! color = worker_group
  ! key = mpi_rank_world
  call mpi_comm_split(problem%mpi_comm_world, problem%worker_group, problem%mpi_rank_world, problem%mpi_comm_worker_groups, ierr)
  if (ierr .ne. 0) print *,"Error A"
  call mpi_comm_rank(problem%mpi_comm_worker_groups, problem%mpi_rank_worker_groups, ierr)
  if (ierr .ne. 0) print *,"Error A"
  call mpi_comm_size(problem%mpi_comm_worker_groups, problem%N_procs_worker_groups, ierr)
  if (ierr .ne. 0) print *,"Error A"
  problem%proc0_worker_groups = (problem%mpi_rank_worker_groups==0)

  ! Now set up the group_leaders communicator
  if (problem%proc0_worker_groups) then
     color = 0
  else
     color = MPI_UNDEFINED
  end if
  call mpi_comm_split(problem%mpi_comm_world, color, problem%mpi_rank_world, problem%mpi_comm_group_leaders, ierr)
  if (ierr .ne. 0) print *,"Error M"
  if (problem%proc0_worker_groups) then
     call mpi_comm_rank(problem%mpi_comm_group_leaders, problem%mpi_rank_group_leaders, ierr)
     if (ierr .ne. 0) print *,"Error N"
     call mpi_comm_size(problem%mpi_comm_group_leaders, problem%N_procs_group_leaders, ierr)
     if (ierr .ne. 0) print *,"Error O"
  else
     ! We are not allowed to query the rank from procs that are not members of mpi_comm_group_leaders.
     problem%mpi_rank_group_leaders = -1
     problem%N_procs_group_leaders = -1
  end if

  write (proc_assignments_string,fmt="(a,7(i4,a))") "Proc ",problem%mpi_rank_world," of",problem%N_procs_world, &
       " in MPI_COMM_WORLD is in worker group ",problem%worker_group,", has rank",problem%mpi_rank_worker_groups, &
       " of",problem%N_procs_worker_groups," in mpi_comm_worker_groups, and has rank",problem%mpi_rank_group_leaders, &
       " of",problem%N_procs_group_leaders," in mpi_comm_group_leaders."

  ! Print the processor assignments in a coordinated manner.
  if (problem%proc0_world) then
     print "(a)", trim(proc_assignments_string)
     do i = 1,problem%N_procs_world - 1
        tag = i
        call mpi_recv(proc_assignments_string,buffer_length,MPI_CHAR,i,tag,problem%mpi_comm_world,mpi_status,ierr)
        print "(a)", trim(proc_assignments_string)
     end do
  else
     tag = problem%mpi_rank_world
     call mpi_send(proc_assignments_string,buffer_length,MPI_CHAR,0,tag,problem%mpi_comm_world,ierr)
  end if

end subroutine mango_mpi_init
