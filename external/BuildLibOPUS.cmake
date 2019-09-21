ExternalProject_Add(
  LibOpus
  URL "https://ftp.osuosl.org/pub/xiph/releases/opus/opus-1.3.1.tar.gz"
  CONFIGURE_COMMAND ./configure --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )
