include( ExternalProject )

ExternalProject_Add(
  AcesContainer
  GIT_REPOSITORY "https://github.com/ampas/aces_container.git"
  GIT_PROGRESS 1
  DEPENDS ${LibTIFF}
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=--std=c++11
  -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
  )

set( AcesContainer "AcesContainer" )
