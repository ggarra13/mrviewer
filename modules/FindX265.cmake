#-*-cmake-*-
#
# Test for X265 (Color Transform Language)
#
# Once loaded this will define
#  X265_FOUND        - system has OpenEXR
#  X265_INCLUDE_DIR  - include directory for OpenEXR
#  X265_LIBRARIES    - libraries you need to link to
#

SET(X265_FOUND "NO")


IF( X265_LIBRARY_DIR )
  SET( SEARCH_DIRS "${X265_LIBRARY_DIR}" )
ELSE( X265_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{X265_ROOT}/${CMAKE_BUILD_TYPE}/lib/"
    "$ENV{X265_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "$ENV{X265_ROOT}/lib"
    "$ENV{X265_ROOT}/${CMAKE_BUILD_TYPE}/bin"
    "$ENV{X265_ROOT}/bin/${CMAKE_BUILD_TYPE}"
    "$ENV{X265_ROOT}/lib/"
    "$ENV{X265_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /lib/x86_64-linux-gnu/
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    /usr/lib/x86_64-linux-gnu/
    )
ENDIF( X265_LIBRARY_DIR )


FIND_PATH( X265_INCLUDE_DIR x265.h
  "$ENV{X265_ROOT}/include"
  "${CMAKE_PREFIX_PATH}/include"
  /usr/local/include/
  /usr/include/
  DOC   "X265 includes" 
 )

FIND_LIBRARY( X265
  NAMES x265
  PATHS ${SEARCH_DIRS}
  DOC   "X265 library"
)


SET(X265_LIBRARIES ${X265} )


IF(NOT X265_FOUND)
  IF (X265_INCLUDE_DIR)
    IF(X265_LIBRARIES)
      SET(X265_FOUND "YES")
    ENDIF(X265_LIBRARIES)
  ENDIF(X265_INCLUDE_DIR)
ENDIF(NOT X265_FOUND)

IF(NOT X265_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT X265_FIND_QUIETLY)
    IF(X265_FIND_REQUIRED)
      MESSAGE( STATUS "X265_INCLUDE_DIR ${X265_INCLUDE_DIR}" )
      MESSAGE( STATUS "X265_LIBRARIES   ${X265_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "X265 required, please specify its location with X265_ROOT.")
    ELSE(X265_FIND_REQUIRED)
      MESSAGE(STATUS "X265 was not found.")
    ENDIF(X265_FIND_REQUIRED)
  ENDIF(NOT X265_FIND_QUIETLY)
ENDIF(NOT X265_FOUND)

#####

