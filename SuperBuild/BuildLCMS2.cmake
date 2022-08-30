ExternalProject_Add(
  LCMS2
  GIT_REPOSITORY "https://github.com/mm2/Little-CMS.git"
  GIT_PROGRESS 1
  CONFIGURE_COMMAND ./configure --enable-shared --enable-static --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )

set( LCMS2 "LCMS2" )
