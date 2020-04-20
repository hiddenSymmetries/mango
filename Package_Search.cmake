IF (${PLATFORM} MATCHES NERSC_Cori)
  #---- "Allocating" empty vector to be filled with -D flags depending on which packages are used
  SET (INCLUDE_LIST "")
  SET (COMPILE_DEF_LIST "")
  SET (LIBRARY_LINK_LIST "")
  FILE (WRITE ${MANGO_SOURCE_DIR}/examples/packages_available "mango ")
  
  #---- Built-in module FindPkgConfig.cmake to detect *.pc files
  INCLUDE (FindPkgConfig)

  #---- For most modules loaded on the Cray system, a path will be added to the PKG_CONFIG_PATH environment variable pointing to the *.pc file. If it is not included for some reason, it can be appended to the following command with ":/path/to/*.pc"
  SET (ENV{PKG_CONFIG_PATH} $ENV{PKG_CONFIG_PATH}:$ENV{CRAY_PETSC_PREFIX_DIR}/lib/pkgconfig:$ENV{PYTHON_DIR}/lib/pkgconfig)
  MESSAGE ("pkg configs --> $ENV{PKG_CONFIG_PATH}")

  # Checks PKG_CONFIG paths to find *.pc file with name of second argument
  PKG_CHECK_MODULES (CRAY_PETSC PETSc)
  PKG_CHECK_MODULES (GSL gsl)
  PKG_CHECK_MODULES (Python3 python3)

  FIND_PATH (CRAY_PETSC_INCLUDE_DIR petsc.h
    HINTS ${CRAY_PETSC_INCLUDEDIR} ${CRAY_PETSC_INCLUDE_DIR})
  MESSAGE ("petsc include --> ${CRAY_PETSC_INCLUDE_DIR}")
  FIND_LIBRARY (CRAY_PETSC_LIBRARY
    NAMES libcraypetsc_crayclang_real.so libcraypetsc_crayclang_real.a
    HINTS ${CRAY_PETSC_LIBDIR} ${CRAY_PETSC_LIBRARY_DIRS} ${CRAY_PETSC_PREFIX_DIR}/lib)
  MESSAGE ("petsc library --> ${CRAY_PETSC_LIBRARY}")

  FIND_PACKAGE (GSL) # Built-in CMake module
  IF (GSL_FOUND)
    MESSAGE ("found gsl library --> ${GSL_LIBRARY}")
    MESSAGE ("gsl include --> ${GSL_INCLUDE_DIR}")
    LIST (APPEND COMPILE_DEF_LIST MANGO_GSL_AVAILABLE)
    LIST (APPEND INCLUDE_LIST ${GSL_INCLUDE_DIR})
    LIST (APPEND LIBRARY_LINK_LIST ${GSL_LIBRARY})
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "gsl ")
  ENDIF ()

  IF (${CRAY_PETSC_FOUND})
    MESSAGE ("found petsc")
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "petsc ")
    LIST (APPEND COMPILE_DEF_LIST MANGO_PETSC_AVAILABLE)

    # cray-petsc was throwing errors at one point if the following libraries were not included
    #SET (PETSC_HDF5_PARALLEL_LIB $ENV{CRAY_HDF5_PARALLEL_PREFIX}/lib/libhdf5_parallel.a)
    #SET (PETSC_SCI_CRAY_MPI_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mpi_mp.a)
    #SET (PETSC_SCI_CRAY_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mp.a)
    #SET (PETSC_FMPICH_LIB $ENV{CRAY_MPICH_DIR}/lib/libfmpich.a)
    #Put all these into PETSC_PACKAGE_LIBS
    
    LIST (APPEND LIBRARY_LINK_LIST ${CRAY_PETSC_LIBRARY})
    INCLUDE ($ENV{CRAY_PETSC_PREFIX_DIR}/lib/petsc/conf/PETScBuildInternal.cmake)
    LIST(APPEND INCLUDE_LIST ${CRAY_PETSC_INCLUDE_DIR};$ENV{CRAY_HDF5_PARALLEL_PREFIX}/include)
  ENDIF ()

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
  
  
ELSEIF (${PLATFORM} MATCHES Macports)
  # do stuff
ENDIF ()
