include( ExternalProject )

ExternalProject_Add(
  FFmpeg
  GIT_REPOSITORY "https://git.ffmpeg.org/ffmpeg.git"
  CONFIGURE_COMMAND ./configure ${ARCH} --enable-pic --enable-shared --enable-gpl --enable-gray --enable-gnutls --enable-libfontconfig --enable-libfreetype --enable-libmp3lame --enable-libtheora --enable-libvorbis --enable-libopus --enable-libass --enable-libvpx --enable-libx264 --enable-libwebp --enable-bzlib --enable-zlib --extra-libs=-lgnutls --extra-libs=-lass --extra-cflags=-I/usr/include/freetype2 --extra-cflags=-I/usr/include/fontconfig --extra-cflags=-I/usr/include/ --prefix=${CMAKE_INSTALL_PREFIX} --enable-libx265
  DEPENDS NASM x264 libvpx LibOpus LibWebP x265
  BUILD_IN_SOURCE 1
  )
