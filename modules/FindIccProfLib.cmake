# - try to find RefIccMax library and include files
#  IccProfLib_INCLUDE_DIR, where to find IccProfile2/ICCProfile.h, etc.
#  IccProfLib_LIBRARIES, the libraries to link against
#

IF(NOT IccProfLib_ROOT)
  SET(IccProfLib_ROOT $ENV{IccProfLib_ROOT} )
ENDIF( NOT IccProfLib_ROOT)

SET( SEARCH_INCLUDE_PATHS
  "${IccProfLib_ROOT}/include/RefIccMAX/IccProfLib2"
  "${IccProfLib_ROOT}/include/IccProfLib2"
  "${IccProfLib_ROOT}/IccProfLib2"
  "${IccProfLib_ROOT}/include"
  ${CMAKE_SYSTEM_INCLUDE_PATH}
  /usr/local/include/RefIccMAX/IccProfLib2
  /usr/include/RefIccMAX/IccProfLib2
  /usr/local/include/IccProfLib2
  /usr/include/IccProfLib2
  )


SET( SEARCH_PATHS
  "/usr/local/lib${CMAKE_BUILD_ARCH}"
  "/opt/local/lib${CMAKE_BUILD_ARCH}"
  "/usr/local/lib"
  "/opt/local/lib"
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
    IF( MSVC140 )
      SET( compiler "vc14" )
    ENDIF( MSVC140 )
  ENDIF( MSVC )

  SET( SEARCH_PATHS
    "${IccProfLib_ROOT}/x${CMAKE_BUILD_ARCH}/${CMAKE_BUILD_TYPE}"
    "${IccProfLib_ROOT}/${compiler}/lib/${CMAKE_BUILD_TYPE}"
    "${IccProfLib_ROOT}/${compiler}/lib"
    "${IccProfLib_ROOT}/lib/x${CMAKE_BUILD_ARCH}/${CMAKE_BUILD_TYPE}"
    "${IccProfLib_ROOT}/lib"
    "${IccProfLib_ROOT}/IccProfLib/${CMAKE_BUILD_TYPE}_CRTDLL"
    "${IccProfLib_ROOT}/IccProfLib/${CMAKE_BUILD_TYPE}"
    ${SEARCH_PATHS}
    )
ENDIF( WIN32 )

MESSAGE( ${SEARCH_INCLUDE_PATHS} )

FIND_PATH( IccProfLib_INCLUDE_DIR
  NAMES IccProfile.h
  PATHS ${SEARCH_INCLUDE_PATHS}
  NO_DEFAULT_PATH
  )
MESSAGE( "IccProfLib_INCLUDE_DIR= ${IccProfLib_INCLUDE_DIR}" )
MESSAGE( "SEARCH_INCLUDE_PATHS=  ${SEARCH_INCLUDE_PATHS}" )
MESSAGE( "SEARCH_PATHS=  ${SEARCH_PATHS}" )

FIND_LIBRARY( IccProfLib
  NAMES IccProfLib2_CRTDLL IccProfLib2
  PATHS ${SEARCH_PATHS}
  NO_DEFAULT_PATH
)

MESSAGE( "IccProfLib=  ${IccProfLib}" )

SET( IccProfLib_LIBRARIES ${IccProfLib} )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( IccProfLib
  DEFAULT_MSG
  IccProfLib_LIBRARIES IccProfLib_INCLUDE_DIR
  )
