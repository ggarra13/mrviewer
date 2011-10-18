#-*-cmake-*-
#
# Test for ImageMagick libraries, unlike CMake's FindImageMagick.cmake which
# tests for ImageMagick's binary utilities
#
# Once loaded this will define
#  MAGICK_FOUND        - system has ImageMagick
#  MAGICK_VERSION      - full version of ImageMagick ( 6.3.1 )
#  MAGICK_INCLUDE_DIR  - include directory for ImageMagick
#  MAGICK_LIBRARY_DIR  - library directory for ImageMagick
#  MAGICK_LIBRARIES    - libraries you need to link to
#  MAGICK_CODECS       - codecs used for reading/writing images
#
#  MAGICK++_FOUND        - system has ImageMagick
#  MAGICK++_INCLUDE_DIR  - include directory for ImageMagick
#  MAGICK++_LIBRARY_DIR  - library directory for ImageMagick
#  MAGICK++_LIBRARIES    - libraries you need to link to
#

SET(MAGICK_FOUND   "NO" )
SET(MAGICK++_FOUND "NO" )

FIND_PATH( MAGICK_INCLUDE_DIR magick/magick.h
  "$ENV{MAGICK_HOME}/include"
  "$ENV{MAGICK_HOME}"
  /usr/local/include/ImageMagick
  /usr/include/ImageMagick
  )

#
# Extract ImageMagick's version number from .h file
#
IF( MAGICK_INCLUDE_DIR )
  FILE( READ "${MAGICK_INCLUDE_DIR}/magick/version.h" tmp )
  STRING( REGEX REPLACE ".*define[ \t]+MagickLibVersionText[ \t]+\"([^\"]+)\".*" "\\1" MAGICK_VERSION ${tmp} )
ENDIF( MAGICK_INCLUDE_DIR )


FIND_PATH( MAGICK++_INCLUDE_DIR wand/magick-wand.h
  "$ENV{MAGICK_HOME}/include"
  "$ENV{MAGICK_HOME}"
  /usr/local/include/ImageMagick
  /usr/include/ImageMagick
  )

FIND_LIBRARY( Magick 
  NAMES MagickCore CORE_RL_Magick_
  PATHS 
  "$ENV{MAGICK_HOME}/magick/.libs"
  "$ENV{MAGICK_HOME}/VisualMagick/bin"
  "$ENV{MAGICK_HOME}/VisualMagick/lib"
  /usr/local/lib${CMAKE_BUILD_ARCH}
  /usr/lib
  DOC   "ImageMagick's Magick library"
)

FIND_LIBRARY( Magick++ 
  NAMES Magick++ CORE_RL_Magick++_
  PATHS 
  "$ENV{MAGICK_HOME}/magick/.libs"
  "$ENV{MAGICK_HOME}/VisualMagick/bin"
  "$ENV{MAGICK_HOME}/VisualMagick/lib"
  /usr/local/lib${CMAKE_BUILD_ARCH}
  /usr/lib
  DOC   "ImageMagick's Magick++ library"
)


FIND_LIBRARY( Wand 
  NAMES MagickWand CORE_RL_wand_
  PATHS 
  "$ENV{MAGICK_HOME}/wand/.libs"
  "$ENV{MAGICK_HOME}/VisualMagick/bin"
  "$ENV{MAGICK_HOME}/VisualMagick/lib"
  /usr/local/lib${CMAKE_BUILD_ARCH}
  /usr/lib
  DOC   "ImageMagick Wand library"
)


SET(MAGICK_LIBRARIES ${Magick} )
SET(MAGICK++_LIBRARIES ${Wand} ${Magick++} ${Magick} )
GET_FILENAME_COMPONENT(MAGICK_LIBRARY_DIR ${Magick}   PATH)


IF(WIN32)
  FILE( GLOB MAGICK_CODECS 
    "$ENV{MAGICK_HOME}/VisualMagick/bin/IM_MOD_*.dll"
    "$ENV{MAGICK_HOME}/ImageMagick-${MAGICK_VERSION}/modules-Q32/coders/*.dll"
    )
ELSE(WIN32)
  FILE( GLOB MAGICK_CODECS 
    "$ENV{MAGICK_HOME}/ImageMagick-${MAGICK_VERSION}/modules-Q32/coders/*.so"
    "${MAGICK_LIBRARY_DIR}/ImageMagick-${MAGICK_VERSION}/modules-Q32/coders/*.so"
    "/usr/local/lib/ImageMagick-${MAGICK_VERSION}/modules-Q32/coders/*.so"
    "/usr/lib/${MAGICK_LIBRARY_DIR}/ImageMagick-${MAGICK_VERSION}/modules-Q32/coders/*.so"
    )
ENDIF(WIN32)

IF (MAGICK_INCLUDE_DIR)
  IF(MAGICK_LIBRARIES)
    SET(MAGICK_FOUND "YES")

    FILE( GLOB AUXLIBS "${MAGICK_LIBRARY_DIR}/CORE_RL_*" )
    SET(MAGICK_LIBRARIES ${MAGICK_LIBRARIES} ${AUXLIBS} )

  ENDIF(MAGICK_LIBRARIES)
ENDIF(MAGICK_INCLUDE_DIR)

IF (MAGICK++_INCLUDE_DIR)
  IF(MAGICK++_LIBRARIES)
    SET(MAGICK++_FOUND "YES")
    GET_FILENAME_COMPONENT(MAGICK++_LIBRARY_DIR ${Wand} PATH)
  ENDIF(MAGICK++_LIBRARIES)
ENDIF(MAGICK++_INCLUDE_DIR)


IF(NOT MAGICK_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT Magick_FIND_QUIETLY)
    IF(Magick_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "ImageMagick required, please specify it's location with MAGICK_HOME")
    ELSE(Magick_FIND_REQUIRED)
      MESSAGE(STATUS "ImageMagick was not found.")
      MESSAGE(STATUS "Magick include:" ${MAGICK_INCLUDE_DIR})
      MESSAGE(STATUS "Magick libs:" ${MAGICK++_LIBRARIES})
    ENDIF(Magick_FIND_REQUIRED)
  ENDIF(NOT Magick_FIND_QUIETLY)
ENDIF(NOT MAGICK_FOUND)


#####

