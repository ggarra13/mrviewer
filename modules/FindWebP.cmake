#-*-cmake-*-
#
# Test for WEBP (Color Transform Language)
#
# Once loaded this will define
#  WEBP_FOUND        - system has OpenEXR
#  WEBP_INCLUDE_DIR  - include directory for OpenEXR
#  WEBP_LIBRARIES    - libraries you need to link to
#

SET(WEBP_FOUND "NO")


IF( WEBP_LIBRARY_DIR )
  SET( SEARCH_DIRS "${WEBP_LIBRARY_DIR}" )
ELSE( WEBP_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{WEBP_ROOT}/${CMAKE_BUILD_TYPE}/lib/"
    "$ENV{WEBP_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "$ENV{WEBP_ROOT}/lib"
    "$ENV{WEBP_ROOT}/${CMAKE_BUILD_TYPE}/bin"
    "$ENV{WEBP_ROOT}/bin/${CMAKE_BUILD_TYPE}"
    "$ENV{WEBP_ROOT}/lib/"
    "$ENV{WEBP_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /lib/x86_64-linux-gnu/
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    /usr/lib/x86_64-linux-gnu/
    )
ENDIF( WEBP_LIBRARY_DIR )


FIND_PATH( WEBP_INCLUDE_DIR decode.h
  "$ENV{WEBP_ROOT}/include/webp"
  ${CMAKE_PREFIX_PATH}/include/webp
  /usr/local/include/webp
  /usr/include/webp
  DOC   "WEBP includes" 
 )

FIND_LIBRARY( WEBP
  NAMES webp
  PATHS ${SEARCH_DIRS}
  DOC   "WEBP library"
)


SET(WEBP_LIBRARIES ${WEBP} )


IF(NOT WEBP_FOUND)
  IF (WEBP_INCLUDE_DIR)
    IF(WEBP_LIBRARIES)
      SET(WEBP_FOUND "YES")
    ENDIF(WEBP_LIBRARIES)
  ENDIF(WEBP_INCLUDE_DIR)
ENDIF(NOT WEBP_FOUND)

IF(NOT WEBP_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT WEBP_FIND_QUIETLY)
    IF(WEBP_FIND_REQUIRED)
      MESSAGE( STATUS "WEBP_INCLUDE_DIR ${WEBP_INCLUDE_DIR}" )
      MESSAGE( STATUS "WEBP_LIBRARIES   ${WEBP_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "WEBP required, please specify its location with WEBP_ROOT.")
    ELSE(WEBP_FIND_REQUIRED)
      MESSAGE(STATUS "WEBP was not found.")
    ENDIF(WEBP_FIND_REQUIRED)
  ENDIF(NOT WEBP_FIND_QUIETLY)
ENDIF(NOT WEBP_FOUND)

#####

