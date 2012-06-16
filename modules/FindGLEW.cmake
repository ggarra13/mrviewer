#-*-cmake-*-
#
# Test for GLEW libraries
#
# Once loaded this will define
#  GLEW_FOUND        - system has GLEW
#  GLEW_INCLUDE_DIR  - include directory for GLEW
#  GLEW_LIBRARY_DIR  - library directory for GLEW
#  GLEW_LIBRARIES    - libraries you need to link to
#

SET(GLEW_FOUND "NO")

MESSAGE( STATUS "GLEW ROOT=" $ENV{GLEW_ROOT} )

FIND_PATH( GLEW_INCLUDE_DIR GL/glew.h
  $ENV{GLEW_ROOT}/include
  $ENV{GLEW_ROOT}
  /usr/include/glew
  /usr/local/include/glew
  /usr/local/glew/include
  )

FIND_LIBRARY( GLEW     glew32 glew GLEW
  PATHS 
  $ENV{GLEW_ROOT}/lib
  $ENV{GLEW_ROOT}
  /usr/local/glew/lib
  /usr/lib${CMAKE_BUILD_ARCH}
  /usr/lib
  DOC   "GLEW library"
)

SET( GLEW_LIBRARIES ${GLEW} )


IF (GLEW_INCLUDE_DIR)
  IF(GLEW_LIBRARIES)
    SET(GLEW_FOUND "YES")
    GET_FILENAME_COMPONENT(GLEW_LIBRARY_DIR ${GLEW} PATH)
  ENDIF(GLEW_LIBRARIES)
ENDIF(GLEW_INCLUDE_DIR)

IF(NOT GLEW_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT GLEW_FIND_QUIETLY)
    IF(GLEW_FIND_REQUIRED)
      MESSAGE( STATUS "GLEW_INCLUDE_DIR=" ${GLEW_INCLUDE_DIR} )
      MESSAGE( STATUS "GLEW_LIBRARIES=" ${GLEW_LIBRARIES} )
      MESSAGE(FATAL_ERROR
              "GLEW required, please specify it's location with GLEW_ROOT")
    ELSE(GLEW_FIND_REQUIRED)
      MESSAGE(STATUS "libGLee was not found.")
    ENDIF(GLEW_FIND_REQUIRED)
  ENDIF(NOT GLEW_FIND_QUIETLY)
ENDIF(NOT GLEW_FOUND)


#####

