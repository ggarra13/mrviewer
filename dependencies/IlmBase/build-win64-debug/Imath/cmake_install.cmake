# Install script for directory: F:/code/lib/openexr-git/IlmBase/Imath

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "F:/code/lib/openexr-git/IlmBase/build-win64-debug/Imath/Imath-2_2.lib")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "F:/code/lib/openexr-git/IlmBase/build-win64-debug/Imath/Imath-2_2.dll")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathBoxAlgo.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathBox.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathColorAlgo.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathColor.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathEuler.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathExc.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathExport.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathForward.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathFrame.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathFrustum.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathFrustumTest.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathFun.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathGL.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathGLU.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathHalfLimits.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathInt64.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathInterval.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathLimits.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathLineAlgo.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathLine.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathMath.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathMatrixAlgo.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathMatrix.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathNamespace.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathPlane.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathPlatform.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathQuat.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathRandom.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathRoots.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathShear.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathSphere.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathVecAlgo.h"
    "F:/code/lib/openexr-git/IlmBase/Imath/ImathVec.h"
    )
endif()

