module mango

#define mango_interface_string_length 128

  ! Modeled after
  ! https://modelingguru.nasa.gov/docs/DOC-2642
  ! http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects

  use, intrinsic :: ISO_C_Binding, only: C_int, C_ptr, C_NULL_ptr
  implicit none
  private
  
  type mango_problem
     private
     type(C_ptr) :: object = C_NULL_ptr
  end type mango_problem

  interface
     function C_mango_problem_create() result(this) bind(C,name="mango_problem_create")
       import
       type(C_ptr) :: this
     end function C_mango_problem_create
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
       character :: algorithm_str(mango_interface_string_length)
     end subroutine C_mango_set_algorithm_from_string
     subroutine C_mango_read_input_file(this, filename) bind(C,name="mango_read_input_file")
       import
       type(C_ptr), value :: this
       character :: filename(mango_interface_string_length)
     end subroutine C_mango_read_input_file
     subroutine C_mango_mpi_init (this, mpi_comm) bind(C,name="mango_mpi_init")
       import
       integer(C_int) :: mpi_comm
       type(C_ptr), value :: this
     end subroutine C_mango_mpi_init
  end interface

  public :: mango_problem
  public :: mango_problem_create, mango_problem_destroy, mango_set_algorithm, mango_set_algorithm_from_string, mango_read_input_file, mango_mpi_init
  
contains

  subroutine mango_problem_create(this)
    type(mango_problem), intent(out) :: this
    this%object = C_mango_problem_create()
  end subroutine mango_problem_create

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
    character :: algorithm_str_padded(mango_interface_string_length)
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
    character :: filename_padded(mango_interface_string_length)
    integer :: j
    filename_padded = char(0);
    if (len(filename) > mango_interface_string_length-1) stop "String is too long!" ! -1 because C expects strings to be terminated with char(0);
    do j = 1, len(filename)
       filename_padded(j) = filename(j:j)
    end do
    call C_mango_read_input_file(this%object, filename_padded)
  end subroutine mango_read_input_file

  subroutine mango_mpi_init(this,mpi_comm)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: mpi_comm
    call C_mango_mpi_init(this%object, int(mpi_comm,C_int))
  end subroutine mango_mpi_init

end module mango
