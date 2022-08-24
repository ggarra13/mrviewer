include( ExternalProject )

set( lm_lib )
if( UNIX AND NOT APPLE )
  set( lm_lib --extra-libs=-lm )
endif()

set( ENV{LD_FLAGS} "-Wl,--copy-dt-needed-entries" )

ExternalProject_Add(
  FFmpeg
  #GIT_REPOSITORY "https://git.ffmpeg.org/ffmpeg.git"
  URL https://ffmpeg.org/releases/ffmpeg-5.1.tar.bz2
  CONFIGURE_COMMAND ./configure
  --disable-appkit
  --disable-avfoundation
  --disable-coreimage
  --disable-audiotoolbox
  --disable-vaapi
  --enable-pic
  --enable-shared
  --enable-gpl
  --enable-gray
  --enable-gnutls
  --enable-libfontconfig
  --enable-libfreetype
  --enable-libmp3lame
  --enable-libtheora
  --enable-libvorbis
  --enable-libopus
  --enable-libass
  --enable-libvpx
  --enable-libx264
  --enable-libwebp
  --enable-bzlib
  --enable-zlib
  --extra-libs=-lgnutls
  ${lm_lib}
  --extra-libs=-lass
  --extra-libs=-lpthread
  --extra-cflags=-I/usr/include/freetype2
  --extra-cflags=-I/usr/include/fontconfig
  --extra-cflags=-I${CMAKE_PREFIX_PATH}/include/opus
  --extra-cflags=-I/usr/include/
  --extra-ldflags=-L${CMAKE_PREFIX_PATH}/lib
  --extra-ldflags=-L${CMAKE_PREFIX_PATH}/lib64
  --prefix=${CMAKE_INSTALL_PREFIX}
  --enable-libx265
  DEPENDS ${NASM} ${LibAss} ${x264} ${LIBPNG} ${LibTIFF} ${JPEGTURBO} ${LibVPX}
  ${LibOpus} ${LibWebP} ${x265}
  BUILD_IN_SOURCE 1
  )

set( FFmpeg FFmpeg )
