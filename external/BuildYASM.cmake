include( ExternalProject )


ExternalProject_Add(
  YASM
  URL "http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz"
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )

set( YASM "YASM" )
