include( ExternalProject )


ExternalProject_Add(
  YASM
  GIT_REPOSITORY "https://github.com/yasm/yasm.git"
  #URL "http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz"
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DBUILD_SHARED_LIBS=TRUE
  )

set( YASM "YASM" )
