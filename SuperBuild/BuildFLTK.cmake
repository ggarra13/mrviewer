ExternalProject_Add(
  FLTK
  GIT_REPOSITORY "https://github.com/fltk/fltk.git"
  GIT_PROGRESS 1
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DFLTK_BUILD_EXAMPLES=OFF
  -DFLTK_BUILD_TEST=OFF
  -DOPTION_BUILD_SHARED_LIBS=0
  -DOPTION_USE_SYSTEM_ZLIB=0
  -DOPTION_USE_SYSTEM_LIBJPEG=0
  -DOPTION_USE_SYSTEM_LIBPNG=0)
