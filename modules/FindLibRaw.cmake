#-*-cmake-*-
#
# Test for LibRaw (Color Transform Language)
#
# Once loaded this will define
#  LibRaw_FOUND        - system has OpenEXR
#  LibRaw_INCLUDE_DIR  - include directory for OpenEXR
#  LibRaw_LIBRARIES    - libraries you need to link to
#

SET(LibRaw_FOUND "NO")


IF( LibRaw_LIBRARY_DIR )
  SET( SEARCH_DIRS "${LibRaw_LIBRARY_DIR}" )
ELSE( LibRaw_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{LibRaw_ROOT}/${CMAKE_BUILD_TYPE}/lib/"
    "$ENV{LibRaw_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "$ENV{LibRaw_ROOT}/lib"
    "$ENV{LibRaw_ROOT}/${CMAKE_BUILD_TYPE}/bin"
    "$ENV{LibRaw_ROOT}/bin/${CMAKE_BUILD_TYPE}"
    "$ENV{LibRaw_ROOT}/lib/"
    "$ENV{LibRaw_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    )
ENDIF( LibRaw_LIBRARY_DIR )


FIND_PATH( LibRaw_INCLUDE_DIR libraw/libraw.h
  "$ENV{LibRaw_ROOT}/include"
  /usr/local/include/
  /usr/include/
  DOC   "LibRaw includes" 
 )

FIND_LIBRARY( LibRaw
  NAMES raw libraw 
  PATHS ${SEARCH_DIRS}
  NO_SYSTEM_PATH
  DOC   "LibRaw library"
)


SET(LibRaw_LIBRARIES ${LibRaw} )


IF(NOT LibRaw_FOUND)
  IF (LibRaw_INCLUDE_DIR)
    IF(LibRaw_LIBRARIES)
      SET(LibRaw_FOUND "YES")
    ENDIF(LibRaw_LIBRARIES)
  ENDIF(LibRaw_INCLUDE_DIR)
ENDIF(NOT LibRaw_FOUND)

IF(NOT LibRaw_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT LibRaw_FIND_QUIETLY)
    IF(LibRaw_FIND_REQUIRED)
      MESSAGE( STATUS "LibRaw_INCLUDE_DIR ${LibRaw_INCLUDE_DIR}" )
      MESSAGE( STATUS "LibRaw_LIBRARIES   ${LibRaw_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "LibRaw required, please specify its location with LibRaw_ROOT.")
    ELSE(LibRaw_FIND_REQUIRED)
      MESSAGE(STATUS "LibRaw was not found.")
    ENDIF(LibRaw_FIND_REQUIRED)
  ENDIF(NOT LibRaw_FIND_QUIETLY)
ENDIF(NOT LibRaw_FOUND)

#####

