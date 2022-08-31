#
# This cmake file must be run under cmake 3.1 or later
#
cmake_minimum_required( VERSION 3.1 )
project( mrViewer )

include( cmake/policies.cmake )


IF(WIN32)
    set( TEST_BUNDLE 0 )
ENDIF()

#
# Modify CMake's module path so our custom modules take precedence over
# cmake's default ones.
list( INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

#
# Target Apple MacOS earlier than Catalina
#
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "Version of MacOS")

#
# Choose XCode SDK
#
#set(CMAKE_OSX_SYSROOT "/Developer/ CACHE STRING "XCode SDK")

#
# All CMake projects need a name.  The name is global to all
# CMakeLists.txt in subdirectories.  Once you define a project
# named "myproject", the variable "myproject_SOURCE_DIR" exists.
# This is the toplevel source directory of your project, i.e.
# wherever your topmost CMakeLists.txt resides.
#
# You can also specify support for languages other than C++, using
# additional arguments.
#

if( UNIX )
  set( CMAKE_SYSTEM_LIBRARY_PATH )
  include( Platform/UnixPaths )
endif( UNIX )

include( cmake/options.cmake )
include( cmake/functions.cmake )
include( cmake/find_packages.cmake )


message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")

string (REGEX MATCH "\\.el[1-9]" os_version_suffix ${CMAKE_SYSTEM})
message("-- os_version_suffix:      ${os_version_suffix}")

set( CENTOS7 OFF )
if ( "${os_version_suffix}" STREQUAL ".el7" )
  set( CENTOS7 ON )
endif()



set( VersionMajor 6 )
set( VersionMinor 2 )
set( VersionPatch 3 )

#STRING( TIMESTAMP VersionDate ".%Y%m%d%H" )
set( VersionDate "" )


set( SHORTVERSION  "${VersionMajor}.${VersionMinor}.${VersionPatch}" )
set( VERSION "${VersionMajor}.${VersionMinor}.${VersionPatch}${VersionDate}" )

set(mrViewerShortName mrViewer-v${SHORTVERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH})
set(mrViewerPackageName mrViewer-v${SHORTVERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH})

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)


#
# Project's subdirectories
#
set( LIB_ACES_CLIP_ONLY 1 )
set( LIB_AMF_ONLY 1 )



ADD_SUBDIRECTORY( libAMF )
ADD_SUBDIRECTORY( libACESclip )
ADD_SUBDIRECTORY( mrViewer/src )
