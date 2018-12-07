#
# This cmake file must be run under cmake 2.5 or later
#
CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)

# CMAKE_POLICY( SET CMP0003 NEW )

SET( TEST_BUNDLE 0 )

#
# Modify CMake's module path so our custom modules take precedence over
# cmake's default ones.

IF( NOT CMAKE_MODULE_PATH )
  SET( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules )
ENDIF( NOT CMAKE_MODULE_PATH )

#
# All CMake projects need a name.  The name is global to all
# CMakeLists.txt in subdirectories.  Once you define a project
# named "myproject", the variable "myproject_SOURCE_DIR" exists.
# This is the toplevel source directory of your project, i.e.
# wherever your topmost CMakeLists.txt resides.
#
# You can also specify support for languages other than C++, using
# additional arguments.  mrLiquid only needs C++, so we don't bother.
#

IF( UNIX )
  SET( CMAKE_SYSTEM_LIBRARY_PATH )
  INCLUDE( Platform/UnixPaths )
ENDIF( UNIX )

# VP18 switches to c++14, so let's do that by default
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ ISO Standard")
# but switch gnu++14 or other extensions off for portability
set(CMAKE_CXX_EXTENSIONS OFF)


SET( VersionMajor 4 )
SET( VersionMinor 3 )
SET( VersionPatch 6 )
#STRING( TIMESTAMP VersionDate ".%Y%m%d19" )
#STRING( TIMESTAMP VersionDate ".%Y%m%d" )
#SET( VersionDate "2018-09-07" )


SET( SHORTVERSION  "${VersionMajor}.${VersionMinor}.${VersionPatch}" )
SET( VERSION "${VersionMajor}.${VersionMinor}.${VersionPatch}${VersionDate}" )

set(mrViewerShortName mrViewer-v${SHORTVERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH})
set(mrViewerPackageName mrViewer-v${VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH})

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "")

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)





#
# Project's subdirectories
#
set( LIB_ACES_CLIP_ONLY 1 )

ADD_SUBDIRECTORY( libACESclip )
ADD_SUBDIRECTORY( mrViewer/src )
#ADD_SUBDIRECTORY( tools )

IF( UNIX )
  ADD_SUBDIRECTORY( shaders )
  EXECUTE_PROCESS( COMMAND ${CMAKE_SOURCE_DIR}/bin/release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
ENDIF( UNIX )
