include( ExternalProject )

ExternalProject_Add(
  FFmpeg
  GIT_REPOSITORY "https://git.ffmpeg.org/ffmpeg.git"
  CONFIGURE_COMMAND ./configure ${ARCH} --enable-pic --enable-shared --enable-gpl --enable-gray --enable-gnutls --enable-libfreetype --enable-libmp3lame --enable-libtheora --enable-libvorbis --enable-libopus --enable-libass --enable-libvpx --enable-libx264 --enable-libwebp --enable-bzlib --enable-zlib --pkg-config=true  --extra-libs=-lgnutls --extra-libs=-lass --extra-cflags=-I/usr/include/freetype2 --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS NASM x264 libvpx LibOpus LibWebP # x265
  BUILD_IN_SOURCE 1
  )
