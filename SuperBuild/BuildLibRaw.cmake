# specific commit on the 2021.09.16
set( LibRaw_TAG 9c861fd72f3961167ef55b037d7ce16056dd32d8 )

set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/LibRaw/FindLCMS2.cmake ${CMAKE_CURRENT_BINARY_DIR}/LibRaw_cmake/cmake/modules )

set( CMAKE_CORE_BUILD_FLAGS
  -DZLIB_ROOT=${CMAKE_PREFIX_PATH} )


ExternalProject_Add(
	LibRaw_cmake
      GIT_REPOSITORY "https://github.com/LibRaw/LibRaw-cmake"
      GIT_TAG master
      DEPENDS ${ZLIB} ${LCMS2}
      BUILD_IN_SOURCE 0
      BUILD_ALWAYS 0
      UPDATE_COMMAND ""
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/LibRaw_cmake
      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      )

ExternalProject_Add(
	LibRaw
      GIT_REPOSITORY "https://github.com/LibRaw/LibRaw"
      GIT_TAG  ${LibRaw_TAG}
      BUILD_IN_SOURCE 0
      BUILD_ALWAYS 0
      PATCH_COMMAND ${patch_command}
      UPDATE_COMMAND ""
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/LibRaw
      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/LibRaw
      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
      # Native libraw configure script doesn't work on centos 7 (autoconf 2.69)
      # CONFIGURE_COMMAND autoconf && ./configure --enable-jpeg --enable-openmp --disable-examples --prefix=${CMAKE_INSTALL_PREFIX}
      # Use cmake build system (not maintained by libraw devs)
      CONFIGURE_COMMAND cp <SOURCE_DIR>_cmake/CMakeLists.txt . && cp -rf <SOURCE_DIR>_cmake/cmake . && ${CMAKE_COMMAND} -G ${CMAKE_GENERATOR} ${CMAKE_CORE_BUILD_FLAGS} -DENABLE_OPENMP=ON -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DENABLE_LCMS=ON -DENABLE_EXAMPLES=OFF ${ZLIB_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> <SOURCE_DIR>
      DEPENDS LibRaw_cmake ${ZLIB} ${LCMS2}
    )

set( LibRaw "LibRaw" )
