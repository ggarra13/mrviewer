set( OTIO_TAG d6ad4ebc0504a65193c5baf6f2eba697f80be89e )

ExternalProject_Add(
  OTIO
  GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/OpenTimelineIO.git"
  GIT_TAG ${OTIO_TAG}
  GIT_PROGRESS 1
  DEPENDS ${Imath}
  CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DBUILD_SHARED_LIBS=ON
  -DBoost_ROOT=${CMAKE_PREFIX_PATH}
#  -DOTIO_IMATH_LIBS=ON
#  -DOTIO_FIND_IMATH=OFF
  -DUSE_PYTHON=OFF)

set( OTIO "OTIO" )
