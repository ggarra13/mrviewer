#-*-cmake-*-
#
# Test for OpenEXR
#
# Once loaded this will define
#  OPENEXR_FOUND        - system has OpenEXR
#  OPENEXR_INCLUDE_DIR  - include directory for OpenEXR
#  OPENEXR_LIBRARY_DIR  - library directory for OpenEXR
#  OPENEXR_LIBRARIES    - libraries you need to link to
#

SET(OPENEXR_FOUND "NO")

IF( OPENEXR_LIBRARY_DIR )
  SET( SEARCH_DIRS "${OPENEXR_LIBRARY_DIR}" )
ELSE( OPENEXR_LIBRARY_DIR )
  SET( SEARCH_DIRS
    $ENV{OPENEXR_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Release
    $ENV{OPENEXR_ROOT}/lib/Win32/Release
    $ENV{OPENEXR_ROOT}/lib
    $ENV{OPENEXR_ROOT}/lib/Release
    $ENV{OPENEXR_ROOT}/lib/Debug
    $ENV{OPENEXR_ROOT}/bin/x${CMAKE_BUILD_ARCH}/Release
    $ENV{OPENEXR_ROOT}/bin/Win32/Release
    $ENV{OPENEXR_ROOT}/bin/Release
    $ENV{OPENEXR_ROOT}/bin/Debug
    ${CMAKE_PREFIX_PATH}/lib${CMAKE_BUILD_ARCH}
    ${CMAKE_PREFIX_PATH}/lib
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    )
ENDIF( OPENEXR_LIBRARY_DIR )


FIND_PATH( OPENEXR_INCLUDE_DIR ImfHeader.h
  "$ENV{OPENEXR_ROOT}/include/OpenEXR"
  "$ENV{OPENEXR_ROOT}/include"
  "${CMAKE_PREFIX_PATH}/include/OpenEXR"
  /usr/local/include/OpenEXR
  /usr/include/OpenEXR
  DOC   "OpenEXR includes"
  )


FIND_PATH( IMATH_INCLUDE_DIR ImathForward.h
  "$ENV{OPENEXR_ROOT}/include/Imath"
  "$ENV{OPENEXR_ROOT}/include/OpenEXR"
  "$ENV{OPENEXR_ROOT}/include"
  ${CMAKE_PREFIX_PATH}/include/Imath
  ${CMAKE_PREFIX_PATH}/include/OpenEXR
  ${CMAKE_PREFIX_PATH}/include
  /usr/local/include/Imath
  /usr/local/include/OpenEXR
  /usr/include/Imath
  /usr/include/OpenEXR
  DOC   "Imath includes"
  )

SET( OPENEXR_INCLUDE_DIR ${OPENEXR_INCLUDE_DIR} ${IMATH_INCLUDE_DIR} )

FIND_LIBRARY( OpenEXRUtil
  NAMES OpenEXRUtil-3_1
  PATHS ${SEARCH_DIRS}
  DOC   "OpenEXR Util library"
)

FIND_LIBRARY( OpenEXR
  NAMES OpenEXR-3_1
  PATHS ${SEARCH_DIRS}
  DOC   "OpenEXR library"
)

MESSAGE( "OpenEXR Root=$ENV{OPENEXR_ROOT} SEARCH_DIRS=${SEARCH_DIRS} IlmImf=" ${IlmImf} )

FIND_LIBRARY( Imath
  NAMES Imath-3_2
  PATHS ${SEARCH_DIRS}
  DOC   "OpenEXR Imath library"
)

FIND_LIBRARY( Iex
  NAMES Iex-3_1
  PATHS ${SEARCH_DIRS}
  DOC   "OpenEXR Iex library"
)

FIND_LIBRARY( OpenEXRCore
  NAMES OpenEXRCore-3_1
  PATHS ${SEARCH_DIRS}
  DOC   "OpenEXR Core library"
)


SET(OPENEXR_LIBRARIES ${OpenEXRUtil} ${OpenEXR} ${OpenEXRCore} ${Imath} ${Iex} )

IF(WIN32 OR WIN64)
  ADD_DEFINITIONS( "-DOPENEXR_DLL" )
ENDIF(WIN32 OR WIN64)


IF(NOT OPENEXR_FOUND)
  IF (OPENEXR_INCLUDE_DIR)
    IF(OPENEXR_LIBRARIES)
      SET(OPENEXR_FOUND "YES")
      IF( NOT OPENEXR_LIBRARY_DIR )
	GET_FILENAME_COMPONENT(OPENEXR_LIBRARY_DIR "${OpenEXR}" PATH)
      ENDIF( NOT OPENEXR_LIBRARY_DIR )
    ENDIF(OPENEXR_LIBRARIES)
  ENDIF(OPENEXR_INCLUDE_DIR)
ENDIF(NOT OPENEXR_FOUND)

IF(NOT OPENEXR_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT OPENEXR_FIND_QUIETLY)
    IF(OPENEXR_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
	      "OpenEXR required, please specify its location with OPENEXR_ROOT.")
    ELSE(OPENEXR_FIND_REQUIRED)
      MESSAGE( STATUS "OpenEXR was not found!!! " ${OPENEXR_INCLUDE_DIR})
      MESSAGE( STATUS ${OPENEXR_LIBRARIES} )
    ENDIF(OPENEXR_FIND_REQUIRED)
  ENDIF(NOT OPENEXR_FIND_QUIETLY)
ENDIF(NOT OPENEXR_FOUND)


#
# We search for IlmThread only in the directory of IlmImf
#
# This is to avoid picking IlmThread for a wrong version of IlmImf.
#
FIND_LIBRARY( IlmThread
  NAMES IlmThread-3_1
  PATHS ${OPENEXR_LIBRARY_DIR}
  DOC   "OpenEXR IlmThread library (1.5 or later)"
  )

IF( NOT IlmThread )
  SET( IlmThread "" )
ENDIF( NOT IlmThread )

SET(OPENEXR_LIBRARIES ${OPENEXR_LIBRARIES} ${IlmThread} )

#####
