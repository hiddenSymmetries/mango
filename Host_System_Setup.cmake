#-----------------------------------------------------------
#-----------------------------------------------------------
#---- SET THESE VARIABLES BEFORE RUNNING CMAKE! ------------
#-----------------------------------------------------------
# Cori modules to load:
# module swap PrgEnv-intel PrgEnv-cray
# module load cray-petsc
# module load gsl
#

SET (AVAILABLE_SYSTEMS cori;travis)
CMAKE_HOST_SYSTEM_INFORMATION (RESULT SYSTEM_NAME QUERY HOSTNAME)

FOREACH (AVAILABLE_SYSTEM_ID ${AVAILABLE_SYSTEMS})
  STRING (FIND ${SYSTEM_NAME} ${AVAILABLE_SYSTEM_ID} STRING_LOCATION)
  IF (NOT ${STRING_LOCATION} EQUAL -1)
    #MESSAGE ("************************************************************")
    #MESSAGE ("CMake has detected ${AVAILABLE_SYSTEM_ID} as the system host")
    SET (HOST_SYSTEM ${AVAILABLE_SYSTEM_ID})
    BREAK ()
  ENDIF ()
ENDFOREACH ()

IF (NOT HOST_SYSTEM)
  MESSAGE (FATAL_ERROR "CMake was not able to detect a known host system. Exiting...")
ENDIF ()

#---- "Allocating" empty vector to be filled with -D flags depending on which packages are used 
SET (INCLUDE_LIST "")
SET (COMPILE_DEF_LIST "")
SET (LIBRARY_LINK_LIST "")

IF (${HOST_SYSTEM} MATCHES cori)

  MESSAGE ("")
  MESSAGE ("")
  MESSAGE ("=========================================================")
  MESSAGE ("NERSC Cori selected as host system")
  MESSAGE ("Please run the following to ensure that the build is done")
  MESSAGE ("with NERSC compiler wrappers")
  MESSAGE ("")
  MESSAGE ("cmake -DCMAKE_CXX_COMPILER=CC ..")
  MESSAGE ("=========================================================")

ELSE ()
  
  IF (${HOST_SYSTEM} MATCHES travis)
    MESSAGE ("Travis CI selected as host system")
    MESSAGE ("=========================================================")
  ENDIF ()
  
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

# Detect whether host uses SLURM batch system
EXECUTE_PROCESS (COMMAND sinfo RESULT_VARIABLE SINFO_RESULT OUTPUT_QUIET ERROR_QUIET)
#MESSAGE ("result of sinfo --> ${SINFO_RESULT}")
EXECUTE_PROCESS (COMMAND mpiexec --version RESULT_VARIABLE MPIEXEC_RESULT OUTPUT_QUIET ERROR_QUIET)
#MESSAGE ("result of mpiexec --> ${MPIEXEC_RESULT}")

IF (SINFO_RESULT EQUAL 0)
  SET (MANGO_COMMAND_TO_SUBMIT_JOB "srun -n NUM_PROCS")
  # Message with "STATUS" tag prints to stdout, which the Python script "run_mpi_unit_tests" uses to set the submit command for each system 
  MESSAGE (STATUS "MANGO_COMMAND_TO_SUBMIT_JOB=${MANGO_COMMAND_TO_SUBMIT_JOB}\n")
ELSEIF (MPIEXEC_RESULT EQUAL 0)
  SET (MANGO_COMMAND_TO_SUBMIT_JOB "mpiexec -n NUM_PROCS")
  IF (${HOST_SYSTEM} MATCHES travis)
    STRING (APPEND MANGO_COMMAND_TO_SUBMIT_JOB " --mca btl_base_warn_component_unused 0 --mca orte_base_help_aggregate 0")
    MESSAGE (STATUS "MANGO_COMMAND_TO_SUBMIT_JOB=${MANGO_COMMAND_TO_SUBMIT_JOB}\n")
  ELSE ()
    MESSAGE (STATUS "MANGO_COMMAND_TO_SUBMIT_JOB=${MANGO_COMMAND_TO_SUBMIT_JOB}\n")
  ENDIF ()
ELSE ()
  MESSAGE (FATAL_ERROR "WARNING!! Neither slurm nor mpiexec was detected. Exiting...")
ENDIF ()

#---- Create job submit files for unit tests and regression tests
FILE (WRITE ${MANGO_SOURCE_DIR}/tests/jobSubmit.info ${MANGO_COMMAND_TO_SUBMIT_JOB})
FILE (WRITE ${MANGO_SOURCE_DIR}/examples/jobSubmit.info ${MANGO_COMMAND_TO_SUBMIT_JOB})
