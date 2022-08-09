include( ExternalProject )
ExternalProject_Add(
  x264
  GIT_REPOSITORY "https://code.videolan.org/videolan/x264.git"
  # URL "https://code.videolan.org/videolan/x264/-/archive/master/x264-master.tar.bz2"
  CONFIGURE_COMMAND ./configure ${ARCH} --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS ${NASM}
  BUILD_IN_SOURCE 1
  )

set( x264 "x264" )
