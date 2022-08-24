include( ExternalProject )

set( LIBVPX_FLAGS --enable-shared )
if( APPLE )
  set( LIBVPX_FLAGS --enable-static )
endif()

ExternalProject_Add(
  LibVPX
  GIT_REPOSITORY "https://github.com/webmproject/libvpx.git"
  #https://chromium.googlesource.com/webm/libvpx
  #GIT_TAG v1.12.0
  CONFIGURE_COMMAND ./configure ${ARCH} ${LIBVPX_FLAGS} --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS ${YASM}
  BUILD_IN_SOURCE 1
  )

SET( LibVPX "LibVPX" )
