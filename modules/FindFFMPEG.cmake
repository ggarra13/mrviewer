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
  "C:/msys/1.0/local/include"
  /usr/local/include
)

SET( SEARCH_DIRS 
  "$ENV{FFMPEG_ROOT}/bin"
  "$ENV{FFMPEG_ROOT}/lib"
  "C:/msys/1.0/local/bin"
  "C:/msys/1.0/local/lib"
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


#
# Find FFMPEG libraries
#
FIND_LIBRARY(FFMPEG_avformat_LIBRARY 
    NAMES avformat avformat-51 avformat-50
    PATHS ${SEARCH_DIRS}
)

FIND_LIBRARY(FFMPEG_avcodec_LIBRARY 
    NAMES avcodec avcodec-52 avcodec-51
    PATHS ${SEARCH_DIRS}
)

FIND_LIBRARY(FFMPEG_avutil_LIBRARY 
    NAMES avutil avutil-50 avutil-49
    PATHS ${SEARCH_DIRS}
)

FIND_LIBRARY(FFMPEG_avdevice_LIBRARY 
    NAMES avdevice avdevice-50 avdevice-49
    PATHS ${SEARCH_DIRS}
)

FIND_LIBRARY(FFMPEG_swscale_LIBRARY 
    NAMES swscale swscale-50 swscale-49
    PATHS ${SEARCH_DIRS}
)

FOREACH( lib 
    a52 amr_nb amr_wb dc1394 dirac dts faac faad gsm 
    lagarith mp3lame nut postproc-51 postproc 
    swscale theora 
    vorbis vorbisenc-2 vorbisenc x264-50 x264
    xvidcore z 
    )
    STRING( REGEX REPLACE "-[0-9]+" "" it ${lib} )


    IF(WIN32)
      #
      # This is what it should be, but since msys compiles .lib files
      # as .a files, cmake won't find them.
      #
      FIND_LIBRARY( FFMPEG_${it}_LIBRARY
      	NAMES ${it}  lib${it}  ${it}-0 lib${it}-0 
              ${lib} lib${lib} ${lib}-0 lib${lib}-0 
      	PATHS ${SEARCH_DIRS}
      	)

    ELSE(WIN32)
      FIND_LIBRARY( FFMPEG_${it}_LIBRARY
	NAMES ${it} lib${lib} lib${it} lib${it}-0
	PATHS ${SEARCH_DIRS}
	)

    ENDIF(WIN32)

    IF( NOT FFMPEG_${it}_LIBRARY )
      SET( FFMPEG_${it}_LIBRARY "" )
    ENDIF( NOT FFMPEG_${it}_LIBRARY )
ENDFOREACH( lib )

MESSAGE( STATUS "FFMPEG_INCLUDE_DIR=" ${FFMPEG_INCLUDE_DIR} )

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
	    ${FFMPEG_swscale_LIBRARY}  # GPL
            )

	  SET( FFMPEG_BSD_LIBRARIES
            ${FFMPEG_gsm_LIBRARY}       # BSD
	    ${FFMPEG_nut_LIBRARY}       # MIT
            ${FFMPEG_theora_LIBRARY}    # BSD
            ${FFMPEG_vorbis_LIBRARY}    # BSD
            ${FFMPEG_vorbisenc_LIBRARY} # BSD
            ${FFMPEG_z_LIBRARY}         # Zlib
	    )
	  
	  SET( FFMPEG_LGPL_LIBRARIES
            ${FFMPEG_BASIC_LIBRARIES} # LPGL
            ${FFMPEG_dc1394_LIBRARY}  # GPL/LGPL
	    ${FFMPEG_dirac_LIBRARY}   # LPGL & Free of patents (verified by BBC)
	    ${FFMPEG_mp3lame_LIBRARY} # LPGL
	    )
	  
          SET( FFMPEG_LIBRARIES  
	    ${FFMPEG_LGPL_LIBRARIES} 
	    ${FFMPEG_BSD_LIBRARIES} )
	  
	  
	  SET( FFMPEG_GPL_LIBRARIES
	    ${FFMPEG_a52_LIBRARY}      # GPL
	    ${FFMPEG_dts_LIBRARY}      # GPL
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
  FFMPEG_a52_LIBRARY
  FFMPEG_amr_nb_LIBRARY
  FFMPEG_amr_wb_LIBRARY
  FFMPEG_avformat_LIBRARY
  FFMPEG_avcodec_LIBRARY
  FFMPEG_avutil_LIBRARY
  FFMPEG_avdevice_LIBRARY
  FFMPEG_dc1394_LIBRARY
  FFMPEG_dirac_LIBRARY
  FFMPEG_dts_LIBRARY
  FFMPEG_faac_LIBRARY
  FFMPEG_faad_LIBRARY
  FFMPEG_gsm_LIBRARY
  FFMPEG_lagarith_LIBRARY
  FFMPEG_mp3lame_LIBRARY
  FFMPEG_nut_LIBRARY
  FFMPEG_postproc_LIBRARY
  FFMPEG_swscale_LIBRARY
  FFMPEG_theora_LIBRARY
  FFMPEG_vorbis_LIBRARY
  FFMPEG_vorbisenc_LIBRARY
  FFMPEG_x264_LIBRARY
  FFMPEG_xvidcore_LIBRARY
  FFMPEG_z_LIBRARY
  )
