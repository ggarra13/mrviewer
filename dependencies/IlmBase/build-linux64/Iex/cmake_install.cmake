# Install script for directory: /media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex

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
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libIex-2_2.so.12.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libIex-2_2.so.12"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libIex-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/build-linux64/Iex/libIex-2_2.so.12.0.0"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/build-linux64/Iex/libIex-2_2.so.12"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/build-linux64/Iex/libIex-2_2.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libIex-2_2.so.12.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libIex-2_2.so.12"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libIex-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexBaseExc.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexMathExc.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexThrowErrnoExc.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexErrnoExc.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexMacros.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/Iex.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexNamespace.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexExport.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Iex/IexForward.h"
    )
endif()

