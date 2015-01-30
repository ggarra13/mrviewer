#-*-cmake-*-
#
# Test for Tinyxml2
#
# Once loaded this will define
#  TINYXML2_FOUND        - system has Tinyxml2
#  TINYXML2_INCLUDE_DIR  - include directory for Tinyxml2
#  TINYXML2_LIBRARY_DIR  - library directory for Tinyxml2
#  TINYXML2_LIBRARIES    - libraries you need to link to
#

SET(TINYXML2_FOUND "NO")

IF( TINYXML2_LIBRARY_DIR )
  SET( SEARCH_DIRS "${TINYXML2_LIBRARY_DIR}" )
ELSE( TINYXML2_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    $ENV{TINYXML2_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Release
    $ENV{TINYXML2_ROOT}/lib/Win32/Release
    $ENV{TINYXML2_ROOT}/lib
    $ENV{TINYXML2_ROOT}/lib/Release
    $ENV{TINYXML2_ROOT}/lib/Debug
    $ENV{TINYXML2_ROOT}/bin/x${CMAKE_BUILD_ARCH}/Release
    $ENV{TINYXML2_ROOT}/bin/Win32/Release
    $ENV{TINYXML2_ROOT}/bin/Release
    $ENV{TINYXML2_ROOT}/bin/Debug
    /usr/local/lib/x86_${CMAKE_BUILD_ARCH}-linux-gnu
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/lib${CMAKE_BUILD_ARCH}
    )
ENDIF( TINYXML2_LIBRARY_DIR )


FIND_PATH( TINYXML2_INCLUDE_DIR tinyxml2.h
  "$ENV{TINYXML2_ROOT}/include/tinyxml2"
  "$ENV{TINYXML2_ROOT}/include"
  /usr/local/include/tinyxml2
  /usr/include/tinyxml2
  DOC   "Tinyxml2 includes"
  )

FIND_LIBRARY( tinyxml2 
  NAMES tinyxml2
  PATHS ${SEARCH_DIRS}
  DOC   "Tinyxml2 library"
)


SET(TINYXML2_LIBRARIES ${tinyxml2}  )


IF(NOT TINYXML2_FOUND)
  IF (TINYXML2_INCLUDE_DIR)
    IF(TINYXML2_LIBRARIES)
      SET(TINYXML2_FOUND "YES")
      IF( NOT TINYXML2_LIBRARY_DIR )
	GET_FILENAME_COMPONENT(TINYXML2_LIBRARY_DIR "${tinyxml2}" PATH)
      ENDIF( NOT TINYXML2_LIBRARY_DIR )
    ENDIF(TINYXML2_LIBRARIES)
  ENDIF(TINYXML2_INCLUDE_DIR)
ENDIF(NOT TINYXML2_FOUND)

IF(NOT TINYXML2_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT TINYXML2_FIND_QUIETLY)
    IF(TINYXML2_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "Tinyxml2 required, please specify its location with TINYXML2_ROOT.")
    ELSE(TINYXML2_FIND_REQUIRED)
      MESSAGE( STATUS "Tinyxml2 was not found!!! " ${TINYXML2_INCLUDE_DIR})
      MESSAGE( STATUS ${TINYXML2_LIBRARIES} )
    ENDIF(TINYXML2_FIND_REQUIRED)
  ENDIF(NOT TINYXML2_FIND_QUIETLY)
ENDIF(NOT TINYXML2_FOUND)


#####

