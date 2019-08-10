if(UNIX)
  string( JOIN " " cxx_flags ${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations  )
endif()

ExternalProject_Add(
  OIIO
  # URL "https://github.com/OpenImageIO/oiio/archive/release.zip"
  GIT_REPOSITORY "https://github.com/OpenImageIO/oiio.git"
  GIT_PROGRESS 1
  DEPENDS OpenEXR LIBPNG OCIO ${LibRaw} ${LibWebP}
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${cxx_flags}
  -DBUILD_SHARED_LIBS=ON
  -DUSE_PYTHON=OFF
  -DOIIO_BUILD_TESTS=OFF)
