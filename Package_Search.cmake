#---- "Allocating" empty vector to be filled with -D flags depending on which packages are used
FILE (WRITE ${MANGO_SOURCE_DIR}/examples/packages_available "mango ")

#---- Built-in module FindPkgConfig.cmake to detect *.pc files
INCLUDE (FindPkgConfig)
#---- For most modules loaded on the Cray system, a path will be added to the PKG_CONFIG_PATH environment variable pointing to the *.pc file. If it is not included for some reason, it can be appended to the following command with ":/path/to/*.pc"

IF (MANGO_INCLUDE_PETSC)
  
  IF (${PLATFORM} MATCHES NERSC_Cori)
    # Cori had problems detecting the .pc files for cray-petsc
    SET (ENV{PKG_CONFIG_PATH} $ENV{PKG_CONFIG_PATH}:$ENV{CRAY_PETSC_PREFIX_DIR}/lib/pkgconfig:$ENV{PYTHON_DIR}/lib/pkgconfig)
  ENDIF ()
    
  PKG_CHECK_MODULES (PETSC PETSc)
  FIND_PACKAGE (PETSc)
  IF (${PETSC_FOUND})
    MESSAGE ("found petsc")
    FIND_LIBRARY (PETSC_LIBRARY
      NAMES libcraypetsc_crayclang_real.so libcraypetsc_crayclang_real.a libpetsc.a libpetsc.so libpetsc_real.a libpetsc_real.so
      HINTS ${PETSC_LIBDIR} ${PETSC_LIBRARY_DIRS} ${CRAY_PETSC_PREFIX_DIR}/lib ${PETSC_LIBRARIES}) 
    MESSAGE ("petsc include --> ${PETSC_INCLUDE_DIRS}")
    MESSAGE ("petsc library --> ${PETSC_LIBRARY}")
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "petsc ")
    LIST (APPEND COMPILE_DEF_LIST MANGO_PETSC_AVAILABLE)
  
    # cray-petsc was throwing errors at one point if the following libraries were not included
    #SET (PETSC_HDF5_PARALLEL_LIB $ENV{CRAY_HDF5_PARALLEL_PREFIX}/lib/libhdf5_parallel.a)
    #SET (PETSC_SCI_CRAY_MPI_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mpi_mp.a)
    #SET (PETSC_SCI_CRAY_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mp.a)
    #SET (PETSC_FMPICH_LIB $ENV{CRAY_MPICH_DIR}/lib/libfmpich.a)
    #Put all these into PETSC_PACKAGE_LIBS
    
    LIST (APPEND LIBRARY_LINK_LIST "-L${PETSC_LIBRARY_DIRS} -l${PETSC_LIBRARIES}")
    LIST (APPEND INCLUDE_LIST ${PETSC_INCLUDE_DIRS})
    #IF (${PLATFORM} MATCHES Travis_CI)
      # Some headers could not be found on Travis (specifically petsctao.h) with the main include directory 
    #  LIST (APPEND INCLUDE_LIST ${PETSC_INCLUDE_DIR}/petsc/finclude)
    #ENDIF ()
    IF (${PLATFORM} MATCHES NERSC_Cori)
      INCLUDE ($ENV{CRAY_PETSC_PREFIX_DIR}/lib/petsc/conf/PETScBuildInternal.cmake)
      LIST(APPEND INCLUDE_LIST $ENV{CRAY_HDF5_PARALLEL_PREFIX}/include)
    ENDIF ()
    
  ELSE ()
    MESSAGE (FATAL_ERROR "PETSC was not found. Check include/lib directories. Exiting...")
  ENDIF ()
  
ENDIF ()

PKG_CHECK_MODULES (GSL gsl)
FIND_PACKAGE (GSL) # Built-in CMake module
IF (GSL_FOUND)
  MESSAGE ("found gsl library --> ${GSL_LIBRARY}")
  MESSAGE ("gsl include --> ${GSL_INCLUDE_DIR}")
  LIST (APPEND COMPILE_DEF_LIST MANGO_GSL_AVAILABLE)
  LIST (APPEND INCLUDE_LIST ${GSL_INCLUDE_DIR})
  LIST (APPEND LIBRARY_LINK_LIST ${GSL_LIBRARY})
  FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "gsl ")
ENDIF ()

PKG_CHECK_MODULES (Python3 python3)
#  FIND_PACKAGE(Python3 COMPONENTS Interpreter Development)
#  IF (Python3_FOUND)
#    MESSAGE ("found python library --> ${Python3_LIBRARY_DIRS}${Python3_LIBRARIES}")
#    MESSAGE ("python include --> ${Python3_INCLUDE_DIRS}")
#    INCLUDE_DIRECTORIES (SYSTEM ${PYTHON_INCLUDE_DIRS})
#    SET (LIBRARY_LINK_LIST ${LIBRARY_LINK_LIST};${PYTHON_LIBRARY_DIRS}${PYTHON_LIBRARIES})
#    #FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "gsl ")
#  ENDIF ()
#  IF (Python3_Development_FOUND)
#    MESSAGE ("found development packages")
#  ENDIF ()
