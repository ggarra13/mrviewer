#-*-cmake-*-
#
# Test for ACESclip libraries
#
# Once loaded this will define
#  ACESclip_FOUND        - system has ACESclip
#  ACESclip_INCLUDE_DIR  - include directory for ACESclip
#  ACESclip_LIBRARY_DIR  - library directory for ACESclip
#  ACESclip_LIBRARIES    - libraries you need to link to
#

SET(ACESclip_FOUND "NO")

FIND_PATH( ACESclip_INCLUDE_DIR ao.h
  $ENV{ACESclip_LOCATION}
  $ENV{ACESclip_LOCATION}/ao
  $ENV{ACESclip_LOCATION}/include/ao
  /usr/local/include/ao
  /usr/include/ao
  )

FIND_LIBRARY( ACESclip     ACESclip
  PATHS $ENV{ACESclip_LOCATION}/lib${CMAKE_BUILD_ARCH}
        /usr/local/lib${CMAKE_BUILD_ARCH}
        /usr/lib${CMAKE_BUILD_ARCH}
  DOC   "ACESclip library"
)

SET( ACESclip_LIBRARIES ${ACESclip} )


IF (ACESclip_INCLUDE_DIR)
  IF(ACESclip_LIBRARIES)
    SET(ACESclip_FOUND "YES")
    GET_FILENAME_COMPONENT(ACESclip_LIBRARY_DIR ${ACESclip} PATH)
  ENDIF(ACESclip_LIBRARIES)
ENDIF(ACESclip_INCLUDE_DIR)

IF(NOT ACESclip_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT ACESclip_FIND_QUIETLY)
    IF(ACESclip_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "libao required, please specify it's location with ACESclip_LOCATION")
    ELSE(ACESclip_FIND_REQUIRED)
      MESSAGE(STATUS "libao was not found.")
    ENDIF(ACESclip_FIND_REQUIRED)
  ENDIF(NOT ACESclip_FIND_QUIETLY)
ENDIF(NOT ACESclip_FOUND)


#####

