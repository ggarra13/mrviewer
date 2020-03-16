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


include( ExternalProject )

include( BuildLibZ.cmake )
include( BuildLibPNG.cmake )

set( NASM "" )
set( YASM "" )
if( UNIX )
  include( BuildNASM.cmake )
  include( BuildYASM.cmake )
endif()
include( BuildOpenEXR.cmake )
set( LIBLZMA "" )
include( BuildLZMA.cmake )
include( BuildJPEGTurbo.cmake )
include( BuildTIFF.cmake )
include( BuildAcesContainer.cmake )
set( OCIO_NAME "OCIO" )
set( OCIO_DEPENDS "" )
include( BuildOCIO.cmake )
set( LibRaw "" )
if( UNIX )
  include( BuildLCMS2.cmake )
  include( BuildLibWebP.cmake )
  include( BuildLibOPUS.cmake )
#   # include( BuildRefIccMAX.cmake )  # does not work use SampleICC
  include( BuildSampleICC.cmake )
  include( BuildLibRaw.cmake )
  include( BuildLibVPX.cmake )
  include( BuildX264.cmake )
  include( BuildX265.cmake )
  include( BuildFFmpeg.cmake )
  include( BuildImageMagick.cmake )
endif()

include( BuildOIIO.cmake )


# We include OCIO twice to compile the stand-alone utilities that depend
# on OIIO.  We remove this step as only one utility is useful.
# set( OCIO_NAME "OCIO2" )
# set( OCIO_DEPENDS "OIIO" )
# include( BuildOCIO.cmake )

include( BuildCTL.cmake )
include( BuildFLTK.cmake )
