module mango

#define mango_interface_string_length 128

  ! Modeled after
  ! https://modelingguru.nasa.gov/docs/DOC-2642
  ! http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects

  !use, intrinsic :: ISO_C_Binding, only: C_int, C_ptr, C_NULL_ptr, C_double
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
       type(C_funptr), value :: objective_function
     end function C_mango_problem_create
     function C_mango_problem_create_least_squares(N_parameters, state_vector, N_terms, targets, sigmas) result(this) bind(C,name="mango_problem_create_least_squares")
       import
       integer(C_int) :: N_parameters, N_terms
       type(C_ptr) :: this
       real(C_double) :: state_vector(:), targets(:), sigmas(:)
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
  end interface

  public :: mango_problem
  public :: mango_problem_create, mango_problem_create_least_squares, mango_problem_destroy, &
       mango_set_algorithm, mango_set_algorithm_from_string, mango_read_input_file, mango_set_output_filename, &
       mango_mpi_init, mango_optimize
  

  abstract interface
  subroutine objective_function_interface(N_parameters, state_vector, f, failed)
    integer, intent(in) :: N_parameters
    double precision, intent(in) :: state_vector(N_parameters)
    !double precision, intent(in) :: state_vector(:)
    double precision, intent(out) :: f
    integer, intent(out) :: failed
  end subroutine objective_function_interface
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

    ! For info on passing function pointers, see
    ! https://gcc.gnu.org/onlinedocs/gcc-4.6.1/gfortran/C_005fFUNLOC.html#C_005fFUNLOC
    !print *,"mango.F90 subroutine mango_problem_create: C_funloc(objective_function)=",C_funloc(objective_function)
    print *,"state_vector size in mango.F90:",size(state_vector_copy)
    print *,"state_vector in mango.F90:",state_vector_copy
    call objective_function(2, x, f, failed_temp)
    print *,"Done calling objective fn from mango.F90. f=",f
    !this%object = C_mango_problem_create(int(N_parameters,C_int), real(state_vector,C_double), C_funloc(objective_function))
    !this%object = C_mango_problem_create(int(N_parameters,C_int), c_loc(state_vector(1)), C_funloc(objective_function))
    !this%object = C_mango_problem_create(int(N_parameters,C_int), state_vector_copy(1), int(dummy,C_int), C_funloc(objective_function))
    this%object = C_mango_problem_create(int(N_parameters,C_int), state_vector(1), int(dummy,C_int), C_funloc(objective_function))
  end subroutine mango_problem_create

  subroutine mango_problem_create_least_squares(this, N_parameters, state_vector, N_terms, targets, sigmas)
    type(mango_problem), intent(out) :: this
    integer :: N_parameters, N_terms
    double precision :: state_vector(:), targets(:), sigmas(:)
    this%object = C_mango_problem_create_least_squares(int(N_parameters,C_int), real(state_vector,C_double), int(N_terms,C_int), real(targets,C_double), real(sigmas,C_double))
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

end module mango
