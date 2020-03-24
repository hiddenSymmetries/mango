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

#---- Choose system to build on (only NERSC Cori is available now)
SET (PLATFORM NERSC_Cori)
MESSAGE ("Platform is NERSC Cori")
#SET (PLATFORM PLATFORM_NAME_HERE)
