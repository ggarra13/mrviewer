if (WIN32)
  set( boost_on "TRUE" )
  set( generator "NMake Makefiles" )
  set( cxx_flags "-DOPENEXR_DLL;${CMAKE_CXX_FLAGS}" )
else()
  set( boost_on "FALSE" )
  set( generator "Unix Makefiles" )
  set( cxx_flags ${CMAKE_CXX_FLAGS} )
endif()

ExternalProject_Add(
  ${OCIO_NAME}
  # GIT_REPOSITORY "https://github.com/imageworks/OpenColorIO.git"
  #URL "https://github.com/imageworks/OpenColorIO/zipball/master"
  URL "https://github.com/imageworks/OpenColorIO/zipball/v1.0.9"
  CMAKE_GENERATOR ${generator}
  DEPENDS OpenEXR ${OCIO_DEPENDS}
  CMAKE_ARGS
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
  -DOCIO_BUILD_GPU_TESTS=OFF
  -DOCIO_BUILD_PYTHON=OFF
  -DOCIO_BUILD_JAVA=OFF
  -DOCIO_WARNING_AS_ERROR=OFF)
