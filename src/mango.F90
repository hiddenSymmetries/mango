module mango

#define mango_interface_string_length 256

  ! Modeled after
  ! https://modelingguru.nasa.gov/docs/DOC-2642
  ! http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects

  use, intrinsic :: ISO_C_Binding
  implicit none
  private
  
  type mango_problem
     private
     type(C_ptr) :: object = C_NULL_ptr
  end type mango_problem

  interface
!     function C_mango_problem_create(N_parameters) result(this) bind(C,name="mango_problem_create")
!       import
!       integer(C_int) :: N_parameters
!       type(C_ptr) :: this
!     end function C_mango_problem_create
!     function C_mango_problem_create_least_squares(N_parameters,N_terms) result(this) bind(C,name="mango_problem_create_least_squares")
!       import
!       integer(C_int) :: N_parameters, N_terms
!       type(C_ptr) :: this
!     end function C_mango_problem_create_least_squares
     function C_mango_problem_create(N_parameters, state_vector, dummy, objective_function) result(this) bind(C,name="mango_problem_create")
       import
       integer(C_int) :: N_parameters, dummy
       type(C_ptr) :: this
       !real(C_double) :: state_vector(:)
       !real(C_double) :: state_vector(N_parameters)
       real(C_double) :: state_vector
       type(C_funptr), value :: objective_function ! The "value" attribute is critical; otherwise a pointer to the pointer is passed instead of the pointer.
     end function C_mango_problem_create
     function C_mango_problem_create_least_squares(N_parameters, state_vector, N_terms, targets, sigmas, residual_function) result(this) bind(C,name="mango_problem_create_least_squares")
       import
       integer(C_int) :: N_parameters, N_terms
       type(C_ptr) :: this
       real(C_double) :: state_vector, targets, sigmas
       type(C_funptr), value :: residual_function ! The "value" attribute is critical; otherwise a pointer to the pointer is passed instead of the pointer.
     end function C_mango_problem_create_least_squares
     subroutine C_mango_problem_destroy (this) bind(C,name="mango_problem_destroy")
       import
       type(C_ptr), value :: this
     end subroutine C_mango_problem_destroy
     subroutine C_mango_set_algorithm (this, algorithm) bind(C,name="mango_set_algorithm")
       import
       integer(C_int) :: algorithm
       type(C_ptr), value :: this
     end subroutine C_mango_set_algorithm
     subroutine C_mango_set_algorithm_from_string (this, algorithm_str) bind(C,name="mango_set_algorithm_from_string")
       import
       type(C_ptr), value :: this
       character(C_char) :: algorithm_str(mango_interface_string_length)
     end subroutine C_mango_set_algorithm_from_string
     subroutine C_mango_read_input_file(this, filename) bind(C,name="mango_read_input_file")
       import
       type(C_ptr), value :: this
       character(C_char) :: filename(mango_interface_string_length)
     end subroutine C_mango_read_input_file
     subroutine C_mango_set_output_filename(this, filename) bind(C,name="mango_set_output_filename")
       import
       type(C_ptr), value :: this
       character(C_char) :: filename(mango_interface_string_length)
     end subroutine C_mango_set_output_filename
     subroutine C_mango_mpi_init (this, mpi_comm) bind(C,name="mango_mpi_init")
       import
       integer(C_int) :: mpi_comm
       type(C_ptr), value :: this
     end subroutine C_mango_mpi_init
     subroutine C_mango_optimize(this) bind(C,name="mango_optimize")
       import
       type(C_ptr), value :: this
     end subroutine C_mango_optimize
     function C_mango_get_mpi_rank_world(this) result(mpi_rank) bind(C,name="mango_get_mpi_rank_world")
       import
       integer(C_int) :: mpi_rank
       type(C_ptr), value :: this
     end function C_mango_get_mpi_rank_world
     function C_mango_get_mpi_rank_worker_groups(this) result(mpi_rank) bind(C,name="mango_get_mpi_rank_worker_groups")
       import
       integer(C_int) :: mpi_rank
       type(C_ptr), value :: this
     end function C_mango_get_mpi_rank_worker_groups
     function C_mango_get_mpi_rank_group_leaders(this) result(mpi_rank) bind(C,name="mango_get_mpi_rank_group_leaders")
       import
       integer(C_int) :: mpi_rank
       type(C_ptr), value :: this
     end function C_mango_get_mpi_rank_group_leaders
     function C_mango_get_N_procs_world(this) result(N_procs) bind(C,name="mango_get_N_procs_world")
       import
       integer(C_int) :: N_procs
       type(C_ptr), value :: this
     end function C_mango_get_N_procs_world
     function C_mango_get_N_procs_worker_groups(this) result(N_procs) bind(C,name="mango_get_N_procs_worker_groups")
       import
       integer(C_int) :: N_procs
       type(C_ptr), value :: this
     end function C_mango_get_N_procs_worker_groups
     function C_mango_get_N_procs_group_leaders(this) result(N_procs) bind(C,name="mango_get_N_procs_group_leaders")
       import
       integer(C_int) :: N_procs
       type(C_ptr), value :: this
     end function C_mango_get_N_procs_group_leaders
     function C_mango_is_proc0_world(this) result(proc0) bind(C,name="mango_is_proc0_world")
       import
       integer(C_int) :: proc0
       type(C_ptr), value :: this
     end function C_mango_is_proc0_world
     function C_mango_is_proc0_worker_groups(this) result(proc0) bind(C,name="mango_is_proc0_worker_groups")
       import
       integer(C_int) :: proc0
       type(C_ptr), value :: this
     end function C_mango_is_proc0_worker_groups
     function C_mango_get_mpi_comm_world(this) result(comm) bind(C,name="mango_get_mpi_comm_world")
       import
       integer(C_int) :: comm
       type(C_ptr), value :: this
     end function C_mango_get_mpi_comm_world
     function C_mango_get_mpi_comm_worker_groups(this) result(comm) bind(C,name="mango_get_mpi_comm_worker_groups")
       import
       integer(C_int) :: comm
       type(C_ptr), value :: this
     end function C_mango_get_mpi_comm_worker_groups
     function C_mango_get_mpi_comm_group_leaders(this) result(comm) bind(C,name="mango_get_mpi_comm_group_leaders")
       import
       integer(C_int) :: comm
       type(C_ptr), value :: this
     end function C_mango_get_mpi_comm_group_leaders
     subroutine C_mango_set_centered_differences(this, centered_differences_int) bind(C,name="mango_set_centered_differences")
       import
       type(C_ptr), value :: this
       integer(C_int) :: centered_differences_int
     end subroutine C_mango_set_centered_differences
  end interface

  public :: mango_problem
  public :: mango_problem_create, mango_problem_create_least_squares, mango_problem_destroy, &
       mango_set_algorithm, mango_set_algorithm_from_string, mango_read_input_file, mango_set_output_filename, &
       mango_mpi_init, mango_optimize, &
       mango_get_mpi_rank_world, mango_get_mpi_rank_worker_groups, mango_get_mpi_rank_group_leaders, &
       mango_get_N_procs_world, mango_get_N_procs_worker_groups, mango_get_N_procs_group_leaders, &
       mango_is_proc0_world, mango_is_proc0_worker_groups, &
       mango_get_mpi_comm_world, mango_get_mpi_comm_worker_groups, mango_get_mpi_comm_group_leaders, &
       mango_set_centered_differences
  

  abstract interface
  subroutine objective_function_interface(N_parameters, state_vector, f, failed, this)
    import
    integer, intent(in) :: N_parameters
    double precision, intent(in) :: state_vector(N_parameters)
    double precision, intent(out) :: f
    integer, intent(out) :: failed
    type(mango_problem), value, intent(in) :: this
  end subroutine objective_function_interface
  subroutine residual_function_interface(N_parameters, state_vector, N_terms, f, failed, this)
    import
    integer, intent(in) :: N_parameters, N_terms
    double precision, intent(in) :: state_vector(N_parameters)
    !double precision, intent(in) :: state_vector(:)
    double precision, intent(out) :: f(N_terms)
    integer, intent(out) :: failed
    type(mango_problem), value, intent(in) :: this
  end subroutine residual_function_interface
  end interface

contains

!  subroutine mango_problem_create(this, N_parameters)
!    type(mango_problem), intent(out) :: this
!    integer :: N_parameters
!    this%object = C_mango_problem_create(N_parameters)
!  end subroutine mango_problem_create
!
!  subroutine mango_problem_create_least_squares(this, N_parameters, N_terms)
!    type(mango_problem), intent(out) :: this
!    integer :: N_parameters, N_terms
!    this%object = C_mango_problem_create(N_parameters)
!  end subroutine mango_problem_create_least_squares

  subroutine mango_problem_create(this, N_parameters, state_vector, dummy, objective_function)
    type(mango_problem), intent(out) :: this
    integer, intent(in) :: N_parameters, dummy
    !double precision, intent(in) :: state_vector(:)
    real(C_double), intent(in) :: state_vector(:)
    double precision, dimension(2) :: x = (/ 3.0, 4.0 /)
    double precision :: f
    real(C_double) :: state_vector_copy(N_parameters)
    procedure(objective_function_interface) :: objective_function
    integer :: failed_temp = 0
    !external objective_function

    if (size(state_vector) .ne. N_parameters) then
       print *,"Error! N_parameters does not equal size(state_vector)."
       print *,"N_parameters:",N_parameters," size(state_vector):",size(state_vector)
       stop
    end if
    state_vector_copy = state_vector

    !this%object = C_mango_problem_create(int(N_parameters,C_int), real(state_vector,C_double), C_funloc(objective_function))
    !this%object = C_mango_problem_create(int(N_parameters,C_int), c_loc(state_vector(1)), C_funloc(objective_function))
    !this%object = C_mango_problem_create(int(N_parameters,C_int), state_vector_copy(1), int(dummy,C_int), C_funloc(objective_function))
    this%object = C_mango_problem_create(int(N_parameters,C_int), state_vector(1), int(dummy,C_int), C_funloc(objective_function))

    ! For info on passing function pointers, see
    ! https://gcc.gnu.org/onlinedocs/gcc-4.6.1/gfortran/C_005fFUNLOC.html#C_005fFUNLOC
    !print *,"mango.F90 subroutine mango_problem_create: C_funloc(objective_function)=",C_funloc(objective_function)
    print *,"state_vector size in mango.F90:",size(state_vector_copy)
    print *,"state_vector in mango.F90:",state_vector_copy
    call objective_function(2, x, f, failed_temp, this)
    print *,"Done calling objective fn from mango.F90. f=",f
  end subroutine mango_problem_create

  subroutine mango_problem_create_least_squares(this, N_parameters, state_vector, N_terms, targets, sigmas, residual_function)
    type(mango_problem), intent(out) :: this
    integer, intent(in) :: N_parameters, N_terms
    real(C_double), intent(in) :: state_vector(:), targets(:), sigmas(:)
    procedure(residual_function_interface) :: residual_function
    this%object = C_mango_problem_create_least_squares(int(N_parameters,C_int), state_vector(1), int(N_terms,C_int), targets(1), sigmas(1), C_funloc(residual_function))
  end subroutine mango_problem_create_least_squares

  subroutine mango_problem_destroy(this)
    type(mango_problem), intent(inout) :: this
    call C_mango_problem_destroy(this%object)
    this%object = C_NULL_ptr
  end subroutine mango_problem_destroy

  subroutine mango_set_algorithm(this,algorithm)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: algorithm
    call C_mango_set_algorithm(this%object, int(algorithm,C_int))
  end subroutine mango_set_algorithm

  subroutine mango_set_algorithm_from_string(this,algorithm_str)
    ! Passing strings between fortran and C is fraught, so I opted to pass fixed-size character arrays instead.
    type(mango_problem), intent(in) :: this
    character(len=*), intent(in) :: algorithm_str
    character(C_char) :: algorithm_str_padded(mango_interface_string_length)
    integer :: j
    algorithm_str_padded = char(0);
    if (len(algorithm_str) > mango_interface_string_length-1) stop "String is too long!" ! -1 because C expects strings to be terminated with char(0);
    do j = 1, len(algorithm_str)
       algorithm_str_padded(j) = algorithm_str(j:j)
    end do
    call C_mango_set_algorithm_from_string(this%object, algorithm_str_padded)
  end subroutine mango_set_algorithm_from_string

  subroutine mango_read_input_file(this,filename)
    ! Passing strings between fortran and C is fraught, so I opted to pass fixed-size character arrays instead.
    type(mango_problem), intent(in) :: this
    character(len=*), intent(in) :: filename
    character(C_char) :: filename_padded(mango_interface_string_length)
    integer :: j
    filename_padded = char(0);
    if (len(filename) > mango_interface_string_length-1) stop "String is too long!" ! -1 because C expects strings to be terminated with char(0);
    do j = 1, len(filename)
       filename_padded(j) = filename(j:j)
    end do
    call C_mango_read_input_file(this%object, filename_padded)
  end subroutine mango_read_input_file

  subroutine mango_set_output_filename(this,filename)
    ! Passing strings between fortran and C is fraught, so I opted to pass fixed-size character arrays instead.
    type(mango_problem), intent(in) :: this
    character(len=*), intent(in) :: filename
    character(C_char) :: filename_padded(mango_interface_string_length)
    integer :: j
    filename_padded = char(0);
    if (len(filename) > mango_interface_string_length-1) stop "String is too long!" ! -1 because C expects strings to be terminated with char(0);
    do j = 1, len(filename)
       filename_padded(j) = filename(j:j)
    end do
    call C_mango_set_output_filename(this%object, filename_padded)
  end subroutine mango_set_output_filename

  subroutine mango_mpi_init(this,mpi_comm)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: mpi_comm
    call C_mango_mpi_init(this%object, int(mpi_comm,C_int))
  end subroutine mango_mpi_init

  subroutine mango_optimize(this)
    type(mango_problem), intent(in) :: this
    call C_mango_optimize(this%object)
  end subroutine mango_optimize

  integer function mango_get_mpi_rank_world(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_rank_world = C_mango_get_mpi_rank_world(this%object)
  end function mango_get_mpi_rank_world

  integer function mango_get_mpi_rank_worker_groups(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_rank_worker_groups = C_mango_get_mpi_rank_worker_groups(this%object)
  end function mango_get_mpi_rank_worker_groups

  integer function mango_get_mpi_rank_group_leaders(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_rank_group_leaders = C_mango_get_mpi_rank_group_leaders(this%object)
  end function mango_get_mpi_rank_group_leaders

  integer function mango_get_N_procs_world(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_procs_world = C_mango_get_N_procs_world(this%object)
  end function mango_get_N_procs_world

  integer function mango_get_N_procs_worker_groups(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_procs_worker_groups = C_mango_get_N_procs_worker_groups(this%object)
  end function mango_get_N_procs_worker_groups

  integer function mango_get_N_procs_group_leaders(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_procs_group_leaders = C_mango_get_N_procs_group_leaders(this%object)
  end function mango_get_N_procs_group_leaders

  logical function mango_is_proc0_world(this)
    type(mango_problem), intent(in) :: this
    integer :: result
    result = C_mango_is_proc0_world(this%object)
    if (result == 0) then
       mango_is_proc0_world = .false.
    elseif (result == 1) then
       mango_is_proc0_world = .true.
    else
       stop "Error in mango_is_proc0_world"
    end if
  end function mango_is_proc0_world

  logical function mango_is_proc0_worker_groups(this)
    type(mango_problem), intent(in) :: this
    integer :: result
    result = C_mango_is_proc0_worker_groups(this%object)
    if (result == 0) then
       mango_is_proc0_worker_groups = .false.
    elseif (result == 1) then
       mango_is_proc0_worker_groups = .true.
    else
       stop "Error in mango_is_proc0_worker_groups"
    end if
  end function mango_is_proc0_worker_groups

  integer function mango_get_mpi_comm_world(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_comm_world = C_mango_get_mpi_comm_world(this%object)
  end function mango_get_mpi_comm_world

  integer function mango_get_mpi_comm_worker_groups(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_comm_worker_groups = C_mango_get_mpi_comm_worker_groups(this%object)
  end function mango_get_mpi_comm_worker_groups

  integer function mango_get_mpi_comm_group_leaders(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_comm_group_leaders = C_mango_get_mpi_comm_group_leaders(this%object)
  end function mango_get_mpi_comm_group_leaders

  subroutine mango_set_centered_differences(this, centered_differences)
    type(mango_problem), intent(in) :: this
    logical, intent(in) :: centered_differences
    integer(C_int) :: logical_to_int
    logical_to_int = 0
    if (centered_differences) logical_to_int = 1
    call C_mango_set_centered_differences(this%object, logical_to_int)
  end subroutine mango_set_centered_differences

end module mango
