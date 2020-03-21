# This .cmake file is not yet called by the main CMakeLists.txt file
FIND_PACKAGE (MPI REQUIRED)

#---- the find_package command sets '<package>_found' to true if it's successful
IF (MPI_XXX_FOUND)
  INCLUDE_DIRECTORIES (SYSTEM ${MPI_C_INCLUDE_PATH})
  INCLUDE_DIRECTORIES (SYSTEM ${MPI_CXX_INCLUDE_PATH})
  INCLUDE_DIRECTORIES (SYSTEM ${MPI_Fortran_INCLUDE_PATH})
  # Must explicitly include the header directories once the MPI libs/headers are found by the FindMPI.cmake package
ENDIF ()

# had some luck getting cmake to find executables when specifying this..?
#SET (MPIEXEC_EXECUTABLE ${CRAY_MPICH_DIR}/bin/mpiexec) 
