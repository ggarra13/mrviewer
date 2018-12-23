#-*-cmake-*-
#
# Test for OCIO (Color Transform Language)
#
# Once loaded this will define
#  OCIO_FOUND        - system has OpenEXR
#  OCIO_INCLUDE_DIR  - include directory for OpenEXR
#  OCIO_LIBRARIES    - libraries you need to link to
#

SET(OCIO_FOUND "NO")


IF( OCIO_LIBRARY_DIR )
  SET( SEARCH_DIRS "${OCIO_LIBRARY_DIR}" )
ELSE( OCIO_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{OCIO_ROOT}/${CMAKE_BUILD_TYPE}/lib/"
    "$ENV{OCIO_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "$ENV{OCIO_ROOT}/lib"
    "$ENV{OCIO_ROOT}/${CMAKE_BUILD_TYPE}/bin"
    "$ENV{OCIO_ROOT}/bin/${CMAKE_BUILD_TYPE}"
    "$ENV{OCIO_ROOT}/lib/"
    "$ENV{OCIO_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    )
ENDIF( OCIO_LIBRARY_DIR )


FIND_PATH( OCIO_INCLUDE_DIR OpenColorIO/OpenColorIO.h
  "$ENV{OCIO_ROOT}/include"
  /usr/local/include/OpenColorIO
  /usr/include/
  DOC   "OCIO includes" 
 )

FIND_LIBRARY( OpenColorIO
  NAMES OpenColorIO
  PATHS ${SEARCH_DIRS}
  NO_SYSTEM_PATH
  DOC   "OCIO library"
)


SET(OCIO_LIBRARIES ${OpenColorIO} )



IF(NOT OCIO_FOUND)
  IF (OCIO_INCLUDE_DIR)
    IF(OCIO_LIBRARIES)
      SET(OCIO_FOUND "YES")
    ENDIF(OCIO_LIBRARIES)
  ENDIF(OCIO_INCLUDE_DIR)
ENDIF(NOT OCIO_FOUND)

IF(NOT OCIO_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT OCIO_FIND_QUIETLY)
    IF(OCIO_FIND_REQUIRED)
      MESSAGE( STATUS "OCIO_INCLUDE_DIR ${OCIO_INCLUDE_DIR}" )
      MESSAGE( STATUS "OCIO_LIBRARIES   ${OCIO_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "OCIO required, please specify its location with OCIO_ROOT.")
    ELSE(OCIO_FIND_REQUIRED)
      MESSAGE(STATUS "OCIO was not found.")
    ENDIF(OCIO_FIND_REQUIRED)
  ENDIF(NOT OCIO_FIND_QUIETLY)
ENDIF(NOT OCIO_FOUND)

#####

