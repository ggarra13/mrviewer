# - Find SWIG
# This module finds an installed SWIG.  It sets the following variables:
#  SWIG_FOUND - set to true if SWIG is found
#  SWIG_VERSION - set to version of SWIG found
#  SWIG_MAJOR_VERSION
#  SWIG_MINOR_VERSION
#  SWIG_PATCH_VERSION
#  SWIG_DIR - the directory where swig is installed
#  SWIG_EXECUTABLE - the path to the swig executable

SET(SWIG_FOUND FOOBAR)
FIND_PATH(SWIG_DIR
  SWIGConfig.cmake
  /usr/local/share/swig/1.3.32
  /usr/local/share/swig/1.3.31
  /usr/local/share/swig/1.3.30
  /usr/local/share/swig/1.3.29
  /usr/local/share/swig1.3
  /usr/share/swig1.3
  /usr/lib/swig1.3)
FIND_PATH(SWIG_DIR
  swig.swg
  /usr/local/share/swig/1.3.32
  /usr/local/share/swig/1.3.31
  /usr/local/share/swig/1.3.30
  /usr/local/share/swig/1.3.29
  /usr/local/share/swig1.3
  /usr/share/swig1.3
  /usr/lib/swig1.3
  "$ENV{PROGRAMFILES}/swig/1.3.32"
  "$ENV{PROGRAMFILES}/swig/1.3.31"
  "$ENV{PROGRAMFILES}/swig/1.3.30"
  "$ENV{PROGRAMFILES}/swig/1.3.29"
  "$ENV{PROGRAMFILES}/swig1.3"
  "C:/cygwin/usr/local/share/swig/1.3.32"
  "C:/cygwin/usr/local/share/swig/1.3.31"
  "C:/cygwin/usr/local/share/swig/1.3.30"
  "C:/cygwin/usr/local/share/swig/1.3.29"
  "C:/cygwin/usr/share/swig/1.3.32"
  "C:/cygwin/usr/share/swig/1.3.31"
  "C:/cygwin/usr/share/swig/1.3.30"
  "C:/cygwin/usr/share/swig/1.3.29"
  "C:/cygwin/usr/local/share/swig1.3"
  )


IF(EXISTS ${SWIG_DIR})
  IF("x${SWIG_DIR}x" STREQUAL "x${CMAKE_ROOT}/Modulesx")
    MESSAGE("SWIG_DIR should not be modules subdirectory of CMake")
  ENDIF("x${SWIG_DIR}x" STREQUAL "x${CMAKE_ROOT}/Modulesx")

  IF(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
    INCLUDE(${SWIG_DIR}/SWIGConfig.cmake)
    SET(SWIG_FOUND 1)
  ELSE(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
    FIND_PROGRAM(SWIG_EXECUTABLE
      NAMES swig1.3 swig
      PATHS ${SWIG_DIR} "${SWIG_DIR}/.." "${SWIG_DIR}/../../bin" "${SWIG_DIR}/../../../bin" )
    FIND_PATH( SWIG_USE_FILE_PATH 
      NAME  UseSWIG.cmake
      PATHS ${CMAKE_MODULE_PATH} ${CMAKE_ROOT}/Modules )
    SET( SWIG_USE_FILE "${SWIG_USE_FILE_PATH}/UseSWIG.cmake" )
  ENDIF(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
ENDIF(EXISTS ${SWIG_DIR})

IF("x${SWIG_FOUND}x" STREQUAL "xFOOBARx")
  SET(SWIG_FOUND 0)
  IF(EXISTS ${SWIG_DIR})
    IF(EXISTS ${SWIG_USE_FILE})
      IF(EXISTS ${SWIG_EXECUTABLE})
        SET(SWIG_FOUND 1)
      ENDIF(EXISTS ${SWIG_EXECUTABLE})
    ENDIF(EXISTS ${SWIG_USE_FILE})
  ENDIF(EXISTS ${SWIG_DIR})
  IF(NOT ${SWIG_FOUND})
    IF(${SWIG_FIND_REQUIRED})
      MESSAGE(FATAL_ERROR "Swig was not found on the system. Please specify the location of Swig.")
    ENDIF(${SWIG_FIND_REQUIRED})
  ENDIF(NOT ${SWIG_FOUND})
ENDIF("x${SWIG_FOUND}x" STREQUAL "xFOOBARx")


IF( SWIG_FOUND )
  EXECUTE_PROCESS( 
    COMMAND ${SWIG_EXECUTABLE} -version 
    OUTPUT_VARIABLE version
    )

  STRING( REGEX REPLACE ".*SWIG Version[\t ]+([0-9.]+).*" "\\1" 
    SWIG_VERSION ${version} )

  STRING( REGEX MATCH "^[0-9]+" SWIG_MAJOR_VERSION ${SWIG_VERSION} )
  STRING( REGEX MATCH "[0-9]+$" SWIG_PATCH_VERSION ${SWIG_VERSION} )
  STRING( REGEX REPLACE "^${SWIG_MAJOR_VERSION}\\.([0-9]+).*" "\\1"
    SWIG_MINOR_VERSION ${SWIG_VERSION} )

ENDIF( SWIG_FOUND )
