# - try to find SampleICC library and include files
#  SampleICC_INCLUDE_DIR, where to find SampleICC/ICCProfile.h, etc.
#  SampleICC_LIBRARIES, the libraries to link against
#

IF(NOT SampleICC_ROOT)
  SET(SampleICC_ROOT $ENV{SampleICC_ROOT} )
ENDIF( NOT SampleICC_ROOT)

SET( SEARCH_INCLUDE_PATHS
  "${SampleICC_ROOT}/include"
  "${SampleICC_ROOT}"
  ${CMAKE_SYSTEM_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )


SET( SEARCH_PATHS 
  "/usr/local/lib${CMAKE_BUILD_ARCH}"
  "/opt/local/lib${CMAKE_BUILD_ARCH}"
  "/opt/lib${CMAKE_BUILD_ARCH}"
  "/usr/lib${CMAKE_BUILD_ARCH}"
  ${CMAKE_SYSTEM_LIBRARY_PATH}
  )

IF( WIN32 )
  IF( MSVC )
    SET( compiler "vc7" )
    IF( MSVC80 )
      SET( compiler "vc8" )
    ENDIF( MSVC80 )
  ENDIF( MSVC )

  SET( SEARCH_PATHS 
    "${SampleICC_ROOT}/${compiler}/lib/${CMAKE_BUILD_TYPE}"
    "${SampleICC_ROOT}/${compiler}/lib"
    "${SampleICC_ROOT}/lib/${CMAKE_BUILD_TYPE}"
    "${SampleICC_ROOT}/lib"
    "${SampleICC_ROOT}/IccProfLib/${CMAKE_BUILD_TYPE}_CRTDLL"
    ${SEARCH_PATHS}
    )
ENDIF( WIN32 )


FIND_PATH( SampleICC_INCLUDE_DIR 
  NAMES SampleICC/IccProfile.h IccProfLib/IccProfile.h
  PATHS ${SEARCH_INCLUDE_PATHS}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( SampleICC 
  NAMES SampleICC IccProfLib IccProfLib_CRTDLL
  PATHS ${SEARCH_PATHS}
  NO_DEFAULT_PATH
)

SET( SampleICC_LIBRARIES ${SampleICC} )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SampleICC
  DEFAULT_MSG
  SampleICC_LIBRARIES SampleICC_INCLUDE_DIR
  ) 

