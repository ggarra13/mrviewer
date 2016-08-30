# Install script for directory: D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil

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
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "D:/code/lib/Windows_32/lib/IlmImfUtil-2_2.lib")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImfUtil/IlmImfUtil-2_2.lib")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "D:/code/lib/Windows_32/lib/IlmImfUtil-2_2.dll")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/lib" TYPE SHARED_LIBRARY FILES "D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImfUtil/IlmImfUtil-2_2.dll")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "D:/code/lib/Windows_32/include/OpenEXR/ImfImageChannel.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFlatImageChannel.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepImageChannel.h;D:/code/lib/Windows_32/include/OpenEXR/ImfSampleCountChannel.h;D:/code/lib/Windows_32/include/OpenEXR/ImfImageLevel.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFlatImageLevel.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepImageLevel.h;D:/code/lib/Windows_32/include/OpenEXR/ImfImage.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFlatImage.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepImage.h;D:/code/lib/Windows_32/include/OpenEXR/ImfImageIO.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFlatImageIO.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepImageIO.h;D:/code/lib/Windows_32/include/OpenEXR/ImfImageDataWindow.h;D:/code/lib/Windows_32/include/OpenEXR/ImfImageChannelRenaming.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/include/OpenEXR" TYPE FILE FILES
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageChannel.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImageChannel.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImageChannel.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfSampleCountChannel.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageLevel.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImageLevel.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImageLevel.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImage.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImage.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImage.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageIO.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImageIO.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImageIO.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageDataWindow.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageChannelRenaming.h"
    )
endif()

