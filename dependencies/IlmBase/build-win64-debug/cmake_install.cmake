# Install script for directory: F:/code/lib/openexr-git/IlmBase

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenEXR" TYPE FILE FILES "F:/code/lib/openexr-git/IlmBase/build-win64-debug/config/IlmBaseConfig.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "F:/code/lib/openexr-git/IlmBase/build-win64-debug/IlmBase.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/Half/cmake_install.cmake")
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/Iex/cmake_install.cmake")
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/IexMath/cmake_install.cmake")
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/Imath/cmake_install.cmake")
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/IlmThread/cmake_install.cmake")
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/HalfTest/cmake_install.cmake")
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/IexTest/cmake_install.cmake")
  include("F:/code/lib/openexr-git/IlmBase/build-win64-debug/ImathTest/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

file(WRITE "F:/code/lib/openexr-git/IlmBase/build-win64-debug/${CMAKE_INSTALL_MANIFEST}" "")
foreach(file ${CMAKE_INSTALL_MANIFEST_FILES})
  file(APPEND "F:/code/lib/openexr-git/IlmBase/build-win64-debug/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
endforeach()
