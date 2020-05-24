include( ExternalProject )

ExternalProject_Add(
  FFmpeg
  GIT_REPOSITORY "https://git.ffmpeg.org/ffmpeg.git"
  CONFIGURE_COMMAND ./configure --enable-pic --enable-shared --enable-gpl --enable-gray --enable-gnutls --enable-libfreetype --enable-libmp3lame --enable-libtheora --enable-libvorbis --enable-libopus --enable-libass --enable-libvpx --enable-libx264 --enable-libx265 --enable-libwebp --enable-bzlib --enable-zlib --pkg-config=true  --extra-libs="-lgnutls -lass" --extra-cflags='-I/usr/local/include/freetype2' --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS NASM x264 x265 libvpx LibOpus LibWebP
  BUILD_IN_SOURCE 1
  )
