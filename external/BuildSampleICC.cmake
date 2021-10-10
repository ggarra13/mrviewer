include( ExternalProject )


ExternalProject_Add(
  SampleICC
  URL  "https://sourceforge.net/projects/sampleicc/files/sampleicc%20tar/SampleIcc-1.6.8/SampleICC-1.6.8.tar.gz/download"
  CONFIGURE_COMMAND sh configure ${ARCH} --enable-shared --enable-static --prefix=${CMAKE_INSTALL_PREFIX}
  PATCH_COMMAND rm -rf 'Contrib/Mac_OS_X/extract_probe_data'
  BUILD_IN_SOURCE 1
  )
