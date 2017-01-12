
set(PROJECT choco )
cmake_minimum_required( VERSION 2.8 )

set(mrViewerPackageName32 mrViewer-v${VERSION}-${CMAKE_SYSTEM_NAME}-32)
set(mrViewerPackageName64 mrViewer-v${VERSION}-${CMAKE_SYSTEM_NAME}-64)
set( ZIPFILE32 "${CMAKE_CURRENT_BINARY_DIR}/../../../../../Windows-6.3.9600-32/Release/tmp/${mrViewerPackageName32}.zip" )
set( ZIPFILE64 "${CMAKE_CURRENT_BINARY_DIR}/../../${mrViewerPackageName64}.zip" )

execute_process( COMMAND checksum --type sha256 ${ZIPFILE32} OUTPUT_VARIABLE CHECKSUM OUTPUT_STRIP_TRAILING_WHITESPACE )
execute_process( COMMAND checksum --type sha256 ${ZIPFILE64} OUTPUT_VARIABLE CHECKSUM64 OUTPUT_STRIP_TRAILING_WHITESPACE )

MESSAGE( STATUS "" )
MESSAGE( STATUS "CHECKSUM   .zip ${CHECKSUM}" )
MESSAGE( STATUS "CHECKSUM64 .zip ${CHECKSUM64}" )
MESSAGE( STATUS "" )

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/mrviewer.nuspec.in ${CMAKE_CURRENT_SOURCE_DIR}/mrviewer.nuspec @ONLY NEWLINE_STYLE WIN32 )
    
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/tools/chocolateyinstall.ps1.in ${CMAKE_CURRENT_SOURCE_DIR}/tools/chocolateyinstall.ps1 @ONLY NEWLINE_STYLE WIN32 )

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/tools/chocolateyuninstall.ps1.in ${CMAKE_CURRENT_SOURCE_DIR}/tools/chocolateyuninstall.ps1 @ONLY NEWLINE_STYLE WIN32 )

execute_process( COMMAND choco pack WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
