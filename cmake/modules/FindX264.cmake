#-*-cmake-*-
#
# Test for X264 (Color Transform Language)
#
# Once loaded this will define
#  X264_FOUND        - system has OpenEXR
#  X264_INCLUDE_DIR  - include directory for OpenEXR
#  X264_LIBRARIES    - libraries you need to link to
#

SET(X264_FOUND "NO")


IF( X264_LIBRARY_DIR )
  SET( SEARCH_DIRS "${X264_LIBRARY_DIR}" )
ELSE( X264_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{X264_ROOT}/${CMAKE_BUILD_TYPE}/lib/"
    "$ENV{X264_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "$ENV{X264_ROOT}/lib"
    "$ENV{X264_ROOT}/${CMAKE_BUILD_TYPE}/bin"
    "$ENV{X264_ROOT}/bin/${CMAKE_BUILD_TYPE}"
    "$ENV{X264_ROOT}/lib/"
    "$ENV{X264_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /lib/x86_64-linux-gnu/
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    /usr/lib/x86_64-linux-gnu/
    )
ENDIF( X264_LIBRARY_DIR )


FIND_PATH( X264_INCLUDE_DIR x264.h
  "$ENV{X264_ROOT}/include"
  /usr/local/include/
  /usr/include/
  DOC   "X264 includes" 
 )

FIND_LIBRARY( X264
  NAMES x264
  PATHS ${SEARCH_DIRS}
  DOC   "X264 library"
)


SET(X264_LIBRARIES ${X264} )


IF(NOT X264_FOUND)
  IF (X264_INCLUDE_DIR)
    IF(X264_LIBRARIES)
      SET(X264_FOUND "YES")
    ENDIF(X264_LIBRARIES)
  ENDIF(X264_INCLUDE_DIR)
ENDIF(NOT X264_FOUND)

IF(NOT X264_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT X264_FIND_QUIETLY)
    IF(X264_FIND_REQUIRED)
      MESSAGE( STATUS "X264_INCLUDE_DIR ${X264_INCLUDE_DIR}" )
      MESSAGE( STATUS "X264_LIBRARIES   ${X264_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "X264 required, please specify its location with X264_ROOT.")
    ELSE(X264_FIND_REQUIRED)
      MESSAGE(STATUS "X264 was not found.")
    ENDIF(X264_FIND_REQUIRED)
  ENDIF(NOT X264_FIND_QUIETLY)
ENDIF(NOT X264_FOUND)

#####

