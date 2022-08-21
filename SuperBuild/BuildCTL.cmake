include( ExternalProject )

## Under the g++-7 compiler, CTL crashes in Release mode.
## So we set it to debug.  Note that the main directory will still report
## as Release as it is created by the mk bash script file.
## However, the install command will report the proper configuration at its
## beginning.
if( CMAKE_COMPILER_IS_GNUCXX MATCHES 1)
  set( build_type "Debug" )
else()
  set( build_type "Release" )
endif()

if (WIN32)
  STRING( JOIN " " cxx_flags -EHsc -DWIN32 -D_WIN32 -DIMATH_DLL -DOPENEXR_DLL ${CMAKE_CXX_FLAGS} )
else()
  set( cxx_flags ${CMAKE_CXX_FLAGS} )
endif()

set( patch_command ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/CTL/FindIlmBase.cmake ${CMAKE_BINARY_DIR}/CTL-prefix/src/CTL/cmake/modules/ && ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/CTL/CMakeLists.txt ${CMAKE_BINARY_DIR}/CTL-prefix/src/CTL/lib/IlmCtlSimd )

ExternalProject_Add(
    CTL
    GIT_REPOSITORY "https://github.com/ggarra13/CTL.git"
    GIT_TAG master
    GIT_PROGRESS 1
    DEPENDS ${AcesContainer} ${OpenEXR} ${Imath}
    PATCH_COMMAND ${patch_command}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_BUILD_TYPE=${build_type}
    -DCMAKE_CXX_FLAGS=${cxx_flags}
    -DCMAKE_CXX_STANDARD=14
    -DCMAKE_CXX_EXTENSIONS=OFF
    ${MRV_EXTERNAL_ARGS}
    )

set( CTL "CTL" )
