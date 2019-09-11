## Under the g++-7 compiler, CTL crashes in Release mode.
## So we set it to debug.  Note that the main directory will still report
## as Release as it is created by the mk bash script file.
## However, the install command will report the proper configuration at its
## beginning.
if (CMAKE_CXX_COMPILER_VERSION MATCHES "^7\." )
  set( CMAKE_BUILD_TYPE "Debug" )
endif()

if (WIN32)
  STRING( JOIN " " cxx_flags -DOPENEXR_DLL -DWIN32 -D_WIN32 ${CMAKE_CXX_FLAGS} )
  set( DO_SHARED FALSE )
else()
  set( cxx_flags ${CMAKE_CXX_FLAGS} )
  set( DO_SHARED TRUE )
endif()


ExternalProject_Add(
    CTL
    GIT_REPOSITORY "https://github.com/ggarra13/CTL.git"
    GIT_PROGRESS 1
    DEPENDS OpenEXR
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${cxx_flags}
    -DENABLE_SHARED=${DO_SHARED}
    -DBUILD_SHARED_LIBS=${DO_SHARED}
    )
