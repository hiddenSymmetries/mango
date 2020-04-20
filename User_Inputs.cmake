#-----------------------------------------------------------
#-----------------------------------------------------------
#---- SET THESE VARIABLES BEFORE RUNNING CMAKE! ------------
#-----------------------------------------------------------
# Cori modules to load:
# module load cmake
# module load PrgEnv-cray
# module load cray-petsc
# module load cray-hdf5-parallel
# module load gsl
# also set the environment variable "export CRAYPE_LINK_TYPE=static"

#---- Choose system to build on
#---- Options: "NERSC_Cori", "Travis_CI"
#---- Syntax: SET (PLATFORM Enter_system_name_here)
SET (PLATFORM NERSC_Cori)
MESSAGE ("Platform is ${PLATFORM}")

#---- Choose packages to be included in the MANGO build
SET (MANGO_INCLUDE_HOPSPACK true)
SET (MANGO_INCLUDE_PETSC true)
SET (MANGO_INCLUDE_GSL true)
SET (MANGO_INCLUDE_NLOPT true)
SET (MANGO_INCLUDE_DAKOTA false)

