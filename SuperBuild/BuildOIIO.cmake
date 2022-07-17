If(UNIX)
  string( JOIN " " cxx_flags ${CMAKE_CXX_FLAGS} -std=c++11 -Wno-error=deprecated-declarations  )
endif()

ExternalProject_Add(
  OIIO
  #URL "https://github.com/OpenImageIO/oiio/archive/master.zip"
  GIT_REPOSITORY "https://github.com/OpenImageIO/oiio.git"
  GIT_TAG 1e488aed
  GIT_PROGRESS 1
  DEPENDS OpenEXR LibTIFF LIBPNG OCIO ${LibRaw} ${LibWebP} FFmpeg
  PATCH_COMMAND patch -p 1 < ${CMAKE_CURRENT_SOURCE_DIR}/patches/oiio_patch.txt
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${cxx_flags}
  -DBUILD_SHARED_LIBS=ON
  -DUSE_PYTHON=OFF
  -DSTOP_ON_WARNING=OFF
  -DUSE_QT=OFF
  -DOIIO_BUILD_TESTS=OFF)
