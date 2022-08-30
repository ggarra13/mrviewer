set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/external")

set(install_cmd ${CMAKE_COMMAND} -E copy_directory bin/ ${CMAKE_INSTALL_PREFIX}/bin && ${CMAKE_COMMAND} -E copy_directory lib/ ${CMAKE_INSTALL_PREFIX}/lib  && ${CMAKE_COMMAND} -E copy_directory libraw ${CMAKE_INSTALL_PREFIX}/include/libraw/ )

# Add libraw

set( CMAKE_CORE_BUILD_FLAGS
  -DPC_LCMS2_INCLUDEDIR=${CMAKE_INSTALL_PREFIX}/include
  -DPC_LCMS2_LIBDIR="${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib64" )

ExternalProject_Add(
	libraw_cmake
      GIT_REPOSITORY "https://github.com/LibRaw/LibRaw-cmake"
      GIT_TAG master
      PREFIX ${BUILD_DIR}
      BUILD_IN_SOURCE 0
      BUILD_ALWAYS 0
      UPDATE_COMMAND ""
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libraw_cmake
      BINARY_DIR ${BUILD_DIR}/libraw_build
      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      )

ExternalProject_Add(
	LibRaw
      #URL https://github.com/LibRaw/LibRaw/archive/0.20.0.tar.gz
      GIT_REPOSITORY "https://github.com/LibRaw/LibRaw"
      GIT_TAG 9c861fd72f3961167ef55b037d7ce16056dd32d8 # specific commit on the 2021.09.16
      PREFIX ${BUILD_DIR}
      BUILD_IN_SOURCE 0
      BUILD_ALWAYS 0
      UPDATE_COMMAND ""
      SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libraw
      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libraw
      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
      # Native libraw configure script doesn't work on centos 7 (autoconf 2.69)
      # CONFIGURE_COMMAND autoconf && ./configure --enable-jpeg --enable-openmp --disable-examples --prefix=${CMAKE_INSTALL_PREFIX}
      # Use cmake build system (not maintained by libraw devs)
    CONFIGURE_COMMAND cp <SOURCE_DIR>_cmake/CMakeLists.txt . && cp -rf <SOURCE_DIR>_cmake/cmake . && ${CMAKE_COMMAND} ${CMAKE_CORE_BUILD_FLAGS} -DENABLE_OPENMP=ON -DENABLE_LCMS=ON -DENABLE_EXAMPLES=OFF ${ZLIB_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DINSTALL_CMAKE_MODULE_PATH:PATH=${CMAKE_INSTALL_PREFIX}/cmake <SOURCE_DIR>
      BUILD_COMMAND $(MAKE)
      DEPENDS libraw_cmake ${ZLIB} ${LCMS2}
    )

set( LibRaw "LibRaw" )
