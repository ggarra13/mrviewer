include( ExternalProject )

set( LibVPX_TAG 888bafc78d8bddb5cfc4262c93f456c812763571 )

set( LIBVPX_FLAGS --enable-shared )
if( APPLE )
  set( LIBVPX_FLAGS --enable-static )
endif()

ExternalProject_Add(
  LibVPX
  GIT_REPOSITORY "https://github.com/webmproject/libvpx.git"
  GIT_TAG ${LibVPX_TAG}
  CONFIGURE_COMMAND sh ./configure ${LIBVPX_FLAGS} --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS ${YASM}
  BUILD_IN_SOURCE 1
  )

SET( LibVPX "LibVPX" )
