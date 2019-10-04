ExternalProject_Add(
  libvpx
  GIT_REPOSITORY "https://github.com/webmproject/libvpx.git"
  CONFIGURE_COMMAND ./configure --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS YASM
  BUILD_IN_SOURCE 1
  )
