cmake_minimum_required(VERSION 3.10)

# Use extraction timestamps
if( NOT CMAKE_VERSION LESS 3.24.0 )
  cmake_policy( SET CMP0135 NEW )
endif()

#
# Run this file by calling runme.sh - NOT cmake
#
project( Dependencies )


message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")

string (REGEX MATCH "\\.el[1-9]" os_version_suffix ${CMAKE_SYSTEM})
message("-- os_version_suffix:      ${os_version_suffix}")

set( CENTOS7 OFF )
if ( "${os_version_suffix}" STREQUAL ".el7" )
  set( CENTOS7 ON )
endif()


# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "")


set( CMAKE_MAKEFILE_VERBOSE 1 )

set (ARCH "" )
set (OSXARCHS "" )

#  VP18 switches to c++14, so let's do that by default
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ ISO Standard")
# but switch gnu++14 or other extensions off for portability


set( MRV_EXTERNAL_ARGS
  -DBUILD_SHARED_LIBS=ON
  -DENABLE_SHARED=ON )

include( ExternalProject )


include( ../cmake/options.cmake )

set( ZLIB  )
set( OpenEXR  )
set( LZMA  )
set( LCMS2  )
set( NASM  )
set( JPEGTURBO  )
set( LIBPNG  )
set( OCIO  )
set( LibTIFF  )
set( LibWebP  )
set( LibOpus  )
set( SampleICC  )
set( LibRaw  )
set( YASM  )
set( LibVPX  )
set( x264  )
set( x265  )
set( LibHarfbuzz  )
set( LibAss  )
set( FFmpeg  )
set( ImageMagick  )
set( AcesContainer  )
set( TCLAP  )
set( CTL  )
set( OIIO  )
set( OTIO  )
set( FLTK  )
set( BOOST  )
set( TinyXML2 )

include( BuildLibZ.cmake )

if( CENTOS7 OR APPLE )
  include( BuildLibHarfbuzz.cmake )
endif()

include( BuildTinyXML2.cmake )

if( CENTOS7 OR NOT BOOST_FOUND )
   include( BuildBoost.cmake )
endif()


include( BuildOTIO.cmake )

include( BuildImath.cmake )

include( BuildOpenEXR.cmake )

include( BuildLZMA.cmake )

if( UNIX )
     include( BuildLCMS2.cmake )
     include( BuildNASM.cmake )
endif()

include( BuildJPEGTurbo.cmake )

include( BuildTIFF.cmake )

include( BuildOCIO.cmake )

include( BuildLibPNG.cmake )

include( BuildLibWebP.cmake )

include( BuildLibRaw.cmake )

if( UNIX )
     include( BuildLibOPUS.cmake )
     include( BuildSampleICC.cmake )
     include( BuildYASM.cmake )
     if ( USE_VPX )
       include( BuildLibVPX.cmake )
     endif()

     if ( USE_FFMPEG_GPL )
       if (USE_X264 )
	 include( BuildX264.cmake )
       endif()
       if ( USE_X265 )
	 include( BuildX265.cmake )
       endif()
     endif()

     include( BuildLibAss.cmake )
     include( BuildFFmpeg.cmake )
     include( BuildImageMagick.cmake )
endif()

include( BuildAcesContainer.cmake )

include( BuildCTL.cmake )

include( BuildOIIO.cmake )

include( BuildTCLAP.cmake )

include( BuildFLTK.cmake )
