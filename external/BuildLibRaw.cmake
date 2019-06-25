ExternalProject_Add(
  LibRaw
  URL "https://www.libraw.org/data/LibRaw-0.19.2.tar.gz"
  CONFIGURE_COMMAND ./configure --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )
