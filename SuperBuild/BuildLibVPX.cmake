include( ExternalProject )

ExternalProject_Add(
  LibVPX
  GIT_REPOSITORY "https://github.com/webmproject/libvpx.git"
  #https://chromium.googlesource.com/webm/libvpx
  #GIT_TAG v1.12.0
  CONFIGURE_COMMAND ./configure ${ARCH} --enable-shared --prefix=${CMAKE_INSTALL_PREFIX}
  DEPENDS ${YASM}
  BUILD_IN_SOURCE 1
  )

SET( LibVPX "LibVPX" )
