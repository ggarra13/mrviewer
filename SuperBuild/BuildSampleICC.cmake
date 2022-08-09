include( ExternalProject )

set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/SampleICC/Makefile.in ${CMAKE_BINARY_DIR}/SampleICC-prefix/src/SampleICC/Contrib/Mac_OS_X/ )

ExternalProject_Add(
  SampleICC
  URL  "https://sourceforge.net/projects/sampleicc/files/sampleicc%20tar/SampleIcc-1.6.8/SampleICC-1.6.8.tar.gz/download"
  PATCH_COMMAND ${patch_command}
  CONFIGURE_COMMAND sh configure ${ARCH} --enable-shared --enable-static --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )

set( SampleICC "SampleICC" )
