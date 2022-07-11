ExternalProject_Add(
  LibRaw
  URL "https://www.libraw.org/data/LibRaw-0.19.2.tar.gz"
  CONFIGURE_COMMAND ./configure ${ARCH} --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS JPEG-TURBO
  BUILD_IN_SOURCE 1
  )

set( LibRaw "LibRaw" )
