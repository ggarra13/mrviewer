
set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/external")

set(install_cmd ${CMAKE_COMMAND} -E copy_directory bin/ ${CMAKE_INSTALL_PREFIX}/bin && ${CMAKE_COMMAND} -E copy_directory lib/ ${CMAKE_INSTALL_PREFIX}/lib  && ${CMAKE_COMMAND} -E copy_directory libraw ${CMAKE_INSTALL_PREFIX}/include/libraw/ )

if( WIN32 )

ExternalProject_Add(
	LibRaw
      #URL https://github.com/LibRaw/LibRaw/archive/0.20.0.tar.gz
      GIT_REPOSITORY "https://github.com/LibRaw/LibRaw"
      GIT_TAG 9c861fd72f3961167ef55b037d7ce16056dd32d8 # specific commit on the 2021.09.16
      BUILD_IN_SOURCE 1
      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND nmake -f Makefile.msvc
      INSTALL_COMMAND ${install_cmd}
      DEPENDS ${ZLIB}
      )
else()

# Add libraw

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
      # CONFIGURE_COMMAND autoconf && ./configure --enable-jpeg --enable-openmp --disable-examples --prefix=<INSTALL_DIR>
      # Use cmake build system (not maintained by libraw devs)
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>_cmake/CMakeLists.txt . && ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>_cmake/cmake . && ${CMAKE_COMMAND} ${CMAKE_CORE_BUILD_FLAGS} -DENABLE_OPENMP=ON -DENABLE_LCMS=ON -DENABLE_EXAMPLES=OFF ${ZLIB_CMAKE_FLAGS} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DINSTALL_CMAKE_MODULE_PATH:PATH=<INSTALL_DIR>/cmake <SOURCE_DIR>
      BUILD_COMMAND $(MAKE)
      DEPENDS libraw_cmake ${ZLIB_TARGET}
      )
endif()
SET(LIBRAW_CMAKE_FLAGS -DLIBRAW_PATH=${CMAKE_INSTALL_PREFIX} -DPC_LIBRAW_INCLUDEDIR=${CMAKE_INSTALL_PREFIX}/include -DPC_LIBRAW_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib -DPC_LIBRAW_R_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib)

set( LibRaw "LibRaw" )
