include( ExternalProject )

ExternalProject_Add(
  NASM
  URL "https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/nasm-2.14.02.tar.bz2"
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )

set(NASM "NASM" )
