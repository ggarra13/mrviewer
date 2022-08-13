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

if (UNIX)
   set( cxx_flags ${cxx_flags} -std=c++11 )
endif()

ExternalProject_Add(
    CTL
    GIT_REPOSITORY "https://github.com/ggarra13/CTL.git"
    GIT_TAG master
    GIT_PROGRESS 1
    DEPENDS ${AcesContainer} ${OpenEXR}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_BUILD_TYPE=${build_type}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${cxx_flags}
    ${MRV_EXTERNAL_ARGS}
    )

set( CTL "CTL" )
