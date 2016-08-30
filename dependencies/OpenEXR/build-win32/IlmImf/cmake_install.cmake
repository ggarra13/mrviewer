# Install script for directory: D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf

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
   "D:/code/lib/Windows_32/lib/IlmImf-2_2.lib")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImf/IlmImf-2_2.lib")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "D:/code/lib/Windows_32/lib/IlmImf-2_2.dll")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/lib" TYPE SHARED_LIBRARY FILES "D:/code/applications/mrViewer/dependencies/OpenEXR/build-win32/IlmImf/IlmImf-2_2.dll")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "D:/code/lib/Windows_32/include/OpenEXR/ImfForward.h;D:/code/lib/Windows_32/include/OpenEXR/ImfExport.h;D:/code/lib/Windows_32/include/OpenEXR/ImfAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfBoxAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfCRgbaFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfChannelList.h;D:/code/lib/Windows_32/include/OpenEXR/ImfChannelListAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfCompressionAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDoubleAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFloatAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFrameBuffer.h;D:/code/lib/Windows_32/include/OpenEXR/ImfHeader.h;D:/code/lib/Windows_32/include/OpenEXR/ImfIO.h;D:/code/lib/Windows_32/include/OpenEXR/ImfInputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfIntAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfLineOrderAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfMatrixAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfOpaqueAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfOutputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfRgbaFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfStringAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfVecAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfHuf.h;D:/code/lib/Windows_32/include/OpenEXR/ImfWav.h;D:/code/lib/Windows_32/include/OpenEXR/ImfLut.h;D:/code/lib/Windows_32/include/OpenEXR/ImfArray.h;D:/code/lib/Windows_32/include/OpenEXR/ImfCompression.h;D:/code/lib/Windows_32/include/OpenEXR/ImfLineOrder.h;D:/code/lib/Windows_32/include/OpenEXR/ImfName.h;D:/code/lib/Windows_32/include/OpenEXR/ImfPixelType.h;D:/code/lib/Windows_32/include/OpenEXR/ImfVersion.h;D:/code/lib/Windows_32/include/OpenEXR/ImfXdr.h;D:/code/lib/Windows_32/include/OpenEXR/ImfConvert.h;D:/code/lib/Windows_32/include/OpenEXR/ImfPreviewImage.h;D:/code/lib/Windows_32/include/OpenEXR/ImfPreviewImageAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfChromaticities.h;D:/code/lib/Windows_32/include/OpenEXR/ImfChromaticitiesAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfKeyCode.h;D:/code/lib/Windows_32/include/OpenEXR/ImfKeyCodeAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTimeCode.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTimeCodeAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfRational.h;D:/code/lib/Windows_32/include/OpenEXR/ImfRationalAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFramesPerSecond.h;D:/code/lib/Windows_32/include/OpenEXR/ImfStandardAttributes.h;D:/code/lib/Windows_32/include/OpenEXR/ImfEnvmap.h;D:/code/lib/Windows_32/include/OpenEXR/ImfEnvmapAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfInt64.h;D:/code/lib/Windows_32/include/OpenEXR/ImfRgba.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTileDescription.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTileDescriptionAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTiledInputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTiledOutputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTiledRgbaFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfRgbaYca.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTestFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfThreading.h;D:/code/lib/Windows_32/include/OpenEXR/ImfB44Compressor.h;D:/code/lib/Windows_32/include/OpenEXR/ImfStringVectorAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfMultiView.h;D:/code/lib/Windows_32/include/OpenEXR/ImfAcesFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfMultiPartOutputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfGenericOutputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfMultiPartInputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfGenericInputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfPartType.h;D:/code/lib/Windows_32/include/OpenEXR/ImfPartHelper.h;D:/code/lib/Windows_32/include/OpenEXR/ImfOutputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTiledOutputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfInputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfTiledInputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepScanLineOutputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepScanLineOutputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepScanLineInputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepScanLineInputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepTiledInputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepTiledInputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepTiledOutputFile.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepTiledOutputPart.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepFrameBuffer.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepCompositing.h;D:/code/lib/Windows_32/include/OpenEXR/ImfCompositeDeepScanLine.h;D:/code/lib/Windows_32/include/OpenEXR/ImfNamespace.h;D:/code/lib/Windows_32/include/OpenEXR/ImfMisc.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepImageState.h;D:/code/lib/Windows_32/include/OpenEXR/ImfDeepImageStateAttribute.h;D:/code/lib/Windows_32/include/OpenEXR/ImfFloatVectorAttribute.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "D:/code/lib/Windows_32/include/OpenEXR" TYPE FILE FILES
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfForward.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfExport.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfBoxAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCRgbaFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChannelList.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChannelListAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCompressionAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDoubleAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFloatAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFrameBuffer.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfHeader.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfIO.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfInputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfIntAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfLineOrderAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMatrixAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfOpaqueAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfOutputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRgbaFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfStringAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfVecAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfHuf.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfWav.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfLut.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfArray.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCompression.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfLineOrder.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfName.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPixelType.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfVersion.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfXdr.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfConvert.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPreviewImage.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPreviewImageAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChromaticities.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChromaticitiesAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfKeyCode.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfKeyCodeAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTimeCode.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTimeCodeAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRational.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRationalAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFramesPerSecond.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfStandardAttributes.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfEnvmap.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfEnvmapAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfInt64.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRgba.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTileDescription.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTileDescriptionAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledInputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledOutputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledRgbaFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRgbaYca.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTestFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfThreading.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfB44Compressor.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfStringVectorAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMultiView.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfAcesFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMultiPartOutputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfGenericOutputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMultiPartInputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfGenericInputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPartType.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPartHelper.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfOutputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledOutputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfInputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledInputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineOutputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineOutputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineInputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineInputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledInputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledInputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledOutputFile.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledOutputPart.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepFrameBuffer.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepCompositing.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCompositeDeepScanLine.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfNamespace.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMisc.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepImageState.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepImageStateAttribute.h"
    "D:/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFloatVectorAttribute.h"
    )
endif()

