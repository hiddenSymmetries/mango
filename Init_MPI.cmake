IF (${PLATFORM} MATCHES NERSC_Cori)
  
  MESSAGE ("NERSC Cori selected as platform")
  MESSAGE ("=========================================================")
  MESSAGE ("Please run the following to ensure that the build is done")
  MESSAGE ("with NERSC compiler wrappers")
  MESSAGE ("=========================================================")
  MESSAGE ("cmake -DCMAKE_CXX_COMPILER=CC ..")
  # The following Cray compiler wrappers include MPI headers/libraries
  #SET ($ENV{CC} "cc")
  #SET ($ENV{CXX} "CC")
  #SET ($ENV{Fortran} "ftn")

ELSE ()
  
  MESSAGE ("${PLATFORM} selected as platform")
  FIND_PACKAGE (MPI REQUIRED)
  
  #---- the find_package command sets '<package>_found' to true if it's successful
  IF (MPI_FOUND)
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_C_INCLUDE_PATH})
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_CXX_INCLUDE_PATH})
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_Fortran_INCLUDE_PATH})
    # Must explicitly include the header directories once the MPI libs/headers are found by the FindMPI.cmake package
  ENDIF ()

ENDIF ()
