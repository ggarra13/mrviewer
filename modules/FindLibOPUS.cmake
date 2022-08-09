#-*-cmake-*-
#
# Test for LibOPUS (Color Transform Language)
#
# Once loaded this will define
#  LibOPUS_FOUND        - system has OpenEXR
#  LibOPUS_INCLUDE_DIR  - include directory for OpenEXR
#  LibOPUS_LIBRARIES    - libraries you need to link to
#

SET(LibOPUS_FOUND "NO")


IF( LibOPUS_LIBRARY_DIR )
  SET( SEARCH_DIRS "${LibOPUS_LIBRARY_DIR}" )
ELSE( LibOPUS_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{LibOPUS_ROOT}/${CMAKE_BUILD_TYPE}/lib/"
    "$ENV{LibOPUS_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "$ENV{LibOPUS_ROOT}/lib"
    "$ENV{LibOPUS_ROOT}/${CMAKE_BUILD_TYPE}/bin"
    "$ENV{LibOPUS_ROOT}/bin/${CMAKE_BUILD_TYPE}"
    "$ENV{LibOPUS_ROOT}/lib/"
    "$ENV{LibOPUS_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /lib/x86_64-linux-gnu/
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    /usr/lib/x86_64-linux-gnu/
    )
ENDIF( LibOPUS_LIBRARY_DIR )


FIND_PATH( LibOPUS_INCLUDE_DIR opus.h
  "$ENV{LibOPUS_ROOT}/include/opus"
  ${CMAKE_PREFIX_PATH}/include/opus
  /usr/local/include/opus
  /usr/include/opus
  PATH_SUFFIXES vpx
  DOC   "LibOPUS includes" 
 )

FIND_LIBRARY( LibOPUS
  NAMES opus
  PATHS ${SEARCH_DIRS}
  DOC   "LibOPUS library"
)


SET(LibOPUS_LIBRARIES ${LibOPUS} )


IF(NOT LibOPUS_FOUND)
  IF (LibOPUS_INCLUDE_DIR)
    IF(LibOPUS_LIBRARIES)
      SET(LibOPUS_FOUND "YES")
    ENDIF(LibOPUS_LIBRARIES)
  ENDIF(LibOPUS_INCLUDE_DIR)
ENDIF(NOT LibOPUS_FOUND)

IF(NOT LibOPUS_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT LibOPUS_FIND_QUIETLY)
    IF(LibOPUS_FIND_REQUIRED)
      MESSAGE( STATUS "LibOPUS_INCLUDE_DIR ${LibOPUS_INCLUDE_DIR}" )
      MESSAGE( STATUS "LibOPUS_LIBRARIES   ${LibOPUS_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "LibOPUS required, please specify its location with LibOPUS_ROOT.")
    ELSE(LibOPUS_FIND_REQUIRED)
      MESSAGE(STATUS "LibOPUS was not found.")
    ENDIF(LibOPUS_FIND_REQUIRED)
  ENDIF(NOT LibOPUS_FIND_QUIETLY)
ENDIF(NOT LibOPUS_FOUND)

#####

