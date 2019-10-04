ExternalProject_Add(
  ImageMagick
  GIT_REPOSITORY "https://github.com/ImageMagick/ImageMagick.git"
  CONFIGURE_COMMAND ./configure --enable-static --enable-shared --prefix=${CMAKE_INSTALL_PREFIX} --with-quantum-depth=32 --without-magick-plus-plus --disable-assert --enable-hdri --without-raw --without-openexr --with-sysroot=${CMAKE_INSTALL_PREFIX}
  DEPENDS JPEG-TURBO LIBPNG LibTIFF LCMS2 OpenEXR ZLIB
  BUILD_IN_SOURCE 1
  )
