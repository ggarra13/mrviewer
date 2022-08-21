include( ExternalProject )

set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/AcesContainer/aces_formatter.cpp ${CMAKE_BINARY_DIR}/AC-prefix/src/AC/ )

if( WIN32 )
    set( AcesContainer_FLAGS )
else()
    set( AcesContainer_FLAGS -DCMAKE_CXX_STANDARD=11 )
endif()

ExternalProject_Add(
  AC
  GIT_REPOSITORY "https://github.com/ampas/aces_container.git"
  GIT_PROGRESS 1
  DEPENDS ${LibTIFF}
  PATCH_COMMAND ${patch_command}
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
  ${AcesContainer_FLAGS}
  )

set( AcesContainer "AC" )
