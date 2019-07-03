ExternalProject_Add(
  OpenEXR
  GIT_REPOSITORY "https://github.com/openexr/openexr.git"
  GIT_PROGRESS 1
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=--std=c++11
  -DOPENEXR_BUILD_PYTHON_LIBS=0
  -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
  )