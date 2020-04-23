#-----------------------------------------------------------
#-----------------------------------------------------------
#---- SET THESE VARIABLES BEFORE RUNNING CMAKE! ------------
#-----------------------------------------------------------
# Cori modules to load:
# module load PrgEnv-cray
# module load cray-petsc
# module load cray-hdf5-parallel
# module load gsl
SET (AVAILABLE_SYSTEMS cori;travis)
CMAKE_HOST_SYSTEM_INFORMATION (RESULT SYSTEM_NAME QUERY HOSTNAME)

FOREACH (AVAILABLE_SYSTEM_ID ${AVAILABLE_SYSTEMS})
  STRING (FIND ${SYSTEM_NAME} ${AVAILABLE_SYSTEM_ID} STRING_LOCATION)
  IF (NOT ${STRING_LOCATION} EQUAL -1)
    MESSAGE ("************************************************************")
    MESSAGE ("CMake has detected ${AVAILABLE_SYSTEM_ID} as the system host")
    SET (PLATFORM ${AVAILABLE_SYSTEM_ID})
    BREAK ()
  ENDIF ()
ENDFOREACH ()

IF (NOT PLATFORM)
  MESSAGE (FATAL_ERROR "CMake was not able to detect a known host system. Exiting...")
ENDIF ()

#---- Choose packages to be included in the MANGO build
#SET (MANGO_INCLUDE_HOPSPACK true)
#SET (MANGO_INCLUDE_PETSC true)
#SET (MANGO_INCLUDE_GSL true)
#SET (MANGO_INCLUDE_NLOPT true)
#SET (MANGO_INCLUDE_DAKOTA false)

