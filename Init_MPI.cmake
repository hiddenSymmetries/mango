IF (${PLATFORM} MATCHES NERSC_Cori)
  MESSAGE ("NERSC Cori selected as platform")
  # The following Cray compiler wrappers include MPI headers/libraries
  SET (CMAKE_C_COMPILER "cc")
  SET (CMAKE_CXX_COMPILER "CC")
  SET (CMAKE_Fortran_COMPILER "ftn")
ELSE ()
  FIND_PACKAGE (MPI REQUIRED)

  #---- the find_package command sets '<package>_found' to true if it's successful
  IF (MPI_FOUND)
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_C_INCLUDE_PATH})
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_CXX_INCLUDE_PATH})
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_Fortran_INCLUDE_PATH})
    # Must explicitly include the header directories once the MPI libs/headers are found by the FindMPI.cmake package
  ENDIF ()

ENDIF ()
