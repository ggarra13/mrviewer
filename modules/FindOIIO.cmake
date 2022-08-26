# - Find OpenImageIO
#
# This module finds if OpenColorIO is installed and determines where the
# include files and libraries are. It also determines what the name of the
# library is. This code sets the following variables:
#
#  OIIO_INCLUDE_PATH = path to where imageio.h can be found
#  OIIO_LIBRARIES    = full path to OpenImageIO libraries
#

SET( OIIO_FOUND "NO" )

if(OIIO_LIBRARY AND OIIO_INCLUDE_PATH)
   # Already in cache, be silent
   set(OIIO_FIND_QUIETLY TRUE)
endif (OIIO_LIBRARY AND OIIO_INCLUDE_PATH)

FIND_PATH(OIIO_INCLUDE_PATH
  NAMES OpenImageIO/imageio.h
  PATHS $ENV{OIIO_INCLUDE_DIR}
	$ENV{OIIO_ROOT}/include
	${OIIO_INCLUDE_DIR}
	${CMAKE_PREFIX_PATH}/include
      )

      set( DEBUG_EXT )
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
      set( DEBUG_EXT _d )
endif()


FIND_LIBRARY(OIIO_LIBRARY
  NAMES OpenImageIO${DEBUG_EXT}
  PATHS $ENV{OIIO_LIBRARY_DIR}
	$ENV{OIIO_ROOT}/lib
	${OIIO_LIBRARY_DIR}/lib64
	${OIIO_LIBRARY_DIR}/lib
	${CMAKE_PREFIX_PATH}/lib64
	${CMAKE_PREFIX_PATH}/lib
	NO_SYSTEM_PATHS
      )

FIND_LIBRARY(OIIO_UTIL_LIBRARY
  NAMES OpenImageIO_Util${DEBUG_EXT}
  PATHS $ENV{OIIO_LIBRARY_DIR}
	$ENV{OIIO_ROOT}/lib
	${OIIO_LIBRARY_DIR}/lib64
	${OIIO_LIBRARY_DIR}/lib
	${CMAKE_PREFIX_PATH}/lib64
	${CMAKE_PREFIX_PATH}/lib
	NO_SYSTEM_PATHS
  )

SET( OIIO_LIBRARIES ${OIIO_LIBRARY} ${OIIO_UTIL_LIBRARY} )

IF(NOT OIIO_FOUND)
  IF (OIIO_INCLUDE_PATH)
    IF(OIIO_LIBRARY)
      SET(OIIO_FOUND "YES")
      IF( NOT OIIO_LIBRARY_DIR )
	GET_FILENAME_COMPONENT(OIIO_LIBRARY_DIR "${OIIO_LIBRARY}" PATH)
      ENDIF( NOT OIIO_LIBRARY_DIR )
      IF(WIN32)
	ADD_DEFINITIONS( "-DOIIO_DLL" )
      ENDIF(WIN32)
    ENDIF(OIIO_LIBRARY)
  ENDIF(OIIO_INCLUDE_PATH)
ENDIF(NOT OIIO_FOUND)

IF(NOT OIIO_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT OIIO_FIND_QUIETLY)
    IF(OIIO_FIND_REQUIRED)
      MESSAGE( STATUS "OIIO_INCLUDE_PATH=${OIIO_INCLUDE_PATH}" )
      MESSAGE( STATUS "OIIO_LIBRARIES=${OIIO_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
	      "Oiio required, please specify its location with OIIO_ROOT.")
    ELSE(OIIO_FIND_REQUIRED)
      MESSAGE(STATUS "Oiio was not found.")
    ENDIF(OIIO_FIND_REQUIRED)
  ENDIF(NOT OIIO_FIND_QUIETLY)
ENDIF(NOT OIIO_FOUND)


MARK_AS_ADVANCED(
  OIIO_LIBRARY
  OIIO_INCLUDE_PATH
  )
