#
# This cmake file must be run under cmake 3.1 or later
#
cmake_minimum_required( VERSION 3.1 )
project( mrViewer )

cmake_policy( SET CMP0068 NEW )

IF(WIN32)
    SET( TEST_BUNDLE 0 )
ENDIF()

#
# Modify CMake's module path so our custom modules take precedence over
# cmake's default ones.

list( INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/modules)

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

IF( UNIX )
  SET( CMAKE_SYSTEM_LIBRARY_PATH )
  INCLUDE( Platform/UnixPaths )
ENDIF( UNIX )

# This sets -fPIC on all libraries and code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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


#  OIIO needs c++14, so let's do that by default
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ ISO Standard")
# but switch gnu++11 or other extensions off for portability
set(CMAKE_CXX_EXTENSIONS OFF)


SET( VersionMajor 6 )
SET( VersionMinor 2 )
SET( VersionPatch 3 )

#STRING( TIMESTAMP VersionDate ".%Y%m%d%H" )
SET( VersionDate "" )


SET( SHORTVERSION  "${VersionMajor}.${VersionMinor}.${VersionPatch}" )
SET( VERSION "${VersionMajor}.${VersionMinor}.${VersionPatch}${VersionDate}" )

set(mrViewerShortName mrViewer-v${SHORTVERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH})
set(mrViewerPackageName mrViewer-v${SHORTVERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH})

# use, i.e. don't skip the full RPATH for the build tree
#SET(CMAKE_SKIP_BUILD_RPATH FALSE)
#SET(CMAKE_SKIP_RPATH FALSE)

# set @rpaths for libraries to link against
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_PREFIX_PATH}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)



# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_SKIP_INSTALL_RPATH TRUE)

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "")

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)


#
# Project's subdirectories
#
set( LIB_ACES_CLIP_ONLY 1 )
set( LIB_AMF_ONLY 1 )


# CMAKE_POLICY( SET CMP0003 NEW )
cmake_policy( SET CMP0092 NEW )

#ADD_SUBDIRECTORY( SuperBuild )
ADD_SUBDIRECTORY( libAMF )
ADD_SUBDIRECTORY( libACESclip )
ADD_SUBDIRECTORY( mrViewer/src )
#ADD_SUBDIRECTORY( tools )
