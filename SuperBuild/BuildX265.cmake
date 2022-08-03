include( ExternalProject )

if (WIN32 OR UNIX )
  # IF( NOT APPLE )
      ExternalProject_Add(
      x265
      #HG_REPOSITORY "http://hg.videolan.org/x265"
      URL "http://ftp.videolan.org/pub/videolan/x265/x265_3.2.1.tar.gz"
      DEPENDS YASM
      CONFIGURE_COMMAND cd ${CMAKE_BINARY_DIR}/x265-prefix/src/x265/build/linux && ./multilib.sh && cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} ../../source
      BUILD_COMMAND   cd ${CMAKE_BINARY_DIR}/x265-prefix/src/x265/build/linux && make
      INSTALL_COMMAND cd ${CMAKE_BINARY_DIR}/x265-prefix/src/x265/build/linux && make install
      BUILD_IN_SOURCE 1
    )
    # ENDIF()
ENDIF()
