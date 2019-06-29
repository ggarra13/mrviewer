ExternalProject_Add(
  SampleICC
  URL  "https://sourceforge.net/projects/sampleicc/files/sampleicc%20tar/SampleIcc-1.6.8/SampleICC-1.6.8.tar.gz/download"
  CONFIGURE_COMMAND sh configure --enable-shared --enable-static --prefix=${CMAKE_INSTALL_PREFIX}
  BUILD_IN_SOURCE 1
  )
