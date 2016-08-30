# Install script for directory: /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR

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
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/OpenEXR/OpenEXRConfig.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/OpenEXR" TYPE FILE FILES "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/config/OpenEXRConfig.h")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/doc/OpenEXR-2.2.0/TechnicalIntroduction.pdf;/usr/local/share/doc/OpenEXR-2.2.0/ReadingAndWritingImageFiles.pdf;/usr/local/share/doc/OpenEXR-2.2.0/OpenEXRFileLayout.pdf;/usr/local/share/doc/OpenEXR-2.2.0/MultiViewOpenEXR.pdf;/usr/local/share/doc/OpenEXR-2.2.0/InterpretingDeepPixels.pdf;/usr/local/share/doc/OpenEXR-2.2.0/TheoryDeepPixels.pdf")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/doc/OpenEXR-2.2.0" TYPE FILE FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/doc/TechnicalIntroduction.pdf"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/doc/ReadingAndWritingImageFiles.pdf"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/doc/OpenEXRFileLayout.pdf"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/doc/MultiViewOpenEXR.pdf"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/doc/InterpretingDeepPixels.pdf"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/doc/TheoryDeepPixels.pdf"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/doc/OpenEXR-2.2.0/examples/main.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/drawImage.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceExamples.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceTiledExamples.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/generalInterfaceExamples.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/lowLevelIoExamples.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/previewImageExamples.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/generalInterfaceTiledExamples.cpp;/usr/local/share/doc/OpenEXR-2.2.0/examples/generalInterfaceTiledExamples.h;/usr/local/share/doc/OpenEXR-2.2.0/examples/drawImage.h;/usr/local/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceExamples.h;/usr/local/share/doc/OpenEXR-2.2.0/examples/generalInterfaceExamples.h;/usr/local/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceTiledExamples.h;/usr/local/share/doc/OpenEXR-2.2.0/examples/lowLevelIoExamples.h;/usr/local/share/doc/OpenEXR-2.2.0/examples/previewImageExamples.h;/usr/local/share/doc/OpenEXR-2.2.0/examples/namespaceAlias.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/doc/OpenEXR-2.2.0/examples" TYPE FILE FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/main.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/drawImage.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceExamples.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceTiledExamples.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceExamples.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/lowLevelIoExamples.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/previewImageExamples.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceTiledExamples.cpp"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceTiledExamples.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/drawImage.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceExamples.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceExamples.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceTiledExamples.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/lowLevelIoExamples.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/previewImageExamples.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/namespaceAlias.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImf/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfUtil/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfExamples/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfTest/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfUtilTest/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImfFuzzTest/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrheader/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrmaketiled/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrmakepreview/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrenvmap/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrmultiview/cmake_install.cmake")
  include("/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrmultipart/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
