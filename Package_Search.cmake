IF (${PLATFORM} MATCHES NERSC_Cori)
  #---- "Allocating" empty vector to be filled with -D flags depending on which packages are used
  SET (INCLUDE_LIST "")
  SET (COMPILE_DEF_LIST "")
  SET (LIBRARY_LINK_LIST "")
  FILE (WRITE ${MANGO_SOURCE_DIR}/examples/packages_available "mango ")
  
  #---- Built-in module FindPkgConfig.cmake to detect *.pc files
  INCLUDE (FindPkgConfig)

  #---- For most modules loaded on the Cray system, a path will be added to the PKG_CONFIG_PATH environment variable pointing to the *.pc file. If it is not included for some reason, it can be appended to the following command with ":/path/to/*.pc"
  SET (ENV{PKG_CONFIG_PATH} $ENV{PKG_CONFIG_PATH}:$ENV{CRAY_PETSC_PREFIX_DIR}/lib/pkgconfig)
  MESSAGE ("pkg configs --> $ENV{PKG_CONFIG_PATH}")

  # Checks PKG_CONFIG paths to find *.pc file with name of second argument
  PKG_CHECK_MODULES (CRAY_PETSC PETSc)
  PKG_CHECK_MODULES (GSL gsl)

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
    SET (COMPILE_DEF_LIST ${COMPILE_DEF_LIST};MANGO_GSL_AVAILABLE)
    INCLUDE_DIRECTORIES (SYSTEM ${GSL_INCLUDE_DIR})
    SET (LIBRARY_LINK_LIST ${LIBRARY_LINK_LIST};${GSL_LIBRARY})
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "gsl ")
  ENDIF ()

  IF (${CRAY_PETSC_FOUND})
    MESSAGE ("found petsc")
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "petsc ")
    SET (COMPILE_DEF_LIST ${COMPILE_DEF_LIST};MANGO_PETSC_AVAILABLE)
    #INCLUDE_DIRECTORIES (SYSTEM ${CRAY_PETSC_INCLUDE_DIR})
    #INCLUDE_DIRECTORIES ($ENV{CRAY_HDF5_PARALLEL_PREFIX}/include)
    SET (PETSC_HDF5_PARALLEL_LIB $ENV{CRAY_HDF5_PARALLEL_PREFIX}/lib/libhdf5_parallel.a)
    SET (PETSC_SCI_CRAY_MPI_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mpi_mp.a)
    SET (PETSC_SCI_CRAY_MP_LIB $ENV{CRAY_LIBSCI_PREFIX_DIR}/lib/libsci_cray_mp.a)
    SET (PETSC_FMPICH_LIB $ENV{CRAY_MPICH_DIR}/lib/libfmpich.a)
    SET (LIBRARY_LINK_LIST ${LIBRARY_LINK_LIST};${CRAY_PETSC_LIBRARY};${PETSC_PACKAGE_LIBS})
    INCLUDE ($ENV{CRAY_PETSC_PREFIX_DIR}/lib/petsc/conf/PETScBuildInternal.cmake)
    #INCLUDE_DIRECTORIES (SYSTEM ${PETSC_PACKAGE_INCLUDES})
    SET (INCLUDE_LIST ${INCLUDE_LIST};${CRAY_PETSC_INCLUDE_DIR};$ENV{CRAY_HDF5_PARALLEL_PREFIX}/include;${PETSC_PACKAGES_INCLUDES})
  ENDIF ()
ELSEIF (${PLATFORM} MATCHES Macports)
  # do stuff
ENDIF ()
