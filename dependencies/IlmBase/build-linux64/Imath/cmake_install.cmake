# Install script for directory: /media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath

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
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so.12.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so.12"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/build-linux64/Imath/libImath-2_2.so.12.0.0"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/build-linux64/Imath/libImath-2_2.so.12"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/build-linux64/Imath/libImath-2_2.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so.12.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so.12"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/build-linux64/Iex:"
           NEW_RPATH "")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathBoxAlgo.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathBox.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathColorAlgo.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathColor.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathEuler.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathExc.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathExport.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathForward.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFrame.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFrustum.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFrustumTest.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFun.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathGL.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathGLU.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathHalfLimits.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathInt64.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathInterval.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathLimits.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathLineAlgo.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathLine.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathMath.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathMatrixAlgo.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathMatrix.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathNamespace.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathPlane.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathPlatform.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathQuat.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathRandom.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathRoots.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathShear.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathSphere.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathVecAlgo.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathVec.h"
    )
endif()

