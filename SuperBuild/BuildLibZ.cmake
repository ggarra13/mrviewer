include( ExternalProject )

ExternalProject_Add(
  ZLIB
  URL "https://zlib.net/zlib-1.2.12.tar.gz"
#  URL "https://zlib.net/fossils/zlib-1.2.8.tar.gz"
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  ${MRV_EXTERNAL_ARGS}
  -Dlibdeflate=OFF
  ${OSX_ARCHS}

  )

set( ZLIB "ZLIB" )
