# -*-cmake-*-
#
# Find the native FFMPEG includes and library
#
# This module defines
# FFMPEG_INCLUDE_DIR,      where to find avcodec.h, avformat.h ...
# FFMPEG_LIBRARIES,        the libraries to link against to use FFMPEG.
# FFMPEG_BSD_LIBRARIES     BSD/MIT/Zlib libraries.  You can do whatever you
#                          want with these (you might need to add a copyright
#                          notice about them, thou).
# FFMPEG_LGPL_LIBRARIES,   libraries that are LPGL.  Can be used in
#                          commercial software, but modification of them
#                          requires releasing source code.
# FFMPEG_GPL_LIBRARIES,    libraries that are GPL only.  Use or change of
#                          them requires releasing source code.
# FFMPEG_NONGPL_LIBRARIES, libraries that are not GPL/LGPL or have
#                          concerns over patents.
# FFMPEG_FOUND, If false, do not try to use FFMPEG.
#
# also defined, but not for general use are
# FFMPEG_avformat_LIBRARY and FFMPEG_avcodec_LIBRARY, where to find the FFMPEG library.
# This is usefull to do it this way so that we can always add more libraries
# if needed to FFMPEG_LIBRARIES if ffmpeg ever changes...

FIND_PATH(FFMPEG_INCLUDE_DIR libavformat/avformat.h
  "$ENV{FFMPEG_ROOT}/include"
  "$ENV{FFMPEG_ROOT}/.."
  "$ENV{FFMPEG_ROOT}"
  /usr/local/include
)

SET( SEARCH_DIRS 
  "$ENV{FFMPEG_ROOT}/bin"
  "$ENV{FFMPEG_ROOT}/lib"
  "$ENV{FFMPEG_ROOT}/libavcodec"
  "$ENV{FFMPEG_ROOT}/libavdevice"
  "$ENV{FFMPEG_ROOT}/libavfilter"
  "$ENV{FFMPEG_ROOT}/libavformat"
  "$ENV{FFMPEG_ROOT}/libavutil"
  "$ENV{FFMPEG_ROOT}/libpostproc"
  "$ENV{FFMPEG_ROOT}/libswscale"
  /usr/local/lib${CMAKE_BUILD_ARCH}
  )

#
# MSVC does not currently ship C99 compatible inttypes.h and stdint.h
# which are needed to compile/use ffmpeg libraries.
#
IF(MSVC)
  FIND_PATH( FFMPEG_inttypes_INCLUDE inttypes.h
    "$ENV{MSVCDIR}/include"
    "C:/msys/1.0/include"
    DOC "Download C99 compatible inttypes.h from http://code.google.com/p/msinttypes/downloads/list and place it in $ENV{MSVCDIR}/include"
    )

  FIND_PATH( FFMPEG_stdint_INCLUDE stdint.h
    "$ENV{MSVCDIR}/include"
    "C:/msys/1.0/include"
    DOC "Download C99 compatible stdint.h from http://code.google.com/p/msinttypes/downloads/list and place it in $ENV{MSVCDIR}/include"
    )

  FILE(TO_NATIVE_PATH FFMPEG_stdint_INCLUDE ${FFMPEG_stdint_INCLUDE} )

  SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} ${FFMPEG_stdint_INCLUDE} )

ENDIF(MSVC)

MESSAGE( "SEARCH DIRS=" ${SEARCH_DIRS} )

#
# Find FFMPEG libraries
#
FIND_LIBRARY(FFMPEG_avformat_LIBRARY 
    NAMES avformat avformat-52 avformat-53
    PATHS ${SEARCH_DIRS}
)


FIND_LIBRARY(FFMPEG_avcodec_LIBRARY 
    NAMES avcodec avcodec-52 avcodec-53
    PATHS ${SEARCH_DIRS}
)

FIND_LIBRARY(FFMPEG_avutil_LIBRARY 
    NAMES avutil avutil-50  avutil-51
    PATHS ${SEARCH_DIRS}
)

FIND_LIBRARY(FFMPEG_avdevice_LIBRARY 
    NAMES avdevice avdevice-52  avdevice-53
    PATHS ${SEARCH_DIRS}
)

FIND_LIBRARY(FFMPEG_swscale_LIBRARY 
    NAMES swscale swscale-0 swscale-2
    PATHS ${SEARCH_DIRS}
)

MESSAGE( STATUS "FFMPEG_INCLUDE_DIR=" ${FFMPEG_INCLUDE_DIR} )
MESSAGE( STATUS "FFMPEG_avutil_LIBRARY=" ${FFMPEG_avutil_LIBRARY} )
MESSAGE( STATUS "FFMPEG_avformat_LIBRARY=" ${FFMPEG_avformat_LIBRARY} )
MESSAGE( STATUS "FFMPEG_avcodec_LIBRARY=" ${FFMPEG_avcodec_LIBRARY} )
MESSAGE( STATUS "FFMPEG_avdevice_LIBRARY=" ${FFMPEG_avdevice_LIBRARY} )
MESSAGE( STATUS "FFMPEG_swscale_LIBRARY=" ${FFMPEG_swscale_LIBRARY} )

IF(FFMPEG_INCLUDE_DIR)
  IF(FFMPEG_avformat_LIBRARY)
    IF(FFMPEG_avcodec_LIBRARY)
      IF(FFMPEG_avutil_LIBRARY)
	IF(FFMPEG_avdevice_LIBRARY)
          SET( FFMPEG_FOUND "YES" )
	  
          SET( FFMPEG_BASIC_LIBRARIES 
            ${FFMPEG_avcodec_LIBRARY}  # LGPL, but Sorenson patent may apply
            ${FFMPEG_avformat_LIBRARY} # LGPL
            ${FFMPEG_avutil_LIBRARY}   # LGPL
	    ${FFMPEG_avdevice_LIBRARY} # LGPL
	    ${FFMPEG_swscale_LIBRARY}  # LGPL
            )

	  SET( FFMPEG_BSD_LIBRARIES
	    )
	  
	  SET( FFMPEG_LGPL_LIBRARIES
	    ${FFMPEG_BASIC_LIBRARIES}
	    )
	  
          SET( FFMPEG_LIBRARIES  
	    ${FFMPEG_LGPL_LIBRARIES} 
	    ${FFMPEG_BSD_LIBRARIES} )
	  
	  
	  SET( FFMPEG_GPL_LIBRARIES
	    ${FFMPEG_postproc_LIBRARY} # GPL
	    ${FFMPEG_swscale_LIBRARY}  # GPL
	    )
	  
	  SET( FFMPEG_NONGPL_LIBRARIES
	    ${FFMPEG_amr_nb_LIBRARY} # LPGL but patents from Ericson, Nokia &
	    # Universite de Sherbrooke
	    ${FFMPEG_amr_nw_LIBRARY} # LPGL but patents from Ericson, Nokia &
	    # Universite de Sherbrooke
            ${FFMPEG_faac_LIBRARY}   # Commercial
            ${FFMPEG_faad_LIBRARY}   # Commercial
	    ${FFMPEG_xvidcore_LIBRARY} # GPL but possible patent issues
	    ${FFMPEG_x264_LIBRARY}     # GPL but Patent licensing to MPEGLA 
            # (after 100,000 copies sold)
	    ${FFMPEG_lagarith_LIBRARY} # as XviD (shares code)
	    )
	ENDIF(FFMPEG_avdevice_LIBRARY)
      ENDIF(FFMPEG_avutil_LIBRARY)
    ENDIF(FFMPEG_avcodec_LIBRARY)
  ENDIF(FFMPEG_avformat_LIBRARY)
ENDIF(FFMPEG_INCLUDE_DIR)

MARK_AS_ADVANCED(
  FFMPEG_INCLUDE_DIR
  FFMPEG_avformat_LIBRARY
  FFMPEG_avcodec_LIBRARY
  FFMPEG_avutil_LIBRARY
  FFMPEG_avdevice_LIBRARY
  FFMPEG_postproc_LIBRARY
  FFMPEG_swscale_LIBRARY
  )
