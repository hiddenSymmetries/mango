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

! If the module name is "mango", doxygen gets confused and combines the contents with the "mango" C++ namespace.
! Therefore I'll use a longer name for the module name.

!> This Fortran module provides the Fortran API for calling MANGO from outside applications.
module mango_mod

!  The value in the next line must match the corresponding value in mango.F90
#define mango_interface_string_length 256

  ! Modeled after
  ! https://modelingguru.nasa.gov/docs/DOC-2642
  ! http://fortranwiki.org/fortran/show/Fortran+and+Cpp+objects

  use, intrinsic :: ISO_C_Binding
  implicit none
  private

  !> An object that represents an optimization problem.
  type mango_problem
     private
     type(C_ptr) :: object = C_NULL_ptr !< This pointer points to a C++ mango::Problem object.
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
     function C_mango_problem_create_least_squares(N_parameters, state_vector, N_terms, targets, sigmas, best_residual_function, residual_function) result(this) bind(C,name="mango_problem_create_least_squares")
       import
       integer(C_int) :: N_parameters, N_terms
       type(C_ptr) :: this
       real(C_double) :: state_vector, targets, sigmas, best_residual_function
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
     subroutine C_mango_mpi_partition_set_custom(this, comm1, comm2, comm3) bind(C,name="mango_mpi_partition_set_custom")
       import
       integer(C_int) :: comm1, comm2, comm3
       type(C_ptr), value :: this
     end subroutine C_mango_mpi_partition_set_custom
     function C_mango_optimize(this) result(optimum) bind(C,name="mango_optimize")
       import
       type(C_ptr), value :: this
       real(C_double) :: optimum
     end function C_mango_optimize
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
     function C_mango_get_proc0_world(this) result(proc0) bind(C,name="mango_get_proc0_world")
       import
       integer(C_int) :: proc0
       type(C_ptr), value :: this
     end function C_mango_get_proc0_world
     function C_mango_get_proc0_worker_groups(this) result(proc0) bind(C,name="mango_get_proc0_worker_groups")
       import
       integer(C_int) :: proc0
       type(C_ptr), value :: this
     end function C_mango_get_proc0_worker_groups
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
     function C_mango_get_N_parameters(this) result(N) bind(C,name="mango_get_N_parameters")
       import
       integer(C_int) :: N
       type(C_ptr), value :: this
     end function C_mango_get_N_parameters
     function C_mango_get_N_terms(this) result(N) bind(C,name="mango_get_N_terms")
       import
       integer(C_int) :: N
       type(C_ptr), value :: this
     end function C_mango_get_N_terms
     function C_mango_get_worker_group(this) result(N) bind(C,name="mango_get_worker_group")
       import
       integer(C_int) :: N
       type(C_ptr), value :: this
     end function C_mango_get_worker_group
     function C_mango_get_best_function_evaluation(this) result(N) bind(C,name="mango_get_best_function_evaluation")
       import
       integer(C_int) :: N
       type(C_ptr), value :: this
     end function C_mango_get_best_function_evaluation
     function C_mango_get_function_evaluations(this) result(N) bind(C,name="mango_get_function_evaluations")
       import
       integer(C_int) :: N
       type(C_ptr), value :: this
     end function C_mango_get_function_evaluations
     subroutine C_mango_set_max_function_evaluations(this, N) bind(C,name="mango_set_max_function_evaluations")
       import
       type(C_ptr), value :: this
       integer(C_int) :: N
     end subroutine C_mango_set_max_function_evaluations
     subroutine C_mango_set_centered_differences(this, centered_differences_int) bind(C,name="mango_set_centered_differences")
       import
       type(C_ptr), value :: this
       integer(C_int) :: centered_differences_int
     end subroutine C_mango_set_centered_differences
     function C_mango_does_algorithm_exist(algorithm_str) result(temp_int) bind(C,name="mango_does_algorithm_exist")
       import
       character(C_char) :: algorithm_str(mango_interface_string_length)
       integer(C_int) :: temp_int
     end function C_mango_does_algorithm_exist
     subroutine C_mango_set_finite_difference_step_size (this, step) bind(C,name="mango_set_finite_difference_step_size")
       import
       real(C_double) :: step
       type(C_ptr), value :: this
     end subroutine C_mango_set_finite_difference_step_size
     subroutine C_mango_set_bound_constraints(this, lower_bounds, upper_bounds) bind(C,name="mango_set_bound_constraints")
       import
       real(C_double) :: lower_bounds, upper_bounds
       type(C_ptr), value :: this
     end subroutine C_mango_set_bound_constraints
     subroutine C_mango_set_verbose (this, verbose) bind(C,name="mango_set_verbose")
       import
       integer(C_int) :: verbose
       type(C_ptr), value :: this
     end subroutine C_mango_set_verbose
     subroutine C_mango_set_print_residuals_in_output_file(this, print_residuals_in_output_file_int) bind(C,name="mango_set_print_residuals_in_output_file")
       import
       type(C_ptr), value :: this
       integer(C_int) :: print_residuals_in_output_file_int
     end subroutine C_mango_set_print_residuals_in_output_file
     subroutine C_mango_set_user_data(this, user_data) bind(C,name="mango_set_user_data")
       import
       type(C_ptr), value :: this, user_data
     end subroutine C_mango_set_user_data
     subroutine C_mango_stop_workers(this) bind(C,name="mango_stop_workers")
       import
       type(C_ptr), value :: this
     end subroutine C_mango_stop_workers
     subroutine C_mango_mobilize_workers(this) bind(C,name="mango_mobilize_workers")
       import
       type(C_ptr), value :: this
     end subroutine C_mango_mobilize_workers
     function C_mango_continue_worker_loop(this) result(N) bind(C,name="mango_continue_worker_loop")
       import
       integer(C_int) :: N
       type(C_ptr), value :: this
     end function C_mango_continue_worker_loop
     subroutine C_mango_mpi_partition_write(this, filename) bind(C,name="mango_mpi_partition_write")
       import
       type(C_ptr), value :: this
       character(C_char) :: filename(mango_interface_string_length)
     end subroutine C_mango_mpi_partition_write
     subroutine C_mango_set_relative_bound_constraints(this, min_factor, max_factor, min_radius, preserve_sign) bind(C,name="mango_set_relative_bound_constraints")
       import
       type(C_ptr), value :: this
       real(C_double) :: min_factor, max_factor, min_radius
       integer(C_int) :: preserve_sign
     end subroutine C_mango_set_relative_bound_constraints
     subroutine C_mango_set_N_line_search (this, N) bind(C,name="mango_set_N_line_search")
       import
       integer(C_int) :: N
       type(C_ptr), value :: this
     end subroutine C_mango_set_N_line_search
  end interface

  public :: mango_problem
  public :: mango_problem_create, mango_problem_create_least_squares, &
       mango_problem_destroy, &
       mango_set_algorithm, mango_set_algorithm_from_string, mango_read_input_file, mango_set_output_filename, &
       mango_mpi_init, mango_mpi_partition_set_custom, mango_optimize, &
       mango_get_mpi_rank_world, mango_get_mpi_rank_worker_groups, mango_get_mpi_rank_group_leaders, &
       mango_get_N_procs_world, mango_get_N_procs_worker_groups, mango_get_N_procs_group_leaders, &
       mango_get_proc0_world, mango_get_proc0_worker_groups, &
       mango_get_mpi_comm_world, mango_get_mpi_comm_worker_groups, mango_get_mpi_comm_group_leaders, &
       mango_get_N_parameters, mango_get_N_terms, &
       mango_get_worker_group, mango_get_best_function_evaluation, &
       mango_get_function_evaluations, mango_set_max_function_evaluations, mango_set_centered_differences, &
       mango_does_algorithm_exist, mango_set_finite_difference_step_size, mango_set_bound_constraints, &
       mango_set_verbose, mango_set_print_residuals_in_output_file, &
       mango_set_user_data, &
       mango_stop_workers, mango_mobilize_workers, mango_continue_worker_loop, mango_mpi_partition_write, &
       mango_set_relative_bound_constraints, mango_set_N_line_search
  

  abstract interface
  !> Blah blah
  subroutine objective_function_interface(N_parameters, state_vector, f, failed, this, user_data)
    import
    integer(C_int), intent(in) :: N_parameters
    real(C_double), intent(in) :: state_vector(N_parameters)
    real(C_double), intent(out) :: f
    integer(C_int), intent(out) :: failed
    type(mango_problem), value, intent(in) :: this
    type(C_ptr), value, intent(in) :: user_data
  end subroutine objective_function_interface
  subroutine vector_function_interface(N_parameters, state_vector, N_terms, f, failed, this, user_data)
    import
    integer(C_int), intent(in) :: N_parameters, N_terms
    real(C_double), intent(in) :: state_vector(N_parameters)
    !double precision, intent(in) :: state_vector(:)
    real(C_double), intent(out) :: f(N_terms)
    integer(C_int), intent(out) :: failed
    type(mango_problem), value, intent(in) :: this
    type(C_ptr), value, intent(in) :: user_data
  end subroutine vector_function_interface
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

  !> Create a standard (i.e. non-least-squares) optimization problem.
  !> @param this  An object storing the created optimization problem.
  !> @param N_parameters  The number of independent variables.
  !> @param state_vector An array of size N_parameters, which will be used to store the initial condition.
  !>                     You can set the values of this array either before or after calling this routine,
  !>                     as long as they are set before calling \ref mango_optimize.
  !> @param dummy This should be removed.
  !> @param objective_function  A reference to the objective function that will be minimized.
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
    !print *,"state_vector size in mango.F90:",size(state_vector_copy)
    !print *,"state_vector in mango.F90:",state_vector_copy
    !call objective_function(2, x, f, failed_temp, this)
    !print *,"Done calling objective fn from mango.F90. f=",f
  end subroutine mango_problem_create

  !> Create a least-squares optimization problem.
  !> @param this  An object storing the created optimization problem.
  !> @param N_parameters  The number of independent variables.
  !> @param state_vector An array of size N_parameters, which will be used to store the initial condition.
  !>                     You can set the values of this array either before or after calling this routine,
  !>                     as long as they are set before calling \ref mango_optimize.
  !> @param N_terms  The number of terms that are summed in the objective function, i.e. the number of elements in the residual vector.
  !> @param targets  An array of size N_terms, storing the target values of each residual function.
  !> @param sigmas   An array of size N_terms, storing the scaling factors for each term in the residual function.
  !> @param best_residual_function An array of size N_terms. When \ref mango_optimize concludes successfully, this array will store
  !>                               the values of the residuals for the optimum point.
  !> @param residual_function  A reference to the subroutine that computes the residuals.
  subroutine mango_problem_create_least_squares(this, N_parameters, state_vector, N_terms, targets, sigmas, best_residual_function, residual_function)
    type(mango_problem), intent(out) :: this
    integer, intent(in) :: N_parameters, N_terms
    real(C_double), intent(in) :: state_vector(:), targets(:), sigmas(:), best_residual_function(:)
    procedure(vector_function_interface) :: residual_function
    this%object = C_mango_problem_create_least_squares(int(N_parameters,C_int), state_vector(1), int(N_terms,C_int), targets(1), sigmas(1), best_residual_function(1), C_funloc(residual_function))
  end subroutine mango_problem_create_least_squares

  !> Delete an optimization problem, freeing memory used internally by MANGO.
  !> @param this The optimization problem to destroy.
  subroutine mango_problem_destroy(this)
    type(mango_problem), intent(inout) :: this
    call C_mango_problem_destroy(this%object)
    this%object = C_NULL_ptr
  end subroutine mango_problem_destroy

  !> Sets the optimization algorithm
  !>
  !> Note the related subroutine \ref mango_set_algorithm_from_string.
  !> @param this The optimization problem to modify.
  !> @param algorithm One of the enumerated constants representing optimization algorithms.
  !> If the integer supplied is too large or too small, a C++ exception will be thrown.
  subroutine mango_set_algorithm(this,algorithm)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: algorithm
    call C_mango_set_algorithm(this%object, int(algorithm,C_int))
  end subroutine mango_set_algorithm

  !> Sets the optimization algorithm
  !>
  !> Note the related subroutine \ref mango_set_algorithm.
  !> @param this The optimization problem to modify.
  !> @param algorithm_str A lowercase string containing one of the available algorithms, e.g. "petsc_nm".
  !> If the string supplied does not match any of the available algorithms, a C++ exception will be thrown.
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

  !> Reads in the number of worker groups and algorithm from a file.
  !>
  !> This subroutine is used in the examples, so the testing framework can vary the number of worker groups and optimization algorithm.
  !> @param this  The optimization problem.
  !> @param filename The filename of the file to read.
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

  !> Sets the name of the output file that will record values of the objective function at each evaluation.
  !>
  !> @param this The optimization problem
  !> @param filename A string giving the filename to use for the output file. If the file already exists, it will be over-written.
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

  !> Initialize MANGO's internal MPI data that describes the partitioning of the processes into worker groups.
  !>
  !> This subroutine divides up the available MPI processes into worker groups, after checking to see
  !> if the selected optimization algorithm supports concurrent function evaluations.
  !> Do not confuse this subroutine with MPI_Init, the routine from MPI that initializes MPI itself!
  !> mango_mpi_init must be called by the driver code after the call to MPI_Init.
  !> This subroutine should be called after setting N_worker_groups and setting the optimization algorithm.
  !> This way, MANGO can change N_worker_groups to 1 if an algorithm is chosen that does not support concurrent function evaluations.
  !> @param this  The optimization problem.
  !> @param mpi_comm  The MPI communicator to use for the optimization. Usually this is MPI_COMM_WORLD. However,
  !>    if you want to run the optimization on a subset of MPI_COMM_WORLD, you can supply the appropriate communicator instead.
  subroutine mango_mpi_init(this,mpi_comm)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: mpi_comm
    call C_mango_mpi_init(this%object, int(mpi_comm,C_int))
  end subroutine mango_mpi_init

  !> Use a user-supplied partitioning of the MPI processes into worker groups.
  !>
  !> Use either this subroutine or \ref mango_mpi_init, not both.
  !> @param this  The optimization problem for which you want to set the MPI structures.
  !> @param comm_world  An MPI communicator consisting of all the processors that will participate in the optimization in any way.
  !> @param comm_group_leaders  An MPI communicator consisting only of the group leaders.
  !> @param comm_worker_groups  An MPI communicator containing all the processors of comm_world, but with a separate "color" for each worker group. The processes with rank 0 in this communicator must be the same as the processes in comm_group_leaders.
  subroutine mango_mpi_partition_set_custom(this, comm_world, comm_group_leaders, comm_worker_groups)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: comm_world, comm_group_leaders, comm_worker_groups
    call C_mango_mpi_partition_set_custom(this%object, int(comm_world,C_int), int(comm_group_leaders,C_int), int(comm_worker_groups,C_int))
  end subroutine mango_mpi_partition_set_custom

  !> Carry out the optimization.
  !>
  !> This is the main computationally demanding step.
  !> @param this The optimization problem.
  !> @return The minimum value found for the objective function.
  double precision function mango_optimize(this)
    type(mango_problem), intent(in) :: this
    mango_optimize = C_mango_optimize(this%object)
  end function mango_optimize

  !> Get the MPI rank of this processor in MANGO's world communicator.
  !>
  !> @param this The optimization problem.
  !> @return The MPI rank of this processor in MANGO's world communicator.
  integer function mango_get_mpi_rank_world(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_rank_world = C_mango_get_mpi_rank_world(this%object)
  end function mango_get_mpi_rank_world

  !> Get the MPI rank of this processor in MANGO's "worker groups" communicator.
  !>
  !> @param this The optimization problem.
  !> @return The MPI rank of this processor in MANGO's "worker groups" communicator.
  integer function mango_get_mpi_rank_worker_groups(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_rank_worker_groups = C_mango_get_mpi_rank_worker_groups(this%object)
  end function mango_get_mpi_rank_worker_groups

  !> Get the MPI rank of this processor in MANGO's "group leaders" communicator.
  !>
  !> @param this The optimization problem.
  !> @return The MPI rank of this processor in MANGO's "group leaders" communicator.
  !>   A value of -1 is returned on any processor that is not a group leader.
  integer function mango_get_mpi_rank_group_leaders(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_rank_group_leaders = C_mango_get_mpi_rank_group_leaders(this%object)
  end function mango_get_mpi_rank_group_leaders

  !> Get the number of MPI processors in MANGO's world communicator.
  !>
  !> @param this The optimization problem.
  !> @return The number of MPI processors in MANGO's world communicator.
  integer function mango_get_N_procs_world(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_procs_world = C_mango_get_N_procs_world(this%object)
  end function mango_get_N_procs_world

  !> Get the number of MPI processors in the worker group that this processor belongs to.
  !>
  !> @param this The optimization problem.
  !> @return The number of MPI processors in the worker group that this processor belongs to.
  integer function mango_get_N_procs_worker_groups(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_procs_worker_groups = C_mango_get_N_procs_worker_groups(this%object)
  end function mango_get_N_procs_worker_groups

  !> Get the number of MPI processors in the "group leaders" communicator.
  !>
  !> @param this The optimization problem.
  !> @return The number of MPI processors in the "group leaders" communicator.
  !>   On a processor that is a group leader, this number will be the same as the number of worker groups.
  !>   On a processor that is not a group leader, this function will return -1.
  integer function mango_get_N_procs_group_leaders(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_procs_group_leaders = C_mango_get_N_procs_group_leaders(this%object)
  end function mango_get_N_procs_group_leaders

  !> Determine whether this MPI processor has rank=0 in MANGO's world communicator.
  !>
  !> @param this The optimization problem.
  !> @return .true. if this MPI processor has rank=0 in MANGO's world communicator, .false. otherwise.
  logical function mango_get_proc0_world(this)
    type(mango_problem), intent(in) :: this
    integer :: result
    result = C_mango_get_proc0_world(this%object)
    if (result == 0) then
       mango_get_proc0_world = .false.
    elseif (result == 1) then
       mango_get_proc0_world = .true.
    else
       stop "Error in mango_get_proc0_world"
    end if
  end function mango_get_proc0_world

  !> Determine whether this MPI processor has rank=0 in MANGO's "worker groups" communicator.
  !>
  !> @param this The optimization problem.
  !> @return .true. if this MPI processor has rank=0 in MANGO's "worker groups" communicator, .false. otherwise.
  !>    In other words, the return value is .true. for group leaders.
  logical function mango_get_proc0_worker_groups(this)
    type(mango_problem), intent(in) :: this
    integer :: result
    result = C_mango_get_proc0_worker_groups(this%object)
    if (result == 0) then
       mango_get_proc0_worker_groups = .false.
    elseif (result == 1) then
       mango_get_proc0_worker_groups = .true.
    else
       stop "Error in mango_get_proc0_worker_groups"
    end if
  end function mango_get_proc0_worker_groups

  !> Get MANGO's "world" MPI communicator.
  !>
  !> @param this The optimization problem.
  !> @return MANGO's "world" MPI communicator.
  integer function mango_get_mpi_comm_world(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_comm_world = C_mango_get_mpi_comm_world(this%object)
  end function mango_get_mpi_comm_world

  !> Get MANGO's "worker groups" MPI communicator.
  !>
  !> @param this The optimization problem.
  !> @return MANGO's "worker groups" MPI communicator.
  integer function mango_get_mpi_comm_worker_groups(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_comm_worker_groups = C_mango_get_mpi_comm_worker_groups(this%object)
  end function mango_get_mpi_comm_worker_groups

  !> Get MANGO's "group leaders" MPI communicator.
  !>
  !> @param this The optimization problem.
  !> @return MANGO's "group leaders" MPI communicator.
  integer function mango_get_mpi_comm_group_leaders(this)
    type(mango_problem), intent(in) :: this
    mango_get_mpi_comm_group_leaders = C_mango_get_mpi_comm_group_leaders(this%object)
  end function mango_get_mpi_comm_group_leaders

  !> Get the number of independent variables for an optimization problem.
  !> @param this   The mango_problem object to query.
  !> @return       The number of independent variables, i.e. the dimensionality of the parameter space.
  integer function mango_get_N_parameters(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_parameters = C_mango_get_N_parameters(this%object)
  end function mango_get_N_parameters

  !> For least-squares optimization problems, get the number of terms that are summed in the objective function.
  !> @param this   The mango_problem object to query.
  !> @return       The number of terms that are summed in the objective function.
  integer function mango_get_N_terms(this)
    type(mango_problem), intent(in) :: this
    mango_get_N_terms = C_mango_get_N_terms(this%object)
  end function mango_get_N_terms

  !> Determine which worker group this MPI process belongs to.
  !> @param this   The mango_problem object to query.
  !> @return       An integer indicating the worker group to which this MPI process belongs.
  integer function mango_get_worker_group(this)
    type(mango_problem), intent(in) :: this
    mango_get_worker_group = C_mango_get_worker_group(this%object)
  end function mango_get_worker_group

  !> For an optimization problem that has already been solved, return the index of the function evaluation corresponding to the optimum.
  !>
  !> @param this The optimization problem.
  !> @return The index of the function evaluation corresponding to the optimum.
  !>  If \ref mango_optimize has not yet been called, a value of -1 will be returned.
  integer function mango_get_best_function_evaluation(this)
    type(mango_problem), intent(in) :: this
    mango_get_best_function_evaluation = C_mango_get_best_function_evaluation(this%object)
  end function mango_get_best_function_evaluation

  !> For an optimization problem that has already been solved, return the number of times the objective function was evaluated.
  !>
  !> @param this The optimization problem.
  !> @return The number of times the objective function was evaluated.
  !>   If \ref mango_optimize has not yet been called, a value of 0 will be returned.
  integer function mango_get_function_evaluations(this)
    type(mango_problem), intent(in) :: this
    mango_get_function_evaluations = C_mango_get_function_evaluations(this%object)
  end function mango_get_function_evaluations

  !> Set the maximum number of evaluations of the objective function that will be allowed before the optimization is terminated.
  !>
  !> @param this The optimization problem.
  !> @param N The maximum number of evaluations of the objective function that will be allowed before the optimization is terminated.
  !>   If this number is less than 1, a C++ exception will be thrown.
  subroutine mango_set_max_function_evaluations(this, N)
    type(mango_problem), intent(in) :: this
    integer(C_int), intent(in) :: N
    call C_mango_set_max_function_evaluations(this%object, N)
  end subroutine mango_set_max_function_evaluations

  !> Control whether 1-sided or centered finite differences will be used to compute derivatives of the objective function.
  !>
  !> @param this The optimization problem.
  !> @param centered_differences If .true., centered differences will be used. If .false., 1-sided differences will be used.
  !>   Centered differences are more accurate, but require more function evaluations (2*N_parameters) compared to
  !>   1-sided differences (which require N_parameters+1 evaluations).
  subroutine mango_set_centered_differences(this, centered_differences)
    type(mango_problem), intent(in) :: this
    logical, intent(in) :: centered_differences
    integer(C_int) :: logical_to_int
    logical_to_int = 0
    if (centered_differences) logical_to_int = 1
    call C_mango_set_centered_differences(this%object, logical_to_int)
  end subroutine mango_set_centered_differences

  !> Determine whether MANGO has an optimization algorithm corresponding to the provided string.
  !>
  !> @param algorithm_str A string to examine.
  !> @return .true. if algorithm_str corresponds to one of the optimization algorithms known by MANGO, .false. otherwise.
  logical function mango_does_algorithm_exist(algorithm_str)
    ! Passing strings between fortran and C is fraught, so I opted to pass fixed-size character arrays instead.
    character(len=*), intent(in) :: algorithm_str
    character(C_char) :: algorithm_str_padded(mango_interface_string_length)
    integer :: j, result_int
    algorithm_str_padded = char(0);
    if (len(algorithm_str) > mango_interface_string_length-1) stop "String is too long!" ! -1 because C expects strings to be terminated with char(0);
    do j = 1, len(algorithm_str)
       algorithm_str_padded(j) = algorithm_str(j:j)
    end do
    mango_does_algorithm_exist = (C_mango_does_algorithm_exist(algorithm_str_padded) == 1)
  end function mango_does_algorithm_exist

  !> Set an absolute step size for finite difference derivatives.
  !>
  !> @param this The optimization problem
  !> @param finite_difference_step_size An absolute step size to use for finite difference derivatives.
  subroutine mango_set_finite_difference_step_size(this,finite_difference_step_size)
    type(mango_problem), intent(in) :: this
    double precision, intent(in) :: finite_difference_step_size
    call C_mango_set_finite_difference_step_size(this%object, real(finite_difference_step_size,C_double))
  end subroutine mango_set_finite_difference_step_size

  !> Impose bound constraints on an optimization problem.
  !>
  !> Note that not every optimization algorithm allows bound constraints. If bound constraints
  !> have been turned on with this subroutine and \ref mango_optimize is called,
  !> MANGO will check whether the selected algorithm supports bound constraints. If not,
  !> a warning message will be printed to stdout, and the optimization will proceed,
  !> ignoring the bound constraints.
  !> @param this The optimization problem to modify
  !> @param lower_bounds An array of size N_parameters, giving the lower bound for each independent variable.
  !> @param upper_bounds An array of size N_parameters, giving the upper bound for each independent variable.
  subroutine mango_set_bound_constraints(this, lower_bounds, upper_bounds)
    type(mango_problem), intent(in) :: this
    double precision, intent(in) :: lower_bounds(:), upper_bounds(:)
    call C_mango_set_bound_constraints(this%object, lower_bounds(1), upper_bounds(1))
  end subroutine mango_set_bound_constraints

  !> Control how much diagnostic information is printed by MANGO.
  !>
  !> This diagnostic information may be helpful for debugging.
  !> @param this The optimization problem to control
  !> @param verbose If <= 0, no diagnostic information will be printed. If >0, diagnostic information will be printed.
  subroutine mango_set_verbose(this, verbose)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: verbose
    call C_mango_set_verbose(this%object, verbose)
  end subroutine mango_set_verbose

  !> For least-squares problems, control whether or not to print each individual residual in the MANGO output file.
  !>
  !> You may wish to print this information to see the relative magnitude of each term
  !> in the objective function. On the other hand, for problems with many terms, you may
  !> wish to suppress this information to make the file more readable.
  !> @param this The optimization problem to control. If the problem is not a least-squares problem, 
  !>   something bad is likely to happen, like a segmentation fault.
  !> @param print_residuals_in_output_file Whether or not to print every residual term in the output file.
  subroutine mango_set_print_residuals_in_output_file(this, print_residuals_in_output_file)
    type(mango_problem), intent(in) :: this
    logical, intent(in) :: print_residuals_in_output_file
    integer(C_int) :: logical_to_int
    logical_to_int = 0
    if (print_residuals_in_output_file) logical_to_int = 1
    call C_mango_set_print_residuals_in_output_file(this%object, logical_to_int)
  end subroutine mango_set_print_residuals_in_output_file

  !> Pass a data structure to the objective function whenever it is called.
  !>
  !> @param this The optimization problem to modify.
  !> @param user_data A pointer to the variable or data structure you want to pass to the objective function.
  !>   To find this pointer, apply the C_LOC function to your variable. C_LOC is available in the standard ISO_C_BINDING module.
  subroutine mango_set_user_data(this, user_data)
    type(mango_problem), intent(in) :: this
    type(C_ptr), intent(in) :: user_data
    call C_mango_set_user_data(this%object, user_data)
  end subroutine mango_set_user_data

  !> Tell the worker MPI processes (i.e. those that are not group leaders) that the optimization problem is complete.
  !>
  !> This subroutine should only be called by group leaders.
  !> This subroutine is typically called immediately after \ref mango_optimize.
  !> You can see typical usage of this subroutine in the examples. However you are also free to
  !> use your own approach to stopping the worker processes instead of this subroutine.
  !> @param this The optimization problem.
  subroutine mango_stop_workers(this)
    type(mango_problem), intent(in) :: this
    call C_mango_stop_workers(this%object)
  end subroutine mango_stop_workers

  !> Tell the worker MPI processes (i.e. those that are not group leaders) to begin an evaluation of the objective function.
  !>
  !> This subroutine should only be called by group leaders.
  !> This subroutine is typically called at the beginning of the user-supplied subroutine for the objective function.
  !> You can see typical usage of this subroutine in the examples. However you are also free to
  !> use your own approach to controlling the worker processes instead of this subroutine.
  !> @param this The optimization problem.
  subroutine mango_mobilize_workers(this)
    type(mango_problem), intent(in) :: this
    call C_mango_mobilize_workers(this%object)
  end subroutine mango_mobilize_workers

  !> For an MPI worker, determine whether to carry out another evaluation of the objective function or exit.
  !>
  !> This subroutine should only be called on MPI processors that are not group leaders.
  !> You can see typical usage of this subroutine in the examples. However you are also free to
  !> use your own approach to controlling the worker processes instead of this subroutine.
  !> @param this The optimization problem.
  !> @return If .true., this processor should help to evaluate the objective function. If .false., the optimization has been completed,
  !>   so this processor can move on.
  logical function mango_continue_worker_loop(this)
    type(mango_problem), intent(in) :: this
    integer :: result
    result = C_mango_continue_worker_loop(this%object)
    if (result == 0) then
       mango_continue_worker_loop = .false.
    elseif (result == 1) then
       mango_continue_worker_loop = .true.
    else
       stop "Error in mango_continue_worker_loop"
    end if
  end function mango_continue_worker_loop

  !> Write a file showing the worker group assignments and rank of each process in each MPI communicator.
  !>
  !> @param this The optimization problem
  !> @param filename The name of the file to write. If this file already exists, it will be over-written.
  subroutine mango_mpi_partition_write(this,filename)
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
    call C_mango_mpi_partition_write(this%object, filename_padded)
  end subroutine mango_mpi_partition_write

  !> Impose bound constraints on an optimization problem, with the bounds chosen as multiples of the initial state vector.
  !>
  !> To use this subroutine, you must first call \ref mango_set_bound_constraints, so MANGO has pointers to the 
  !> arrays allocted by the user for lower and upper bounds. mango_set_relative_bound_constraints will overwrite the entries of these arrays.
  !>
  !> There are two possible methods of determining the bound constraints using this subroutine, depending
  !> on the parameter preserve_sign. 
  !>
  !> If preserve_sign is false, the bounds will be symmetric about 0, given by the value
  !> of the initial condition multiplied by max_factor, expanded to be at least min_radius different from
  !> the initial condition. Thus, for independent
  !> variable \f$ x_j \f$, the bounds will be \f$ x_j \in [-R_j, R_j] \f$, where
  !> \f$ R_j = \max(\mathtt{min\_radius}, |x_j| \mathtt{max\_factor})\f$. 
  !> Note that the parameter min_factor is not used in this case.
  !>
  !> If preserve_sign is true, the lower and upper bounds for a given independent variable
  !> will have the same sign as the corresponding element of state_vector (the initial condition supplied to the constructor).
  !> The bounds are determined by multiplying each independent variable by min_factor and max_factor, keeping
  !> both bounds at least min_radius from the initial condition (while not crossing 0).
  !> Thus, if independent variable \f$ x_j \f$ is positive, the bounds will be \f$ x_j \in [L_j, R_j] \f$ where
  !> \f$ L_j = \max(0, \min(x_j - \mathtt{min\_radius}, x_j \, \mathtt{min\_factor})) \f$ and
  !> \f$ R_j = \max(x_j + \mathtt{min\_radius}, x_j \, \mathtt{max\_factor}) \f$.
  !> If independent variable \f$ x_j \f$ is negative, the bounds will be \f$ x_j \in [L_j, R_j] \f$ where
  !> \f$ L_j = \min(x_j - \mathtt{min\_radius}, x_j \, \mathtt{max\_factor}) \f$ and
  !> \f$ R_j = \min(0, \max(x_j + \mathtt{min\_radius}, x_j \, \mathtt{min\_factor})) \f$.
  !> For the special case \f$x_j=0\f$, then the bounds are \f$ x_j \in [-\mathtt{min\_radius}, \mathtt{min\_radius}] \f$.
  !>
  !> Note that not every optimization algorithm allows bound constraints. If bound constraints
  !> have been turned on with this subroutine and \ref mango_optimize is called,
  !> MANGO will check whether the selected algorithm supports bound constraints. If not,
  !> a warning message will be printed to stdout, and the optimization will proceed,
  !> ignoring the bound constraints.
  !> @param this The optimization problem to modify
  !> @param min_factor See description above. The value must lie in [0,1] or else a C++ exception will be thrown.
  !> @param max_factor See description above. The value must be \f$ \ge 1 \f$, or else a C++ expection will be thrown.
  !> @param min_radius See description above. The value must be \f$ \ge 0 \f$, or else a C++ exception will be thrown.
  !> @param preserve_sign See description above.
  subroutine mango_set_relative_bound_constraints(this, min_factor, max_factor, min_radius, preserve_sign)
    type(mango_problem), intent(in) :: this
    real(C_double), intent(in) :: min_factor, max_factor, min_radius
    logical, intent(in) :: preserve_sign
    integer(C_int) :: preserve_sign_int
    preserve_sign_int = 0
    if (preserve_sign) preserve_sign_int = 1 
    call C_mango_set_relative_bound_constraints(this%object, min_factor, max_factor, min_radius, preserve_sign_int)
  end subroutine mango_set_relative_bound_constraints

  !> Sets the number of points considered as a set for parallel line searches
  !>
  !> The default value is 0.
  !> If the value is \f$<=\f$0, the number will be set to the number of worker groups.
  !> Normally this default makes sense.
  !> @param this The optimization problem to control
  !> @param N_line_search The number of points considered as a set for parallel line searches.
  subroutine mango_set_N_line_search(this, N_line_search)
    type(mango_problem), intent(in) :: this
    integer, intent(in) :: N_line_search
    call C_mango_set_N_line_search(this%object, N_line_search)
  end subroutine mango_set_N_line_search

end module mango_mod
