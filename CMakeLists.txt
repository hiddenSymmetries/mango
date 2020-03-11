#---- specify version of cmake (this version is random as of 02/19/20)
CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

#---- declaring this project name defines ${mango_source_dir} to be the path
#---- where this source file exists
PROJECT (MANGO CXX C Fortran)

INCLUDE (FindPkgConfig)
SET (ENV{PKG_CONFIG_PATH} $ENV{PKG_CONFIG_PATH}:$ENV{CRAY_PETSC_PREFIX_DIR}/lib/pkgconfig)
#MESSAGE ("pkg configs --> $ENV{PKG_CONFIG_PATH}")
SET (PKG_CONFIG_EXECUTABLE ${PKG_CONFIG_PATH})

#INCLUDE (LibrarySearch.cmake)
PKG_CHECK_MODULES (CRAY_PETSC PETSc)
PKG_CHECK_MODULES (GSL gsl)
FIND_PATH (CRAY_PETSC_INCLUDE_DIR petsc.h
	  HINTS ${CRAY_PETSC_INCLUDEDIR} ${CRAY_PETSC_INCLUDE_DIR})
MESSAGE ("petsc include --> ${CRAY_PETSC_INCLUDE_DIR}")
FIND_LIBRARY (CRAY_PETSC_LIBRARY
	  NAMES libcraypetsc_crayclang_real.so libcraypetsc_crayclang_lrea
	  HINTS ${CRAY_PETSC_LIBDIR} ${CRAY_PETSC_LIBRARY_DIRS} ${CRAY_PETSC_PREFIX_DIR}/lib)
MESSAGE ("petsc library --> ${CRAY_PETSC_LIBRARY}")
FIND_PACKAGE (GSL)
MESSAGE ("gsl library --> ${GSL_LIBRARY}")
MESSAGE ("found petsc --> ${FOUND_CRAY_PETSC}")
#---- set location of mango source files
SET (MANGO_API_DIR ${MANGO_SOURCE_DIR}/src/api)
#---- the location of binaries is the directory where ccmake is invoked, which
#---- is defined by ${mango_binary_dir} when project is declared
SET (MANGO_BINARY_DIR_SRC ${MANGO_SOURCE_DIR}/obj)

#FIND_PACKAGE (CRAY_PETSC REQUIRED)
#SET (CMAKE_C_COMPILER_ID)
#MESSAGE ("${CMAKE_C_COMPILER_ID}")
#SET (CMAKE_CXX_COMPILER_ID)
#SET (CMAKE_Fortran_COMPILER_ID)

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
#SET (MPIEXEC_EXECUTABLE /usr/common/software/mpich/3.3.1-debug/gcc/7.3.0/bin/mpiexec)
FIND_PACKAGE (MPI REQUIRED)
#---- the find_package command sets '<package>_found' to true if it's successful

IF (NOT MPI_C_COMPILER)
   IF (NOT MPI_C_FOUND)
      MESSAGE (FATAL_ERROR "mango: unable to find MPI_C compiler")
   ELSE ()
      MESSAGE (FATAL_ERROR "Found MPI_C ??")
   ENDIF ()
ENDIF (NOT MPI_C_COMPILER)

#IF (NOT MPI_CXX_COMPILER)
#   IF (NOT MPI_CXX_FOUND)
#      MESSAGE (FATAL_ERROR "mango: unable to find MPI_CXX compiler")
#   ENDIF ()
#ELSE ()
   #---- when running make on OSX, cmake can't find "mpi.h". Needed to hardcode it in
INCLUDE_DIRECTORIES(SYSTEM /usr/common/software/mpich/3.3.2/cce/9.1.0/cgpu/include)
#ENDIF ()

#IF (NOT MPI_Fortran_COMPILER)
#   IF (NOT MPI_Fortran_FOUND)
#      MESSAGE (FATAL_ERROR "mango: unable to find MPI_Fortran compiler")
#   ENDIF ()
#ENDIF (NOT MPI_Fortran_FOUND)
#
#---- End of Compiler Verification
#------------------------------------------------------------------------------

EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_hopspack.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages)

EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_nlopt.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages)

EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_catch2.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages)

INCLUDE_DIRECTORIES (src/api)
INCLUDE_DIRECTORIES (external_packages/hopspack/src)
INCLUDE_DIRECTORIES (src/api/tests)
INCLUDE_DIRECTORIES (external_packages/catch2)
INCLUDE_DIRECTORIES (examples)
INCLUDE_DIRECTORIES (examples/src)

# Creating objects from source files in src/api
FILE (GLOB F_SRC_FILES src/api/*.F90)
ADD_LIBRARY (F_OBJ_FILES OBJECT ${F_SRC_FILES})
FILE (GLOB CPP_SRC_FILES src/api/*.cpp)
FILE (GLOB HEADER_FILES src/api/*.hpp)
ADD_LIBRARY (CPP_OBJ_FILES OBJECT ${CPP_SRC_FILES} ${HEADER_FILES})

# Creating objects from source files in hopspack
FILE (GLOB HOPSPACK_CPP_SRC_FILES external_packages/hopspack/src/*.cpp)
FILE (GLOB HOPSPACK_C_SRC_FILES external_packages/hopspack/src/*.c)
FILE (GLOB HOPSPACK_HEADER_FILES external_packages/hopspack/src/*.hpp)
ADD_LIBRARY (HOPSPACK_CPP_OBJ_FILES OBJECT ${HOPSPACK_CPP_SRC_FILES} ${HOPSPACK_HEADER_FILES} ${HEADER_FILES})
ADD_LIBRARY (HOPSPACK_C_OBJ_FILES OBJECT ${HOPSPACK_C_SRC_FILES} ${HOPSPACK_HEADER_FILES})

FILE (GLOB TEST_SRC_FILES src/api/tests/*.cpp)
ADD_LIBRARY (TEST_OBJ_FILES OBJECT ${TEST_SRC_FILES} ${HEADER_FILES})

ADD_LIBRARY (mango $<TARGET_OBJECTS:F_OBJ_FILES> $<TARGET_OBJECTS:CPP_OBJ_FILES> $<TARGET_OBJECTS:HOPSPACK_C_OBJ_FILES> $<TARGET_OBJECTS:HOPSPACK_CPP_OBJ_FILES>)
INSTALL (TARGETS mango DESTINATION ${MANGO_SOURCE_DIR}/lib)

# Creating objects for the examples
FILE (GLOB F_SRC_EXAMPLES_FILES examples/src/*.F90)
FILE (GLOB C_SRC_EXAMPLES_FILES examples/src/*.cpp)
ADD_LIBRARY (F_OBJ_EXAMPLES_FILE ${F_SRC_EXAMPLES_FILES})
ADD_LIBRARY (C_OBJ_EXAMPLES_FILE ${C_SRC_EXAMPLES_FILES})


FOREACH (ALGORITHM_FILE ${F_SRC_EXAMPLES_FILES})
	MESSAGE ("File: ${ALGORITHM_FILE}")
	GET_FILENAME_COMPONENT (ALGORITHM_NAME ${ALGORITHM_FILE} NAME_WE)
	MESSAGE ("${ALGORITHM_NAME}")
	#ADD_EXECUTABLE ( STRING (APPEND ALGORITHM_NAME COUNTER) bin)
	#STRING (APPEND COUNTER 1)
ENDFOREACH ()

#INCLUDE_DIRECTORIES (lib)

#INSTALL (TARGETS ${mango} DESTINATION examples)
