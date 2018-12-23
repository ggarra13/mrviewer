#-*-cmake-*-
#
# Test for SFIO libraries
#
# Once loaded this will define
#  SFIO_FOUND        - system has SFIO
#  SFIO_INCLUDE_DIR  - include directory for SFIO
#  SFIO_LIBRARY_DIR  - library directory for SFIO
#  SFIO_LIBRARIES    - libraries you need to link to
#

SET(SFIO_FOUND "NO")

FIND_PATH( SFIO_INCLUDE_DIR sfio.h
  $ENV{SFIO_LOCATION}/include
  $ENV{SFIO_LOCATION}/sfio
  $ENV{SFIO_LOCATION}/include/sfio
  /usr/include/sfio
  )

MESSAGE( STATUS ${SFIO_INCLUDE_DIR} )

FIND_LIBRARY( SFIO_IO     sfio
  PATHS $ENV{SFIO_LOCATION}/lib
  DOC   "SFIO io library"
)

FIND_LIBRARY( SFIO_STDIO     stdio
  PATHS $ENV{SFIO_LOCATION}/lib
  DOC   "SFIO stdio library"
)

SET( SFIO_LIBRARIES ${SFIO_IO} ${SFIO_STDIO} )


IF (SFIO_INCLUDE_DIR)
  IF(SFIO_LIBRARIES)
    SET(SFIO_FOUND "YES")
    GET_FILENAME_COMPONENT(SFIO_LIBRARY_DIR ${SFIO_IO} PATH)
  ENDIF(SFIO_LIBRARIES)
ENDIF(SFIO_INCLUDE_DIR)

IF(NOT SFIO_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT SFIO_FIND_QUIETLY)
    IF(SFIO_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "libsfio required, please specify it's location with SFIO_LOCATION")
    ELSE(SFIO_FIND_REQUIRED)
      MESSAGE(STATUS "libsfio was not found.")
    ENDIF(SFIO_FIND_REQUIRED)
  ENDIF(NOT SFIO_FIND_QUIETLY)
ENDIF(NOT SFIO_FOUND)


#####

