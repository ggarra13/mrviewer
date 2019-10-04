set( LibWebP "LibWebP" )

ExternalProject_Add(
  ${LibWebP}
  URL "https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-1.0.2.tar.gz"
  CONFIGURE_COMMAND ./configure --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )
