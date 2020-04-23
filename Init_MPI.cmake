#---- "Allocating" empty vector to be filled with -D flags depending on which packages are used 
SET (INCLUDE_LIST "")
SET (COMPILE_DEF_LIST "")
SET (LIBRARY_LINK_LIST "")

IF (${PLATFORM} MATCHES cori)
  
  MESSAGE ("NERSC Cori selected as platform")
  MESSAGE ("=========================================================")
  MESSAGE ("Please run the following to ensure that the build is done")
  MESSAGE ("with NERSC compiler wrappers")
  MESSAGE ("=========================================================")
  MESSAGE ("cmake -DCMAKE_CXX_COMPILER=CC ..")
  MESSAGE ("=========================================================")

ELSE ()
  
  FIND_PACKAGE (MPI REQUIRED)
  
  #---- the find_package command sets '<package>_found' to true if it's successful
  IF (MPI_FOUND)
    # Must explicitly include the header directories once the MPI libs/headers are found by the FindMPI.cmake package
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_C_INCLUDE_PATH})
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_CXX_INCLUDE_PATH})
    INCLUDE_DIRECTORIES (SYSTEM ${MPI_Fortran_INCLUDE_PATH})

    LIST (APPEND LIBRARY_LINK_LIST ${MPI_C_LIBRARIES})
    LIST (APPEND LIBRARY_LINK_LIST ${MPI_CXX_LIBRARIES})
    LIST (APPEND LIBRARY_LINK_LIST ${MPI_Fortran_LIBRARIES})
  ENDIF ()

ENDIF ()
