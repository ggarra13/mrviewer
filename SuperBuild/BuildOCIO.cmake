if (WIN32)
  set( boost_on "TRUE" )
  set( generator "NMake Makefiles" )
  set( cxx_flags "-DOPENEXR_DLL;${CMAKE_CXX_FLAGS}" )
else()
  set( boost_on "FALSE" )
  set( generator "Unix Makefiles" )
  set( cxx_flags ${CMAKE_CXX_FLAGS} )
endif()

if (APPLE)
  set( boost_on "TRUE" )
endif()

cmake_host_system_information(RESULT HAS_SSE2 QUERY HAS_SSE2)

ExternalProject_Add(
  OCIO
  GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/OpenColorIO.git"
  GIT_TAG main
  CMAKE_GENERATOR ${generator}
  DEPENDS OpenEXR
  CMAKE_ARGS
  -DOCIO_USE_SSE=${HAS_SSE2}
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${cxx_flags}
  -DBUILD_SHARED_LIBS=ON
  -DOCIO_USE_BOOST_PTR=${boost_on}
  -DOCIO_BUILD_APPS=OFF
  -DOCIO_BUILD_NUKE=OFF
  -DOCIO_BUILD_DOCS=OFF
  -DOCIO_BUILD_TESTS=OFF
  -DOCIO_BUILD_PYTHON=OFF
  -DOCIO_BUILD_PYGLUE=OFF
  -DOCIO_BUILD_JNIGLUE=OFF
  -DOCIO_STATIC_JNIGLUE=OFF
  )


set( OCIO "OCIO" )
