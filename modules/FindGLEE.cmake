#-*-cmake-*-
#
# Test for GLEE libraries
#
# Once loaded this will define
#  GLEE_FOUND        - system has GLEE
#  GLEE_INCLUDE_DIR  - include directory for GLEE
#  GLEE_LIBRARY_DIR  - library directory for GLEE
#  GLEE_LIBRARIES    - libraries you need to link to
#

SET(GLEE_FOUND "NO")

FIND_PATH( GLEE_INCLUDE_DIR GLee.h
  $ENV{GLEE_ROOT}/include
  $ENV{GLEE_ROOT}
  /usr/include/GLee
  /usr/local/include/GLee
  /usr/local/GLee/include
  )

FIND_LIBRARY( GLEE     GLee glee
  PATHS 
  $ENV{GLEE_ROOT}/lib
  $ENV{GLEE_ROOT}
  /usr/local/GLee/lib
  DOC   "GLee library"
)

SET( GLEE_LIBRARIES ${GLEE} )


IF (GLEE_INCLUDE_DIR)
  IF(GLEE_LIBRARIES)
    SET(GLEE_FOUND "YES")
    GET_FILENAME_COMPONENT(GLEE_LIBRARY_DIR ${GLEE} PATH)
  ENDIF(GLEE_LIBRARIES)
ENDIF(GLEE_INCLUDE_DIR)

IF(NOT GLEE_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT GLEE_FIND_QUIETLY)
    IF(GLEE_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "libGLee required, please specify it's location with GLEE_ROOT")
    ELSE(GLEE_FIND_REQUIRED)
      MESSAGE(STATUS "libGLee was not found.")
    ENDIF(GLEE_FIND_REQUIRED)
  ENDIF(NOT GLEE_FIND_QUIETLY)
ENDIF(NOT GLEE_FOUND)


#####

