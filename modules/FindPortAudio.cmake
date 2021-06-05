#-*-cmake-*-
#
# Test for Portaudio
#
# Once loaded this will define
#  PORTAUDIO_FOUND        - system has Portaudio
#  PORTAUDIO_INCLUDE_DIR  - include directory for Portaudio
#  PORTAUDIO_LIBRARY_DIR  - library directory for Portaudio
#  PORTAUDIO_LIBRARIES    - libraries you need to link to
#

SET(PORTAUDIO_FOUND "NO")

IF( PORTAUDIO_LIBRARY_DIR )
  SET( SEARCH_DIRS "${PORTAUDIO_LIBRARY_DIR}" )
ELSE( PORTAUDIO_LIBRARY_DIR )
  SET( SEARCH_DIRS
    $ENV{PORTAUDIO_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Release
    $ENV{PORTAUDIO_ROOT}/lib/Win32/Release
    $ENV{PORTAUDIO_ROOT}/lib
    $ENV{PORTAUDIO_ROOT}/lib/Release
    $ENV{PORTAUDIO_ROOT}/lib/Debug
    $ENV{PORTAUDIO_ROOT}/bin/x${CMAKE_BUILD_ARCH}/Release
    $ENV{PORTAUDIO_ROOT}/bin/Win32/Release
    $ENV{PORTAUDIO_ROOT}/bin/Release
    $ENV{PORTAUDIO_ROOT}/bin/Debug
    /usr/local/lib/x86_${CMAKE_BUILD_ARCH}-linux-gnu
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/local/lib
    /usr/lib${CMAKE_BUILD_ARCH}
    /usr/lib
    )
ENDIF( PORTAUDIO_LIBRARY_DIR )


FIND_PATH( PORTAUDIO_INCLUDE_DIR portaudio.h
  "$ENV{PORTAUDIO_ROOT}/include/portaudio"
  "$ENV{PORTAUDIO_ROOT}/include"
  /usr/local/include/portaudio
  /usr/local/include
  /usr/include/portaudio
  /usr/include
  DOC   "Portaudio includes"
  )

FIND_LIBRARY( portaudio
  NAMES portaudio_x64 portaudio
  PATHS ${SEARCH_DIRS}
  DOC   "Portaudio library"
)


SET(PORTAUDIO_LIBRARIES ${portaudio}  )


IF(NOT PORTAUDIO_FOUND)
  IF (PORTAUDIO_INCLUDE_DIR)
    IF(PORTAUDIO_LIBRARIES)
      SET(PORTAUDIO_FOUND "YES")
      IF( NOT PORTAUDIO_LIBRARY_DIR )
	GET_FILENAME_COMPONENT(PORTAUDIO_LIBRARY_DIR "${portaudio}" PATH)
      ENDIF( NOT PORTAUDIO_LIBRARY_DIR )
    ENDIF(PORTAUDIO_LIBRARIES)
  ENDIF(PORTAUDIO_INCLUDE_DIR)
ENDIF(NOT PORTAUDIO_FOUND)

IF(NOT PORTAUDIO_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT PORTAUDIO_FIND_QUIETLY)
    IF(PORTAUDIO_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
	      "Portaudio required, please specify its location with PORTAUDIO_ROOT.")
    ELSE(PORTAUDIO_FIND_REQUIRED)
      MESSAGE( STATUS "Portaudio was not found!!! " ${PORTAUDIO_INCLUDE_DIR})
      MESSAGE( STATUS ${PORTAUDIO_LIBRARIES} )
    ENDIF(PORTAUDIO_FIND_REQUIRED)
  ENDIF(NOT PORTAUDIO_FIND_QUIETLY)
ENDIF(NOT PORTAUDIO_FOUND)


#####
