#-*-cmake-*-
#
# Test for GLUX libraries
#
# Once loaded this will define
#  GLUX_FOUND        - system has GLUX
#  GLUX_INCLUDE_DIR  - include directory for GLUX
#  GLUX_LIBRARY_DIR  - library directory for GLUX
#  GLUX_LIBRARIES    - libraries you need to link to
#

SET(GLUX_FOUND "NO")

FIND_PATH( GLUX_INCLUDE_DIR glux.h
  $ENV{GLUX_LOCATION}
  $ENV{GLUX_LOCATION}/include
  /usr/include/glux
  /usr/local/glux/include
  )

FIND_LIBRARY( GLUX     glux
  PATHS $ENV{GLUX_LOCATION}/lib
  /usr/local/glux/lib
  DOC   "GLUX library"
)

SET( GLUX_LIBRARIES ${GLUX} )


IF (GLUX_INCLUDE_DIR)
  IF(GLUX_LIBRARIES)
    SET(GLUX_FOUND "YES")
    GET_FILENAME_COMPONENT(GLUX_LIBRARY_DIR ${GLUX} PATH)
  ENDIF(GLUX_LIBRARIES)
ENDIF(GLUX_INCLUDE_DIR)

IF(NOT GLUX_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT GLUX_FIND_QUIETLY)
    IF(GLUX_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "libglux required, please specify it's location with GLUX_LOCATION")
    ELSE(GLUX_FIND_REQUIRED)
      MESSAGE(STATUS "libglux was not found.")
    ENDIF(GLUX_FIND_REQUIRED)
  ENDIF(NOT GLUX_FIND_QUIETLY)
ENDIF(NOT GLUX_FOUND)


#####

