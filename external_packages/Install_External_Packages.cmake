#---- Installation of packages in mango/external_packages

#---- HOPSPACK
IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/hopspack/hopspack-2.0.2-src)
  #MESSAGE ("hopspack directory already exists. Checking for required files...")
  IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/hopspack/hopspack-2.0.2-src/src/src-main)
    # not sure how to best specify that hopspack has been installed yet
    MESSAGE (STATUS "Found HOPSPACK")
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "hopspack ")
    LIST (APPEND COMPILE_DEF_LIST MANGO_HOPSPACK_AVAILABLE)
    SET (PACKAGE_INCLUDE_LIST "hopspack")
    INCLUDE_DIRECTORIES (external_packages/hopspack/src)
  ELSE ()
    MESSAGE (FATAL_ERROR "Unsuccessful HOPSPACK Installation. Exiting...")
  ENDIF ()
ELSE ()
  EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_hopspack.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages RESULT_VARIABLE ierr)
  MESSAGE ("ierr hops --> ${ierr}")
  IF (${ierr})
    MESSAGE (FATAL_ERROR "Unsuccessful HOPSPACK Installation. Exiting...")
  ELSE ()
    MESSAGE (STATUS "HOPSPACK successfully installed")
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "hopspack ")
    LIST (APPEND COMPILE_DEF_LIST MANGO_HOPSPACK_AVAILABLE)
    SET (PACKAGE_INCLUDE_LIST "hopspack")
    INCLUDE_DIRECTORIES (external_packages/hopspack/src)
  ENDIF ()
ENDIF ()

#---- nlopt
IF (NOT NO_NLOPT)
  LIST (APPEND PACKAGE_INCLUDE_LIST "nlopt")
  SET (NLOPT_LIBRARY ${MANGO_SOURCE_DIR}/external_packages/nlopt/nlopt-2.6.1/install/lib64/libnlopt.a)
  SET (NLOPT_INCLUDE_DIR ${MANGO_SOURCE_DIR}/external_packages/nlopt/nlopt-2.6.1/install/include)
  IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/nlopt)
    #MESSAGE ("nlopt directory already exists. Checking for required files...")
    IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/nlopt/nlopt-2.6.1/install/lib64/libnlopt.a)
      MESSAGE (STATUS "Found nlopt")
      LIST (APPEND COMPILE_DEF_LIST MANGO_NLOPT_AVAILABLE)
      LIST (APPEND INCLUDE_LIST ${NLOPT_INCLUDE_DIR})
      LIST (APPEND LIBRARY_LINK_LIST ${NLOPT_LIBRARY})
      FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "nlopt ")
    ELSE ()
      MESSAGE (FATAL_ERROR "nlopt was not correctly installed. Continuing without...")
    ENDIF ()
  ELSE ()
    EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_nlopt.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages RESULT_VARIABLE ierr)
    MESSAGE ("ierr nlopt --> ${ierr}")
    IF (${ierr})
      MESSAGE (FATAL_ERROR "Unsuccessful NLOPT Installation. User specified MANGO build w/ nlopt. Exiting..")
    ELSE ()
      MESSAGE (STATUS "nlopt successfully installed")
      LIST (APPEND COMPILE_DEF_LIST MANGO_NLOPT_AVAILABLE)
      LIST (APPEND INCLUDE_LIST ${NLOPT_INCLUDE_DIR})
      LIST (APPEND LIBRARY_LINK_LIST ${NLOPT_LIBRARY})
      FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "nlopt ")
    ENDIF ()
  ENDIF ()
ENDIF ()

  
#---- Eigen
IF (NOT NO_EIGEN)
  LIST (APPEND PACKAGE_INCLUDE_LIST "eigen")
  SET (EIGEN_INCLUDE_DIR ${MANGO_SOURCE_DIR}/external_packages/eigen/eigen-3.3.7)
  IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/eigen/eigen-3.3.7/Eigen)
    MESSAGE (STATUS "Found Eigen")
    LIST (APPEND COMPILE_DEF_LIST MANGO_EIGEN_AVAILABLE)
    LIST (APPEND INCLUDE_LIST ${EIGEN_INCLUDE_DIR})
    FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "eigen ")
  ELSE ()
    EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_eigen.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages)
    IF (${ierr})
      MESSAGE (FATAL_ERROR "Unsuccessful Eigen Installation. Exiting...")
    ELSE ()
      MESSAGE (STATUS "Eigen successfully installed")
      LIST (APPEND COMPILE_DEF_LIST MANGO_EIGEN_AVAILABLE)
      LIST (APPEND INCLUDE_LIST ${EIGEN_INCLUDE_DIR})
      FILE (APPEND ${MANGO_SOURCE_DIR}/examples/packages_available "eigen ")
    ENDIF ()
  ENDIF ()
ENDIF ()

#---- Catch2
IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/catch2)
   #MESSAGE ("catch2 directory already exists. Checking for required files...")
   IF (EXISTS ${MANGO_SOURCE_DIR}/external_packages/catch2/catch.hpp)
     MESSAGE (STATUS "Found Catch2")
     INCLUDE_DIRECTORIES (external_packages/catch2)
   ELSE ()
      MESSAGE (FATAL_ERROR "catch.hpp header was not found. Exiting...")
   ENDIF ()
ELSE ()
   EXECUTE_PROCESS (COMMAND ${MANGO_SOURCE_DIR}/external_packages/install_catch2.sh WORKING_DIRECTORY ${MANGO_SOURCE_DIR}/external_packages)
   IF (${ierr})
     MESSAGE (FATAL_ERROR "Unsuccessful Catch2 Installation. Exiting...")
   ELSE ()
     MESSAGE (STATUS "Catch2 successfully installed")
     INCLUDE_DIRECTORIES (external_packages/catch2)
   ENDIF ()
ENDIF ()
