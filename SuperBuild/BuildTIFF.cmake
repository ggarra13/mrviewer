set( TIFF_TAG "8fc154b6c57f642289458f86107c43170b6cd465" )

set( c_flags ${CMAKE_C_FLAGS})
if( UNIX AND NOT APPLE )
    set( c_flags -lpthread ${CMAKE_C_FLAGS} )
endif()

ExternalProject_Add(
  LibTIFF
  GIT_REPOSITORY "https://gitlab.com/libtiff/libtiff.git"
  GIT_TAG ${TIFF_TAG}
  DEPENDS ${ZLIB} ${LCMS2}
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_C_FLAGS=${c_flags}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DBUILD_SHARED_LIBS=ON
  -Dtiff-tools=OFF
  -Dtiff-tests=OFF
  -Dtiff-contrib=OFF
  -Dtiff-docs=OFF
  -Dtiff-deprecated=OFF
  )

set( LibTIFF "LibTIFF" )
