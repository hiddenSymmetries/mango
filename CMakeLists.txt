#---- specify version of cmake (this version is random as of 02/19/20)
CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

# For now it is safer to set the compiler using environment variables or when cmake is called (i.e. "cmake -DCMAKE_CXX_COMPILER=ftn ..")
# But these might be useful in some cases
#SET (CMAKE_CXX_COMPILER $ENV{CRAYPE_DIR}/bin/CC)
#SET (CMAKE_C_COMPILER $ENV{CRAYPE_DIR}/bin/cc)
#SET (CMAKE_C_COMPILER $ENV{CRAYPE_DIR}/bin/ftn)

INCLUDE (User_Inputs.cmake)
#---- declaring this project name defines ${mango_source_dir} to be the path
#---- where this source file exists
PROJECT (MANGO CXX C Fortran)

#---- set location of mango source files
SET (MANGO_API_DIR ${MANGO_SOURCE_DIR}/src/api)
#---- the location of binaries is the directory where ccmake is invoked, which
#---- is defined by ${mango_binary_dir} when project is declared
SET (MANGO_BINARY_DIR_SRC ${MANGO_SOURCE_DIR}/obj)

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MANGO_SOURCE_DIR}/lib)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MANGO_SOURCE_DIR}/lib)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MANGO_SOURCE_DIR}/tests)

# IF STATEMENT depending on whether MPI compiler wrappers exist on given PrgEnv
#INCLUDE (Init_MPI.cmake)
# ENDIF ()

#---- Runs the following .cmake files that should be located in the same directory as this file
INCLUDE (Package_Search.cmake)
INCLUDE (Install_External_Packages.cmake)

#---- Directories containing headers or source files
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

#-----------------------------------------------------------------------
#---- Remaining to be done

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
