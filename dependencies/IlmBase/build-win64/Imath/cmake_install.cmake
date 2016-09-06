# Install script for directory: D:/code/applications/mrViewer/dependencies/IlmBase/Imath

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/code/lib/Windows_64")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/code/applications/mrViewer/dependencies/IlmBase/build-win64/Imath/Imath-2_2.lib")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "D:/code/applications/mrViewer/dependencies/IlmBase/build-win64/Imath/Imath-2_2.dll")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathBoxAlgo.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathBox.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathColorAlgo.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathColor.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathEuler.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathExc.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathExport.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathForward.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFrame.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFrustum.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFrustumTest.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathFun.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathGL.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathGLU.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathHalfLimits.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathInt64.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathInterval.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathLimits.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathLineAlgo.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathLine.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathMath.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathMatrixAlgo.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathMatrix.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathNamespace.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathPlane.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathPlatform.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathQuat.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathRandom.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathRoots.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathShear.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathSphere.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathVecAlgo.h"
    "D:/code/applications/mrViewer/dependencies/IlmBase/Imath/ImathVec.h"
    )
endif()

