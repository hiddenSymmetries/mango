module mango

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
     subroutine C_mango_mpi_init (this, mpi_comm) bind(C,name="mango_mpi_init")
       import
       integer(C_int) :: mpi_comm
       type(C_ptr), value :: this
     end subroutine C_mango_mpi_init
  end interface

!  interface mango_problem_create
!     module procedure F_mango_problem_create
!  end interface
!  interface mango_problem_destroy
!     module procedure F_mango_problem_destroy
!  end interface
!  interface mango_set_algorithm
!     module procedure F_mango_set_algorithm
!  end interface
!  interface mango_mpi_init
!     module procedure F_mango_mpi_init
!  end interface

  public :: mango_problem
!  public :: create, destroy, set_algorithm, mpi_init
  public :: mango_problem_create, mango_problem_destroy, mango_set_algorithm, mango_mpi_init
  
contains

!  subroutine F_mango_problem_create(this)
!    type(mango_problem), intent(out) :: this
!    this%object = C_mango_problem_create()
!  end subroutine F_mango_problem_create
!  subroutine F_mango_problem_destroy(this)
!    type(mango_problem), intent(inout) :: this
!    call C_mango_problem_destroy(this%object)
!    this%object = C_NULL_ptr
!  end subroutine F_mango_problem_destroy
!  subroutine F_mango_set_algorithm(this,algorithm)
!    type(mango_problem), intent(in) :: this
!    integer, intent(in) :: algorithm
!    call C_mango_set_algorithm(this%object, int(algorithm,C_int))
!  end subroutine F_mango_set_algorithm
!  subroutine F_mango_mpi_init(this,mpi_comm)
!    type(mango_problem), intent(in) :: this
!    integer, intent(in) :: mpi_comm
!    call C_mango_mpi_init(this%object, int(mpi_comm,C_int))
!  end subroutine F_mango_mpi_init

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
  subroutine mango_mpi_init(this,mpi_comm)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: mpi_comm
    call C_mango_mpi_init(this%object, int(mpi_comm,C_int))
  end subroutine mango_mpi_init

end module mango
