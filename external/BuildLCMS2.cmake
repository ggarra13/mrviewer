ExternalProject_Add(
  LCMS2
  GIT_REPOSITORY "https://github.com/mm2/Little-CMS.git"
  GIT_PROGRESS 1
  DEPENDS LibTIFF JPEG-TURBO
  CONFIGURE_COMMAND ./configure --enable-shared --enable-static --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )
