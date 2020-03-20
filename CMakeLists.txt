#---- specify version of cmake (this version is random as of 02/19/20)
CMAKE_MINIMUM_REQUIRED (VERSION 3.8)
SET (CMAKE_CXX_COMPILER $ENV{CXX})
SET (CMAKE_C_COMPILER $ENV{CC})
#---- declaring this project name defines ${mango_source_dir} to be the path
#---- where this source file exists
PROJECT (MANGO CXX C Fortran)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MANGO_SOURCE_DIR})
#---- Flags to include MPI headers
#SET (CMAKE_C_FLAGS "-I $ENV{CRAY_MPICH_DIR}/include")
#SET (CMAKE_CXX_FLAGS "-I $ENV{CRAY_MPICH_DIR}/include")
#SET (CMAKE_Fortran_FLAGS "-I $ENV{CRAY_MPICH_DIR}/include")
#SET (MPI_LIB_DIR "-L $ENV{CRAY_MPICH_DIR}/lib/libmpichcxx.a -lfmpich -lmpichcxx -lmpl -lmpichf90")
#SET (MPI_C_HEADER_DIR $ENV{CRAY_MPICH_DIR}/include)

#SET (MPIEXEC_EXECUTABLE /usr/common/software/mpich/3.3.1-debug/gcc/7.3.0/bin/mpiexec)
#SET (MPIEXEC_EXECUTABLE /usr/common/software/mpich/3.3.2/cce/9.1.0/cgpu/bin/mpiexec)
#SET (MPIEXEC_EXECUTABLE ${CRAY_MPICH_DIR}/bin/mpiexec)
#FIND_PACKAGE (MPI REQUIRED)

INCLUDE_DIRECTORIES (SYSTEM ${MPI_C_INCLUDE_PATH})
INCLUDE_DIRECTORIES (SYSTEM ${MPI_CXX_INCLUDE_PATH})
INCLUDE_DIRECTORIES (SYSTEM ${MPI_Fortran_INCLUDE_PATH})
#---- the find_package command sets '<package>_found' to true if it's successful

#IF (NOT MPI_C_COMPILER)
#   IF (NOT MPI_C_FOUND)
#      MESSAGE (FATAL_ERROR "mango: unable to find MPI_C compiler")
#   ELSE ()
#      MESSAGE (FATAL_ERROR "Found MPI_C ??")
#   ENDIF ()
#ENDIF (NOT MPI_C_COMPILER)


#---- Empty vector to be filled with -D flags depending on which packages are used
SET (COMPILE_DEF_LIST "")

INCLUDE (FindPkgConfig)
INCLUDE_DIRECTORIES ($ENV{CRAY_HDF5_PREFIX}/include)
INCLUDE_DIRECTORIES ($ENV{CRAY_HDF5_PARALLEL_PREFIX}/include)
#INCLUDE_DIRECTORIES ($ENV{CRAY_HDF5_DIR}/include)
SET (ENV{PKG_CONFIG_PATH} $ENV{PKG_CONFIG_PATH}:$ENV{CRAY_PETSC_PREFIX_DIR}/lib/pkgconfig)
MESSAGE ("pkg configs --> $ENV{PKG_CONFIG_PATH}")
SET (PKG_CONFIG_EXECUTABLE ${PKG_CONFIG_PATH})

#INCLUDE (LibrarySearch.cmake)
PKG_CHECK_MODULES (CRAY_PETSC PETSc)
PKG_CHECK_MODULES (GSL gsl)
FIND_PATH (CRAY_PETSC_INCLUDE_DIR petsc.h
	  HINTS ${CRAY_PETSC_INCLUDEDIR} ${CRAY_PETSC_INCLUDE_DIR})
MESSAGE ("petsc include --> ${CRAY_PETSC_INCLUDE_DIR}")
FIND_LIBRARY (CRAY_PETSC_LIBRARY
	  NAMES libcraypetsc_crayclang_real.so libcraypetsc_crayclang_real.a
	  HINTS ${CRAY_PETSC_LIBDIR} ${CRAY_PETSC_LIBRARY_DIRS} ${CRAY_PETSC_PREFIX_DIR}/lib)
MESSAGE ("petsc library --> ${CRAY_PETSC_LIBRARY}")
FIND_PACKAGE (GSL)
IF (FOUND_GSL)
  MESSAGE ("found gsl library --> ${GSL_LIBRARY}")
  SET (COMPILE_DEF_LIST ${COMPILE_DEF_LIST};MANGO_GSL_AVAILABLE)
ENDIF ()
MESSAGE ("found petsc --> ${CRAY_PETSC_FOUND}")
IF (${CRAY_PETSC_FOUND})
   MESSAGE ("found petsc")
   SET (COMPILE_DEF_LIST ${COMPILE_DEF_LIST};MANGO_PETSC_AVAILABLE)
   INCLUDE_DIRECTORIES (SYSTEM ${CRAY_PETSC_INCLUDE_DIR})
   SET (PETSC_HDF5_PARALLEL_LIB $ENV{CRAY_HDF5_PARALLEL_PREFIX}/lib/libhdf5_parallel.a)
   SET (PETSC_SCI_CRAY_MPI_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mpi_mp.a)
   SET (PETSC_SCI_CRAY_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mp.a)
   SET (PETSC_FMPICH_LIB $ENV{CRAY_MPICH_DIR}/lib/libfmpich.a)
   INCLUDE ($ENV{CRAY_PETSC_PREFIX_DIR}/lib/petsc/conf/PETScBuildInternal.cmake)
   INCLUDE_DIRECTORIES (SYSTEM ${PETSC_PACKAGE_INCLUDES})
ENDIF ()

FOREACH (i IN LISTS COMPILE_DEF_LIST)
	MESSAGE ("list output --> ${i}")
ENDFOREACH ()
#---- set location of mango source files
SET (MANGO_API_DIR ${MANGO_SOURCE_DIR}/src/api)
#---- the location of binaries is the directory where ccmake is invoked, which
#---- is defined by ${mango_binary_dir} when project is declared
SET (MANGO_BINARY_DIR_SRC ${MANGO_SOURCE_DIR}/obj)

#------------------------------------------------------------------------------
#---- Compiler/MPI Verification
#

IF (NOT CMAKE_C_COMPILER)
   MESSAGE (FATAL_ERROR "C compiler not found. Either the compiler is not installed, or CMake does not recognize it. Add absoulte path to the compiler 'export CC=compiler_path' in the latter case")
ENDIF (NOT CMAKE_C_COMPILER)

IF (NOT CMAKE_CXX_COMPILER)
   MESSAGE (FATAL_ERROR "CXX compiler not found. Either the compiler is not installed, or CMake does not recognize it. Add absoulte path to the compiler 'export CXX=compiler_path' in the latter case")
ENDIF (NOT CMAKE_CXX_COMPILER)

IF (NOT CMAKE_Fortran_COMPILER)
   MESSAGE (FATAL_ERROR "Fortran compiler not found. Either the compiler is not installed, or CMake does not recognize it. Add absoulte path to the compiler 'export FC=compiler_path' in the latter case")
ENDIF (NOT CMAKE_Fortran_COMPILER)

INCLUDE_DIRECTORIES(SYSTEM /usr/common/software/mpich/3.3.2/cce/9.1.0/cgpu/bin)

#SET (MPI_C_COMPILER /usr/common/software/mpich/3.3.2/cce/9.1.0/cgpu/bin/mpicc)
#SET (MPI_CXX_COMPILER /usr/common/software/mpich/3.3.2/cce/9.1.0/cgpu/bin/mpicc)
#SET (MPI_Fortran_COMPILER /usr/common/software/mpich/3.3.2/cce/9.1.0/cgpu/bin/mpif90)

#IF (NOT MPI_C_COMPILER)
#   MESSAGE (FATAL_ERROR "BLAHHH")
#ENDIF ()

#SET (MPI_GUESS_LIBRARY_NAME MPICH)

#IF (NOT MPI_CXX_COMPILER)
#   IF (NOT MPI_CXX_FOUND)
#      MESSAGE (FATAL_ERROR "mango: unable to find MPI_CXX compiler")
#   ENDIF ()
#ELSE ()
   #---- when running make on OSX, cmake can't find "mpi.h". Needed to hardcode it in
#INCLUDE_DIRECTORIES(SYSTEM /usr/common/software/mpich/3.3.2/cce/9.1.0/cgpu/include)
#ENDIF ()

#IF (NOT MPI_Fortran_COMPILER)
#   IF (NOT MPI_Fortran_FOUND)
#      MESSAGE (FATAL_ERROR "mango: unable to find MPI_Fortran compiler")
#   ENDIF ()
#ENDIF (NOT MPI_Fortran_FOUND)
#
#---- End of Compiler Verification
#------------------------------------------------------------------------------

#---- Install HOPSPACK
EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_hopspack.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages RESULT_VARIABLE ierr)
MESSAGE ("ierr hops --> ${ierr}")
IF (${ierr})
   MESSAGE (FATAL_ERROR "Unsuccessful HOPSPACK Installation. Exiting...")
ELSE ()
   SET (COMPILE_DEF_LIST ${COMPILE_DEF_LIST};MANGO_HOPSPACK_AVAILABLE)
ENDIF ()

#---- Install nlopt
IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/nlopt)
   MESSAGE ("nlopt directory already exists. Checking for required files...")
   IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/nlopt/nlopt-2.6.1/install/lib64/libnlopt.a)
      MESSAGE ("nlopt is already installed on this system")
   ELSE ()
      MESSAGE ("nlopt was not correctly installed. Continuing without...")
   ENDIF ()
ELSE ()
   EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_nlopt.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages RESULT_VARIABLE ierr)
   MESSAGE ("ierr nlopt --> ${ierr}")
   IF (${ierr})
      MESSAGE ("Unsuccessful NLOPT Installation. Continuing without NLOPT")
   ELSE ()
      SET (COMPILE_DEF_LIST ${COMPILE_DEF_LIST};MANGO_NLOPT_AVAILABLE)
   ENDIF ()
ENDIF ()

#---- Install Catch2
IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/catch2)
   MESSAGE ("catch2 directory already exists. Checking for required files...")
   IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/catch2/catch.hpp)
      MESSAGE ("catch2 is already installed on this system")
   ELSE ()
      MESSAGE (FATAL_ERROR "catch2 was not correctly installed. Exiting...")
   ENDIF ()
ELSE ()
   EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_catch2.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages)
   IF (${ierr})
      MESSAGE (FATAL_ERROR "Unsuccessful Catch2 Installation. Exiting...")
   ENDIF ()
ENDIF ()

INCLUDE_DIRECTORIES (src/api)
INCLUDE_DIRECTORIES (external_packages/hopspack/src)
INCLUDE_DIRECTORIES (src/api/tests)
INCLUDE_DIRECTORIES (external_packages/catch2)
INCLUDE_DIRECTORIES (examples)
INCLUDE_DIRECTORIES (examples/src)

# Creating objects from source files in src/api
FILE (GLOB F_SRC_FILES src/api/*.F90)
ADD_LIBRARY (F_OBJ_FILES OBJECT ${F_SRC_FILES})
TARGET_COMPILE_DEFINITIONS (F_OBJ_FILES PUBLIC ${COMPILE_DEF_LIST})
FILE (GLOB CPP_SRC_FILES src/api/*.cpp)
FILE (GLOB HEADER_FILES src/api/*.hpp)
ADD_LIBRARY (CPP_OBJ_FILES OBJECT ${CPP_SRC_FILES} ${HEADER_FILES})
TARGET_INCLUDE_DIRECTORIES (CPP_OBJ_FILES PUBLIC $ENV{CRAY_HDF5_PREFIX}/include)
TARGET_COMPILE_DEFINITIONS (CPP_OBJ_FILES PUBLIC ${COMPILE_DEF_LIST})

# Creating objects from source files in hopspack
FILE (GLOB HOPSPACK_CPP_SRC_FILES external_packages/hopspack/src/*.cpp)
FILE (GLOB HOPSPACK_C_SRC_FILES external_packages/hopspack/src/*.c)
FILE (GLOB HOPSPACK_HEADER_FILES external_packages/hopspack/src/*.hpp)
ADD_LIBRARY (HOPSPACK_CPP_OBJ_FILES OBJECT ${HOPSPACK_CPP_SRC_FILES} ${HOPSPACK_HEADER_FILES} ${HEADER_FILES})
TARGET_INCLUDE_DIRECTORIES (HOPSPACK_CPP_OBJ_FILES PUBLIC $ENV{CRAY_HDF5_PREFIX}/include)
TARGET_COMPILE_DEFINITIONS (HOPSPACK_CPP_OBJ_FILES PUBLIC ${COMPILE_DEF_LIST})

ADD_LIBRARY (HOPSPACK_C_OBJ_FILES OBJECT ${HOPSPACK_C_SRC_FILES} ${HOPSPACK_HEADER_FILES})
TARGET_INCLUDE_DIRECTORIES (HOPSPACK_C_OBJ_FILES PUBLIC $ENV{CRAY_HDF5_PREFIX}/include)
TARGET_COMPILE_DEFINITIONS (HOPSPACK_C_OBJ_FILES PUBLIC ${COMPILE_DEF_LIST})

FILE (GLOB TEST_SRC_FILES src/api/tests/*.cpp)
ADD_LIBRARY (TEST_OBJ_FILES OBJECT ${TEST_SRC_FILES} ${HEADER_FILES})

ADD_LIBRARY (mango $<TARGET_OBJECTS:F_OBJ_FILES> $<TARGET_OBJECTS:CPP_OBJ_FILES> $<TARGET_OBJECTS:HOPSPACK_C_OBJ_FILES> $<TARGET_OBJECTS:HOPSPACK_CPP_OBJ_FILES>)
TARGET_LINK_LIBRARIES (mango PUBLIC ${CRAY_PETSC_LIBRARY})
TARGET_COMPILE_DEFINITIONS (mango PUBLIC ${COMPILE_DEF_LIST})
INSTALL (TARGETS mango DESTINATION ${MANGO_SOURCE_DIR}/lib)

# Create executable for unit_tests
ADD_EXECUTABLE (unit_tests ${TEST_SRC_FILES} ${HEADER_FILES})
TARGET_LINK_LIBRARIES (unit_tests PUBLIC mango)
TARGET_LINK_LIBRARIES (unit_tests PUBLIC ${CRAY_PETSC_LIBRARY})
TARGET_LINK_LIBRARIES (unit_tests PUBLIC ${PETSC_PACKAGE_LIBS})
MESSAGE ("Petsc package libs --> ${PETSC_PACKAGE_LIBS}")
TARGET_COMPILE_DEFINITIONS (unit_tests PUBLIC ${COMPILE_DEF_LIST})
INSTALL (TARGETS unit_tests DESTINATION ${MANGO_SOURCE_DIR}/tests)


# Creating objects for the examples
#FILE (GLOB F_SRC_EXAMPLES_FILES examples/src/*.F90)
#FILE (GLOB C_SRC_EXAMPLES_FILES examples/src/*.cpp)
#ADD_LIBRARY (F_OBJ_EXAMPLES_FILE ${F_SRC_EXAMPLES_FILES})
#ADD_LIBRARY (C_OBJ_EXAMPLES_FILE ${C_SRC_EXAMPLES_FILES})


#FOREACH (ALGORITHM_FILE ${F_SRC_EXAMPLES_FILES})
#	MESSAGE ("File: ${ALGORITHM_FILE}")
#	GET_FILENAME_COMPONENT (ALGORITHM_NAME ${ALGORITHM_FILE} NAME_WE)
#	MESSAGE ("${ALGORITHM_NAME}")
	#ADD_EXECUTABLE ( STRING (APPEND ALGORITHM_NAME COUNTER) bin)
	#STRING (APPEND COUNTER 1)
#ENDFOREACH ()

#INCLUDE_DIRECTORIES (lib)

#INSTALL (TARGETS ${mango} DESTINATION examples)
