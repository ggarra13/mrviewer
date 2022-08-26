set( LibHarfbuzz_TAG  23461b75020164252d6af018fa08e6e3e3907b8b)

if( APPLE )
  ####m meson fails to respect prefix and PKG_CONFIG_PATH, so we copy libz
  set( patch_cmd ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/LibHarfbuzz-prefix/src/LibHarfbuzz/build/src/ && ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_INSTALL_PREFIX}/lib/libz.1.dylib ${CMAKE_BINARY_DIR}/LibHarfbuzz-prefix/src/LibHarfbuzz/build/src/ )
endif()

ExternalProject_Add(
  LibHarfbuzz
  GIT_REPOSITORY "https://github.com/harfbuzz/harfbuzz.git"
  GIT_TAG ${LibHarfbuzz_TAG}
  DEPENDS ${ZLIB}
  PATCH_COMMAND ${patch_cmd}
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND meson setup --prefix=${CMAKE_INSTALL_PREFIX} build
  BUILD_COMMAND ninja -C build
  INSTALL_COMMAND meson install -C build
  BUILD_IN_SOURCE 1
  )

set( LibHarfbuzz "LibHarfbuzz" )
