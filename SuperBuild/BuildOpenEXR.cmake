if (WIN32)
  STRING( JOIN " " cxx_flags -EHsc -DOPENEXR_DLL -DOPENEXR_EXPORTS -DIMATH_DLL ${CMAKE_CXX_FLAGS} )
else()
  set( cxx_flags ${CMAKE_CXX_FLAGS} )
endif()

if (APPLE)
  set(CMAKE_CXX_FLAGS -std=c++11 ${CMAKE_CXX_FLAGS} )
endif()


set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/IlmThreadMutex.h ${CMAKE_BINARY_DIR}/OpenEXR-prefix/src/OpenEXR/src/lib/IlmThread/ )

ExternalProject_Add(
  OpenEXR
  GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/openexr.git"
  GIT_TAG v3.1.5
  # GIT_TAG main
  GIT_PROGRESS 1
  DEPENDS ${ZLIB}
  PATCH_COMMAND ${patch_command}
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DBUILD_TESTING=FALSE
  -DPYILMBASE_ENABLE=FALSE
  -DOPENEXR_BUILD_PYTHON_LIBS=FALSE
   ${MRV_EXTERNAL_ARGS}
  )

set( OpenEXR "OpenEXR" )
