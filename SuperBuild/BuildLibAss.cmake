include( ExternalProject )

set( ENV{PATH} ${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH} )


ExternalProject_Add(
  LibAss
  GIT_REPOSITORY "https://github.com/libass/libass.git"
  CONFIGURE_COMMAND sh autogen.sh && sh ./configure --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS ${NASM} ${LibHarfbuzz}
  BUILD_IN_SOURCE 1
  )

set( LibAss "LibAss" )
