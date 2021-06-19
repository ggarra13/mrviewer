include( ExternalProject )

if (WIN32 OR UNIX )
   if (APPLE)
      ExternalProject_Add(
      x265
      HG_REPOSITORY "http://hg.videolan.org/x265"
      HG_TAG tip
      DEPENDS NASM
      CONFIGURE_COMMAND cd ../x265/build/xcode && cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} ../../source
      BUILD_COMMAND   cd ../x265/build/xcode && ninja
      INSTALL_COMMAND cd ../x265/build/xcode && ninja install
      BUILD_IN_SOURCE 1
    )
   else()
      ExternalProject_Add(
      x265
      HG_REPOSITORY "http://hg.videolan.org/x265"
      HG_TAG tip
      DEPENDS NASM
      CONFIGURE_COMMAND cd ../x265/build/linux && ./multilib.sh && cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} ../../source
      BUILD_COMMAND   cd ../x265/build/linux && make
      INSTALL_COMMAND cd ../x265/build/linux && make install
      BUILD_IN_SOURCE 1
    )
  endif()
ENDIF()
