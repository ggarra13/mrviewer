# Install script for directory: /media/Linux/code/lib/openexr-git/IlmBase/Imath

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
    "/media/Linux/code/lib/openexr-git/IlmBase/build-linux64/Imath/libImath-2_2.so.12.0.0"
    "/media/Linux/code/lib/openexr-git/IlmBase/build-linux64/Imath/libImath-2_2.so.12"
    "/media/Linux/code/lib/openexr-git/IlmBase/build-linux64/Imath/libImath-2_2.so"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so.12.0.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so.12"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libImath-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_REMOVE
           FILE "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathBoxAlgo.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathBox.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathColorAlgo.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathColor.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathEuler.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathExc.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathExport.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathForward.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathFrame.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathFrustum.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathFrustumTest.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathFun.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathGL.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathGLU.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathHalfLimits.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathInt64.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathInterval.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathLimits.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathLineAlgo.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathLine.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathMath.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathMatrixAlgo.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathMatrix.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathNamespace.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathPlane.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathPlatform.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathQuat.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathRandom.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathRoots.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathShear.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathSphere.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathVecAlgo.h"
    "/media/Linux/code/lib/openexr-git/IlmBase/Imath/ImathVec.h"
    )
endif()

