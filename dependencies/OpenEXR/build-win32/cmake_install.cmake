# Install script for directory: D:/code/applications/mrViewer/dependencies/OpenEXR

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
   "D:/code/lib/Windows_32/include/OpenEXR/OpenEXRConfig.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/include/OpenEXR" TYPE FILE FILES "D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/config/OpenEXRConfig.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/TechnicalIntroduction.pdf;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/ReadingAndWritingImageFiles.pdf;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/OpenEXRFileLayout.pdf;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/MultiViewOpenEXR.pdf;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/InterpretingDeepPixels.pdf;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/TheoryDeepPixels.pdf")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0" TYPE FILE FILES
    "D:/code/applications/mrViewer/dependencies/OpenEXR/doc/TechnicalIntroduction.pdf"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/doc/ReadingAndWritingImageFiles.pdf"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/doc/OpenEXRFileLayout.pdf"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/doc/MultiViewOpenEXR.pdf"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/doc/InterpretingDeepPixels.pdf"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/doc/TheoryDeepPixels.pdf"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/main.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/drawImage.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceExamples.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceTiledExamples.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/generalInterfaceExamples.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/lowLevelIoExamples.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/previewImageExamples.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/generalInterfaceTiledExamples.cpp;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/generalInterfaceTiledExamples.h;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/drawImage.h;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceExamples.h;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/generalInterfaceExamples.h;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/rgbaInterfaceTiledExamples.h;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/lowLevelIoExamples.h;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/previewImageExamples.h;D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples/namespaceAlias.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/share/doc/OpenEXR-2.2.0/examples" TYPE FILE FILES
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/main.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/drawImage.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceExamples.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceTiledExamples.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceExamples.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/lowLevelIoExamples.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/previewImageExamples.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceTiledExamples.cpp"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceTiledExamples.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/drawImage.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceExamples.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/generalInterfaceExamples.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/rgbaInterfaceTiledExamples.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/lowLevelIoExamples.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/previewImageExamples.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImfExamples/namespaceAlias.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImf/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImfUtil/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImfExamples/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImfTest/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImfUtilTest/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImfFuzzTest/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/exrheader/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/exrmaketiled/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/exrstdattr/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/exrmakepreview/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/exrenvmap/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/exrmultiview/cmake_install.cmake")
  include("D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/exrmultipart/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
