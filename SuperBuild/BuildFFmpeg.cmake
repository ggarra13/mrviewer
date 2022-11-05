include( ExternalProject )

set( lm_lib )
if( UNIX AND NOT APPLE )
  set( lm_lib --extra-libs=-lm )
endif()

set( ENV{LD_FLAGS} "-Wl,--copy-dt-needed-entries" )

  #
  # Optionally enable libx265 if we compiled it
  #
set( ENABLE_X265 )
if ( x265 )
  set( ENABLE_X265  --enable-libx265 )
endif()

  #
  # Optionally enable libvpx if we compiled it
  #
set( ENABLE_VPX )
if ( LibVPX )
  set( ENABLE_VPX  --enable-libvpx )
endif()

if ( LibWebP )
  set( ENABLE_LIBWEBP --enable-libwebp )
endif()


set( DISABLE_POSTPROC --disable-postproc )
set( ENABLE_GPL )

if ( USE_FFMPEG_GPL )

  set( DISABLE_POSTPROC )
  set( ENABLE_GPL --enable-gpl )

  if ( USE_X264 )
    list( APPEND ENABLE_GPL --enable-libx264 )
  endif()
  if ( USE_X265 )
    list( APPEND ENABLE_GPL --enable-libx265 )
  endif()
endif()


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
  ${DISABLE_POSTPROC}
  --enable-pic
  --enable-shared
  ${ENABLE_VPX}
  ${ENABLE_GPL}
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
  ${ENABLE_LIBWEBP}
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
  DEPENDS ${NASM} ${LibAss} ${x264} ${LIBPNG} ${LibTIFF} ${JPEGTURBO} ${LibVPX}
  ${LibOpus} ${LibWebP} ${x265}
  BUILD_IN_SOURCE 1
  )

set( FFmpeg FFmpeg )
