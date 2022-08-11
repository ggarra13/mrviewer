cmake_minimum_required(VERSION 3.10)

# Use extraction timestamps
if( NOT CMAKE_VERSION LESS 3.24.0 )
  cmake_policy( SET CMP0135 NEW )
endif()

#
# Run this file by calling runme.sh - NOT cmake
#
project( Dependencies )


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

if( APPLE )
#    set (ARCH ARCHFLAGS="-arch x86_64 -arch arm64 -Xarch_x86_64 -mmacosx-version-min=10.14" )
#    set (OSXARCHS -DCMAKE_OSX_ARCHITECTURES=x86_64,arm64 )
endif()

#  VP18 switches to c++14, so let's do that by default
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ ISO Standard")
# but switch gnu++14 or other extensions off for portability
set(CMAKE_CXX_EXTENSIONS OFF)


set(MRV_EXTERNAL_ARGS
    -DBUILD_SHARED_LIBS=ON
    -DENABLE_SHARED=ON)

include( ExternalProject )

set( ZLIB "" )
set( OpenEXR "" )
set( LZMA "" )
set( LCMS2 "" )
set( NASM "" )
set( JPEGTURBO "" )
set( LIBPNG "" )
set( OCIO "" )
set( LibTIFF "" )
set( LibWebP "" )
set( LibOpus "" )
set( SampleICC "" )
set( LibRaw "" )
set( YASM "" )
set( LibVPX "" )
set( x264 "" )
set( x265 "" )
set( LibHarfbuzz "" )
set( LibAss "" )
set( FFmpeg "" )
set( ImageMagick "" )

include( BuildLibZ.cmake )

#include( BuildImath.cmake )  # gets built by OpenEXR

include( BuildOpenEXR.cmake )

include( BuildLZMA.cmake )

if( UNIX )
     include( BuildLCMS2.cmake )
endif()

if( UNIX )
     include( BuildNASM.cmake )
endif()

include( BuildJPEGTurbo.cmake )

include( BuildTIFF.cmake )

include( BuildOCIO.cmake )

include( BuildLibPNG.cmake )

include( BuildLibWebP.cmake )

if( UNIX )
     include( BuildLibOPUS.cmake )
     include( BuildSampleICC.cmake )
     include( BuildLibRaw.cmake )
     include( BuildYASM.cmake )
     include( BuildLibVPX.cmake )
     include( BuildX264.cmake )
     include( BuildX265.cmake )
     include( BuildLibHarfbuzz.cmake )
     include( BuildLibAss.cmake )
     include( BuildFFmpeg.cmake )
     include( BuildImageMagick.cmake )
endif()

set( AcesContainer "" )
include( BuildAcesContainer.cmake )

set( CTL "" )
include( BuildCTL.cmake )

set( OIIO "" )
include( BuildOIIO.cmake )

set( FLTK "" )
include( BuildFLTK.cmake )
