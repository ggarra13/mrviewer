if(WIN32)
  set( ffmpeg_includes "D:/abs/local${CMAKE_BUILD_ARCH}/bin-video/ffmpegSHARED/include" )
endif()


if(UNIX)
  string( JOIN " " cxx_flags ${CMAKE_CXX_FLAGS} -std=c++11 -Wno-error=deprecated-declarations  )
  set( ffmpeg_includes "" )
endif()

set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/FindFFmpeg.cmake ${CMAKE_BINARY_DIR}/OIIO-prefix/src/OIIO/src/cmake/modules/ )

ExternalProject_Add(
  OIIO
  #URL "https://github.com/OpenImageIO/oiio/archive/master.zip"
  GIT_REPOSITORY "https://github.com/OpenImageIO/oiio.git"
  GIT_PROGRESS 1
  PATCH_COMMAND ${patch_command}
  DEPENDS ${FFmpeg} ${OCIO} ${OpenEXR} ${LibTIFF} ${LIBPNG} ${LibRaw} ${LibWebP}          ${BOOST}
  # PATCH_COMMAND patch -p 1 < ${CMAKE_CURRENT_SOURCE_DIR}/patches/oiio_patch.txt
  CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DBUILD_SHARED_LIBS=ON
  -DBoost_ROOT=${CMAKE_INSTALL_PREFIX}
  -DFFMPEG_INCLUDES=${ffmpeg_includes}
  -DFFmpeg_ROOT=$ENV{FFMPEG_ROOT}
  -DUSE_PYTHON=OFF
  -DSTOP_ON_WARNING=OFF
  -DUSE_QT=OFF
  -DOIIO_BUILD_TESTS=OFF)

set( OIIO "OIIO" )
