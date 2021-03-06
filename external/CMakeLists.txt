cmake_minimum_required(VERSION 3.10)

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

set (ARCH "" )
set (OSXARCHS "" )

if( APPLE )
#    set (ARCH ARCHFLAGS="-arch x86_64 -arch arm64 -Xarch_x86_64 -mmacosx-version-min=10.14" )
#    set (OSXARCHS -DCMAKE_OSX_ARCHITECTURES=x86_64,arm64 )
endif()

include( ExternalProject )

include( BuildLibZ.cmake )

# set( NASM "" )
# set( YASM "" )
#set( OpenEXR "" )
include( BuildOpenEXR.cmake )
# # set( LIBLZMA "" )
include( BuildLZMA.cmake )

set( OCIO_NAME "OCIO" )
set( OCIO_DEPENDS "" )
include( BuildJPEGTurbo.cmake )
include( BuildTIFF.cmake )
include( BuildOCIO.cmake )
# set( LibRaw "" )
if( UNIX )
     include( BuildLCMS2.cmake )
     include( BuildLibWebP.cmake )
     include( BuildLibOPUS.cmake )
     include( BuildSampleICC.cmake )
     include( BuildLibRaw.cmake )
     include( BuildNASM.cmake )
     include( BuildYASM.cmake )
     include( BuildLibVPX.cmake )
     include( BuildX264.cmake )
     include( BuildX265.cmake )
     include( BuildFFmpeg.cmake )
     include( BuildImageMagick.cmake )
endif()

include( BuildAcesContainer.cmake )
include( BuildCTL.cmake )
include( BuildLibPNG.cmake )
include( BuildOIIO.cmake )
include( BuildFLTK.cmake )
