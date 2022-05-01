include( ExternalProject )

if (WIN32)
  set( generator "NMake Makefiles" )
else()
  set( generator "Unix Makefiles" )
endif()

set( PNG_FLAGS "-I${CMAKE_BINARY_DIR}/ZLIB-prefix/src/ZLIB ${CMAKE_C_FLAGS}" )

ExternalProject_Add(
  LIBPNG
  DEPENDS ZLIB
  URL "https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/libpng-1.6.37.tar.gz/download"
  CMAKE_GENERATOR Ninja
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_C_FLAGS=${PNG_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  )
