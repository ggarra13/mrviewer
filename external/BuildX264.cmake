ExternalProject_Add(
  x264
  URL "ftp://ftp.videolan.org/pub/x264/snapshots/last_x264.tar.bz2"
  CONFIGURE_COMMAND ./configure --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS NASM
  BUILD_IN_SOURCE 1
  )
