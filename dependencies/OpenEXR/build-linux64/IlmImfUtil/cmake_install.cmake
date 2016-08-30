# Install script for directory: /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  foreach(file
      "$ENV{DESTDIR}/usr/local/lib/libIlmImfUtil-2_2.so.22.0.0"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImfUtil-2_2.so.22"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImfUtil-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libIlmImfUtil-2_2.so.22.0.0;/usr/local/lib/libIlmImfUtil-2_2.so.22;/usr/local/lib/libIlmImfUtil-2_2.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE SHARED_LIBRARY FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfUtil/libIlmImfUtil-2_2.so.22.0.0"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfUtil/libIlmImfUtil-2_2.so.22"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfUtil/libIlmImfUtil-2_2.so"
    )
  foreach(file
      "$ENV{DESTDIR}/usr/local/lib/libIlmImfUtil-2_2.so.22.0.0"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImfUtil-2_2.so.22"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImfUtil-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/usr/local/lib:/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfUtil/../IlmImf:/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImf:"
           NEW_RPATH "")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/OpenEXR/ImfImageChannel.h;/usr/local/include/OpenEXR/ImfFlatImageChannel.h;/usr/local/include/OpenEXR/ImfDeepImageChannel.h;/usr/local/include/OpenEXR/ImfSampleCountChannel.h;/usr/local/include/OpenEXR/ImfImageLevel.h;/usr/local/include/OpenEXR/ImfFlatImageLevel.h;/usr/local/include/OpenEXR/ImfDeepImageLevel.h;/usr/local/include/OpenEXR/ImfImage.h;/usr/local/include/OpenEXR/ImfFlatImage.h;/usr/local/include/OpenEXR/ImfDeepImage.h;/usr/local/include/OpenEXR/ImfImageIO.h;/usr/local/include/OpenEXR/ImfFlatImageIO.h;/usr/local/include/OpenEXR/ImfDeepImageIO.h;/usr/local/include/OpenEXR/ImfImageDataWindow.h;/usr/local/include/OpenEXR/ImfImageChannelRenaming.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/OpenEXR" TYPE FILE FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageChannel.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImageChannel.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImageChannel.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfSampleCountChannel.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageLevel.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImageLevel.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImageLevel.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImage.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImage.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImage.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageIO.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfFlatImageIO.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfDeepImageIO.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageDataWindow.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfUtil/ImfImageChannelRenaming.h"
    )
endif()

