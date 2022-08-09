set( LibWebP "LibWebP" )
set( libwebp_version "1.2.4" )

set( file "libwebp-${libwebp_version}.tar.gz" )

ExternalProject_Add(
  ${LibWebP}
  URL "https://storage.googleapis.com/downloads.webmproject.org/releases/webp/${file}"
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DBUILD_TESTING=FALSE
  -DWEBP_BUILD_VWEBP=FALSE
   ${MRV_EXTERNAL_ARGS}
  )
