ExternalProject_Add(
  FFmpeg
  GIT_REPOSITORY "https://git.ffmpeg.org/ffmpeg.git"
  CONFIGURE_COMMAND ./configure --enable-pic --enable-shared --enable-gpl --enable-gray --enable-libfreetype --enable-libmp3lame --enable-libtheora --enable-libvorbis --enable-libopus --enable-libass --enable-libvpx --enable-libx264 --enable-libx265 --enable-libwebp --enable-bzlib --enable-zlib --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS NASM x264 x265 libvpx
  BUILD_IN_SOURCE 1
  )
