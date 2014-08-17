# - try to find SampleICC library and include files
#  SampleICC_INCLUDE_DIR, where to find SampleICC/ICCProfile.h, etc.
#  SampleICC_LIBRARIES, the libraries to link against
#

IF(NOT SampleICC_ROOT)
  SET(SampleICC_ROOT $ENV{SampleICC_ROOT} )
ENDIF( NOT SampleICC_ROOT)

SET( SEARCH_INCLUDE_PATHS
  "${SampleICC_ROOT}/IccProfLib"
  "${SampleICC_ROOT}/SampleICC"
  "${SampleICC_ROOT}/include/IccProfLib"
  "${SampleICC_ROOT}/include/SampleICC"
  "${SampleICC_ROOT}/include"
  ${CMAKE_SYSTEM_INCLUDE_PATH}
  /usr/local/include/SampleICC
  /usr/include/SampleICC
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
    IF( MSVC100 )
      SET( compiler "vc10" )
    ENDIF( MSVC100 )
    IF( MSVC120 )
      SET( compiler "vc12" )
    ENDIF( MSVC120 )
  ENDIF( MSVC )

  SET( SEARCH_PATHS 
    "${SampleICC_ROOT}/x${CMAKE_BUILD_ARCH}/${CMAKE_BUILD_TYPE}"
    "${SampleICC_ROOT}/${compiler}/lib/${CMAKE_BUILD_TYPE}"
    "${SampleICC_ROOT}/${compiler}/lib"
    "${SampleICC_ROOT}/lib/x${CMAKE_BUILD_ARCH}/${CMAKE_BUILD_TYPE}"
    "${SampleICC_ROOT}/lib"
    "${SampleICC_ROOT}/IccProfLib/${CMAKE_BUILD_TYPE}"
    ${SEARCH_PATHS}
    )
ENDIF( WIN32 )


FIND_PATH( SampleICC_INCLUDE_DIR 
  NAMES IccProfile.h
  PATHS ${SEARCH_INCLUDE_PATHS}
  NO_DEFAULT_PATH
  )
MESSAGE( "SampleICC_INCLUDE_DIR= ${SampleICC_INCLUDE_DIR}" )
MESSAGE( "SEARCH_INCLUDE_PATHS=  ${SEARCH_INCLUDE_PATHS}" )
MESSAGE( "SEARCH_PATHS=  ${SEARCH_PATHS}" )

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

