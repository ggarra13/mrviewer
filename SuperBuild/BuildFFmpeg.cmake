include( ExternalProject )

set( ENV{LD_FLAGS} "-Wl,--copy-dt-needed-entries" )

ExternalProject_Add(
  FFmpeg
  GIT_REPOSITORY "https://git.ffmpeg.org/ffmpeg.git"
  GIT_TAG n4.4.2
  CONFIGURE_COMMAND ./configure ${ARCH} --enable-pic --enable-shared --enable-gpl --enable-gray --enable-gnutls --enable-libfontconfig --enable-libfreetype --enable-libmp3lame --enable-libtheora --enable-libvorbis --enable-libopus --enable-libass --enable-libvpx --enable-libx264 --enable-libwebp --enable-bzlib --enable-zlib --extra-libs=-lgnutls --extra-libs=-lass --extra-libs=-lpthread --extra-cflags=-I/usr/include/freetype2 --extra-cflags=-I/usr/include/fontconfig --extra-cflags=-I${CMAKE_INSTALL_PREFIX}/include/opus --extra-cflags=-I/usr/include/ --extra-ldflags=-L${CMAKE_INSTALL_PREFIX}/lib --prefix=${CMAKE_INSTALL_PREFIX} --enable-libx265 --disable-vaapi
  DEPENDS NASM x264 LIBPNG LibTIFF JPEG-TURBO libvpx LibOpus LibWebP x265
  BUILD_IN_SOURCE 1
  )
