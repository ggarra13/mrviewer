# Install script for directory: F:/code/lib/openexr-git/IlmBase/Iex

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "f:/code/lib/Windows_64/Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "F:/code/lib/openexr-git/IlmBase/build-win64-debug/Iex/Iex-2_2.lib")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "F:/code/lib/openexr-git/IlmBase/build-win64-debug/Iex/Iex-2_2.dll")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES
    "F:/code/lib/openexr-git/IlmBase/Iex/IexBaseExc.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/IexMathExc.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/IexThrowErrnoExc.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/IexErrnoExc.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/IexMacros.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/Iex.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/IexNamespace.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/IexExport.h"
    "F:/code/lib/openexr-git/IlmBase/Iex/IexForward.h"
    )
endif()

