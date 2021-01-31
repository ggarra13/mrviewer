include( ExternalProject )

if (WIN32 OR UNIX )
  IF( NOT APPLE )
      ExternalProject_Add(
      x265
      HG_REPOSITORY "http://hg.videolan.org/x265"
      HG_TAG tip
      DEPENDS NASM
      CONFIGURE_COMMAND cd ../x265/build/linux && ./multilib.sh)
    ENDIF()
ENDIF()
