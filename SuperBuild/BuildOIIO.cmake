set( OIIO_TAG v2.4.2.1-dev )
set( BOOST_VERSION 1_73 )




if(UNIX)
  string( JOIN " " cxx_flags ${CMAKE_CXX_FLAGS} -std=c++11 -Wno-error=deprecated-declarations  )
  set( ffmpeg_includes "" )
endif()

set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/FindFFmpeg.cmake ${CMAKE_BINARY_DIR}/OIIO-prefix/src/OIIO/src/cmake/modules/ )


set( FFMPEG_ROOT )
if( WIN32 )
  set( FFMPEG_ROOT $ENV{FFMPEG_ROOT} )
  set( Boost_INCLUDE_DIR
    ${CMAKE_INSTALL_PREFIX}/include/boost-${BOOST_VERSION} )
else()
  set( Boost_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/ )
endif()

if ( WIN32 AND NOT FFMPEG_ROOT )
   message( FATAL_ERROR "Please define the environment variable FFMPEG_ROOT to compile OpenImageIO with ffmpeg support" )
endif()

ExternalProject_Add(
  OIIO
  GIT_REPOSITORY "https://github.com/OpenImageIO/oiio.git"
  GIT_TAG ${OIIO_TAG}
  GIT_PROGRESS 1
  PATCH_COMMAND ${patch_command}
  DEPENDS ${FFmpeg} ${OCIO} ${OpenEXR} ${LibTIFF} ${LIBPNG} ${LibRaw} ${LibWebP}          ${BOOST}
  CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DBUILD_SHARED_LIBS=ON
  -DBoost_ROOT=${CMAKE_INSTALL_PREFIX}
  -DBoost_INCLUDE_DIR=${Boost_INCLUDE_DIR}
  -DFFmpeg_ROOT=$ENV{FFMPEG_ROOT}
  -DUSE_PYTHON=OFF
  -DSTOP_ON_WARNING=OFF
  -DUSE_QT=OFF
  -DBoost_DEBUG=ON
  -DBoost_VERBOSE=ON
  -DVERBOSE=ON
  -DOIIO_BUILD_TESTS=OFF)

set( OIIO "OIIO" )
