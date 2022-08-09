#-*-cmake-*-
#
# Test for R3DSDK
#
# Once loaded this will define
#  R3DSDK_FOUND        - system has R3DSDK
#  R3DSDK_INCLUDE_DIR  - include directory for R3DSDK
#  R3DSDK_LIBRARIES    - libraries for R3SDK
#

SET(R3DSDK_FOUND "NO")


FIND_PATH( R3DSDK_INCLUDE_DIR R3DSDK.h
  "$ENV{R3DSDK_ROOT}/Include"
  "$ENV{R3DSDK_ROOT}"
  ../../../R3DSDKv8_1_0/Include
  ../../../R3DSDKv8_0_3/Include
  ../../../R3DSDKv7_3_4/Include
  ../../../R3DSDKv7_3_1/Include
  ../../../R3DSDKv7_2_0/Include
  ../../../R3DSDKv7_1_0/Include
  /usr/local/include
  /usr/include
  DOC   "R3DSDK includes"
  )

get_filename_component( R3DSDK_ROOT ${R3DSDK_INCLUDE_DIR} DIRECTORY )
IF(APPLE)
#    set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/mac64/libR3DSDK-libstdcpp.a )
    set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/mac64/libR3DSDK-libcpp.a )
ELSEIF(UNIX)
  if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0 )
    set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/linux64/libR3DSDKPIC.a )
  else()
    set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/linux64/libR3DSDK.a )
  endif()
ELSEIF( WIN64 )
      if( ${R3DSDK_ROOT} MATCHES "R3DSDKv8_1_0" )
	  set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/win64/R3DSDK-2017MD.lib )
      else()
	    if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/win64/R3DSDK-2015MDd.lib )
	     else()
		 set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/win64/R3DSDK-2015MD.lib )
	     endif()
       endif()
ELSEIF( WIN32 )
      if( ${R3DSDK_ROOT} MATCHES "R3DSDKv8_1_0" )
	 set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/win32/R3DSDK-2017MD.lib )
       else()
	 set( R3DSDK_LIBRARIES ${R3DSDK_ROOT}/Lib/win32/R3DSDK-2015MD.lib )
       endif()
ENDIF()

IF(NOT R3DSDK_FOUND)
  IF (R3DSDK_INCLUDE_DIR)
    SET(R3DSDK_FOUND "YES")
  ENDIF(R3DSDK_INCLUDE_DIR)
ENDIF(NOT R3DSDK_FOUND)

IF(NOT R3DSDK_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT R3DSDK_FIND_QUIETLY)
    IF(R3DSDK_FIND_REQUIRED)
      MESSAGE( STATUS "R3DSDK_INCLUDE_DIR ${R3DSDK_INCLUDE_DIR}" )
      MESSAGE(FATAL_ERROR
	      "R3DSDK required, please specify its location with R3DSDK_ROOT.")
    ELSE(R3DSDK_FIND_REQUIRED)
      MESSAGE(STATUS "R3DSDK was not found.")
    ENDIF(R3DSDK_FIND_REQUIRED)
  ENDIF(NOT R3DSDK_FIND_QUIETLY)
ENDIF(NOT R3DSDK_FOUND)

#####
