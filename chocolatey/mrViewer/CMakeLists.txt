
cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

set(mrViewerPackageName32 mrViewer-v${VERSION}-${CMAKE_SYSTEM_NAME}-32)
set(mrViewerPackageName64 mrViewer-v${VERSION}-${CMAKE_SYSTEM_NAME}-64)
set( ZIPFILE32 "${CMAKE_BINARY_DIR}/../../BUILD/Windows-6.3.9600-32/Release/tmp/mrViewer/src/${mrViewerPackageName32}.zip" )
set( ZIPFILE64 "${CMAKE_BINARY_DIR}/../../BUILD/Windows-6.3.9600-64/Release/tmp/mrViewer/src/${mrViewerPackageName64}.zip" )

FILE( GLOB NUPACKAGES "${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/*.nupkg" )

IF( NOT "${NUPACKAGES}" STREQUAL "" )
    execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${NUPACKAGES} old/
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/" )
    execute_process( COMMAND ${CMAKE_COMMAND} -E remove ${NUPACKAGES}
		     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/" )
ENDIF( NOT "${NUPACKAGES}" STREQUAL "" )
		     
execute_process( COMMAND ${CMAKE_COMMAND} -E remove tools/*
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/" )
IF( EXISTS ${ZIPFILE32} AND EXISTS ${ZIPFILE64} )

    execute_process( COMMAND checksum --type sha256 ${ZIPFILE32} OUTPUT_VARIABLE CHECKSUM OUTPUT_STRIP_TRAILING_WHITESPACE )

    execute_process( COMMAND checksum --type sha256 ${ZIPFILE64} OUTPUT_VARIABLE CHECKSUM64 OUTPUT_STRIP_TRAILING_WHITESPACE )

    MESSAGE( STATUS "" )
    MESSAGE( STATUS "CHECKSUM   .zip ${CHECKSUM}" )
    MESSAGE( STATUS "CHECKSUM64 .zip ${CHECKSUM64}" )
    MESSAGE( STATUS "" )

    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/mrviewer.nuspec.in ${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/mrviewer.nuspec @ONLY NEWLINE_STYLE WIN32 )
    
    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/input/chocolateyinstall.ps1.in ${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/tools/chocolateyinstall.ps1 @ONLY NEWLINE_STYLE WIN32 )

    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/input/chocolateyuninstall.ps1.in ${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/tools/chocolateyuninstall.ps1 @ONLY NEWLINE_STYLE WIN32 )


    execute_process( COMMAND choco pack WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/" )

ELSE( EXISTS ${ZIPFILE32} AND EXISTS ${ZIPFILE64} )
      MESSAGE( STATUS "" )
      IF( EXISTS ${ZIPFILE64} )
      	  MESSAGE( STATUS "${ZIPFILE32} does not exist." )
      ELSE( EXISTS ${ZIPFILE64} )
      	  MESSAGE( STATUS "${ZIPFILE64} does not exist." )
      ENDIF( EXISTS ${ZIPFILE64} )
      MESSAGE( STATUS "" )

ENDIF( EXISTS ${ZIPFILE32} AND EXISTS ${ZIPFILE64} )
