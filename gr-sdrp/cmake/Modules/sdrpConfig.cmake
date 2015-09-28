INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_SDRP sdrp)

FIND_PATH(
    SDRP_INCLUDE_DIRS
    NAMES sdrp/api.h
    HINTS $ENV{SDRP_DIR}/include
        ${PC_SDRP_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SDRP_LIBRARIES
    NAMES gnuradio-sdrp
    HINTS $ENV{SDRP_DIR}/lib
        ${PC_SDRP_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDRP DEFAULT_MSG SDRP_LIBRARIES SDRP_INCLUDE_DIRS)
MARK_AS_ADVANCED(SDRP_LIBRARIES SDRP_INCLUDE_DIRS)

