if (WIN32)
  set( generator "NMake Makefiles" )
else()
  set( generator "Unix Makefiles" )
endif()

ExternalProject_Add(
  LIBPNG
  URL "https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/libpng-1.6.37.tar.gz/download"
  CMAKE_GENERATOR "${generator}"
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  )
