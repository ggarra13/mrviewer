include( ExternalProject )


ExternalProject_Add(
  LibHarfbuzz
  GIT_REPOSITORY "https://github.com/harfbuzz/harfbuzz.git"
  GIT_TAG main
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND meson --prefix=${CMAKE_INSTALL_PREFIX} build
  BUILD_COMMAND ninja -C build
  INSTALL_COMMAND meson install -C build
  BUILD_IN_SOURCE 1
  )

set( LibHarfbuzz "LibHarfbuzz" )
