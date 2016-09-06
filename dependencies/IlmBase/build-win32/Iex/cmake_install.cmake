# Install script for directory: D:/code/applications/mrViewer/dependencies/IlmBase/Iex

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/code/lib/Windows_32")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/code/applications/mrViewer/dependencies/IlmBase/build-win32/Iex/Iex-2_2.lib")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "D:/code/applications/mrViewer/dependencies/IlmBase/build-win32/Iex/Iex-2_2.dll")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexBaseExc.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexMathExc.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexThrowErrnoExc.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexErrnoExc.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexMacros.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/Iex.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexNamespace.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexExport.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Iex/IexForward.h"
    )
endif()

