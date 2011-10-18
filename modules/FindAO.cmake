#-*-cmake-*-
#
# Test for AO libraries
#
# Once loaded this will define
#  AO_FOUND        - system has AO
#  AO_INCLUDE_DIR  - include directory for AO
#  AO_LIBRARY_DIR  - library directory for AO
#  AO_LIBRARIES    - libraries you need to link to
#

SET(AO_FOUND "NO")

FIND_PATH( AO_INCLUDE_DIR ao.h
  $ENV{AO_LOCATION}
  $ENV{AO_LOCATION}/ao
  $ENV{AO_LOCATION}/include/ao
  /usr/local/include/ao
  /usr/include/ao
  )

FIND_LIBRARY( AO     ao2 ao
  PATHS $ENV{AO_LOCATION}/lib${CMAKE_BUILD_ARCH}
        /usr/local/lib${CMAKE_BUILD_ARCH}
        /usr/lib${CMAKE_BUILD_ARCH}
  DOC   "AO library"
)

SET( AO_LIBRARIES ${AO} )


IF (AO_INCLUDE_DIR)
  IF(AO_LIBRARIES)
    SET(AO_FOUND "YES")
    GET_FILENAME_COMPONENT(AO_LIBRARY_DIR ${AO} PATH)
  ENDIF(AO_LIBRARIES)
ENDIF(AO_INCLUDE_DIR)

IF(NOT AO_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT AO_FIND_QUIETLY)
    IF(AO_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "libao required, please specify it's location with AO_LOCATION")
    ELSE(AO_FIND_REQUIRED)
      MESSAGE(STATUS "libao was not found.")
    ENDIF(AO_FIND_REQUIRED)
  ENDIF(NOT AO_FIND_QUIETLY)
ENDIF(NOT AO_FOUND)


#####

