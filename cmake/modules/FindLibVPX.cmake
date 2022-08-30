#-*-cmake-*-
#
# Test for LibVPX (Color Transform Language)
#
# Once loaded this will define
#  LibVPX_FOUND        - system has OpenEXR
#  LibVPX_INCLUDE_DIR  - include directory for OpenEXR
#  LibVPX_LIBRARIES    - libraries you need to link to
#

SET(LibVPX_FOUND "NO")


IF( LibVPX_LIBRARY_DIR )
  SET( SEARCH_DIRS "${LibVPX_LIBRARY_DIR}" )
ELSE( LibVPX_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{LibVPX_ROOT}/${CMAKE_BUILD_TYPE}/lib/"
    "$ENV{LibVPX_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "$ENV{LibVPX_ROOT}/lib"
    "$ENV{LibVPX_ROOT}/${CMAKE_BUILD_TYPE}/bin"
    "$ENV{LibVPX_ROOT}/bin/${CMAKE_BUILD_TYPE}"
    "$ENV{LibVPX_ROOT}/lib/"
    "$ENV{LibVPX_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /lib/x86_64-linux-gnu/
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    /usr/lib/x86_64-linux-gnu/
    )
ENDIF( LibVPX_LIBRARY_DIR )


FIND_PATH( LibVPX_INCLUDE_DIR vpx_decoder.h
  "$ENV{LibVPX_ROOT}/include"
  /usr/local/include/
  /usr/include/
  PATH_SUFFIXES vpx
  DOC   "LibVPX includes" 
 )

FIND_LIBRARY( LibVPX
  NAMES vpx
  PATHS ${SEARCH_DIRS}
  DOC   "LibVPX library"
)


SET(LibVPX_LIBRARIES ${LibVPX} )


IF(NOT LibVPX_FOUND)
  IF (LibVPX_INCLUDE_DIR)
    IF(LibVPX_LIBRARIES)
      SET(LibVPX_FOUND "YES")
    ENDIF(LibVPX_LIBRARIES)
  ENDIF(LibVPX_INCLUDE_DIR)
ENDIF(NOT LibVPX_FOUND)

IF(NOT LibVPX_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT LibVPX_FIND_QUIETLY)
    IF(LibVPX_FIND_REQUIRED)
      MESSAGE( STATUS "LibVPX_INCLUDE_DIR ${LibVPX_INCLUDE_DIR}" )
      MESSAGE( STATUS "LibVPX_LIBRARIES   ${LibVPX_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "LibVPX required, please specify its location with LibVPX_ROOT.")
    ELSE(LibVPX_FIND_REQUIRED)
      MESSAGE(STATUS "LibVPX was not found.")
    ENDIF(LibVPX_FIND_REQUIRED)
  ENDIF(NOT LibVPX_FIND_QUIETLY)
ENDIF(NOT LibVPX_FOUND)

#####

