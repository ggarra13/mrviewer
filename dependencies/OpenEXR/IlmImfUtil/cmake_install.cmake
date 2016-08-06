# Install script for directory: F:/code/lib/openexr-git/OpenEXR/IlmImfUtil

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "f:/code/lib/Windows_64")
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

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/Debug/IlmImfUtil-2_2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/Release/IlmImfUtil-2_2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/MinSizeRel/IlmImfUtil-2_2.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/RelWithDebInfo/IlmImfUtil-2_2.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE SHARED_LIBRARY FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/Debug/IlmImfUtil-2_2.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE SHARED_LIBRARY FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/Release/IlmImfUtil-2_2.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE SHARED_LIBRARY FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/MinSizeRel/IlmImfUtil-2_2.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "f:/code/lib/Windows_64/lib/IlmImfUtil-2_2.dll")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/lib" TYPE SHARED_LIBRARY FILES "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/RelWithDebInfo/IlmImfUtil-2_2.dll")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "f:/code/lib/Windows_64/include/OpenEXR/ImfImageChannel.h;f:/code/lib/Windows_64/include/OpenEXR/ImfFlatImageChannel.h;f:/code/lib/Windows_64/include/OpenEXR/ImfDeepImageChannel.h;f:/code/lib/Windows_64/include/OpenEXR/ImfSampleCountChannel.h;f:/code/lib/Windows_64/include/OpenEXR/ImfImageLevel.h;f:/code/lib/Windows_64/include/OpenEXR/ImfFlatImageLevel.h;f:/code/lib/Windows_64/include/OpenEXR/ImfDeepImageLevel.h;f:/code/lib/Windows_64/include/OpenEXR/ImfImage.h;f:/code/lib/Windows_64/include/OpenEXR/ImfFlatImage.h;f:/code/lib/Windows_64/include/OpenEXR/ImfDeepImage.h;f:/code/lib/Windows_64/include/OpenEXR/ImfImageIO.h;f:/code/lib/Windows_64/include/OpenEXR/ImfFlatImageIO.h;f:/code/lib/Windows_64/include/OpenEXR/ImfDeepImageIO.h;f:/code/lib/Windows_64/include/OpenEXR/ImfImageDataWindow.h;f:/code/lib/Windows_64/include/OpenEXR/ImfImageChannelRenaming.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "f:/code/lib/Windows_64/include/OpenEXR" TYPE FILE FILES
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfImageChannel.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfFlatImageChannel.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfDeepImageChannel.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfSampleCountChannel.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfImageLevel.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfFlatImageLevel.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfDeepImageLevel.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfImage.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfFlatImage.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfDeepImage.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfImageIO.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfFlatImageIO.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfDeepImageIO.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfImageDataWindow.h"
    "F:/code/lib/openexr-git/OpenEXR/IlmImfUtil/ImfImageChannelRenaming.h"
    )
endif()

