# Install script for directory: /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf

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
      "$ENV{DESTDIR}/usr/local/lib/libIlmImf-2_2.so.22.0.0"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImf-2_2.so.22"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImf-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libIlmImf-2_2.so.22.0.0;/usr/local/lib/libIlmImf-2_2.so.22;/usr/local/lib/libIlmImf-2_2.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/lib" TYPE SHARED_LIBRARY FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImf/libIlmImf-2_2.so.22.0.0"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImf/libIlmImf-2_2.so.22"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/IlmImf/libIlmImf-2_2.so"
    )
  foreach(file
      "$ENV{DESTDIR}/usr/local/lib/libIlmImf-2_2.so.22.0.0"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImf-2_2.so.22"
      "$ENV{DESTDIR}/usr/local/lib/libIlmImf-2_2.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/usr/local/lib:"
           NEW_RPATH "")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/OpenEXR/ImfForward.h;/usr/local/include/OpenEXR/ImfExport.h;/usr/local/include/OpenEXR/ImfAttribute.h;/usr/local/include/OpenEXR/ImfBoxAttribute.h;/usr/local/include/OpenEXR/ImfCRgbaFile.h;/usr/local/include/OpenEXR/ImfChannelList.h;/usr/local/include/OpenEXR/ImfChannelListAttribute.h;/usr/local/include/OpenEXR/ImfCompressionAttribute.h;/usr/local/include/OpenEXR/ImfDoubleAttribute.h;/usr/local/include/OpenEXR/ImfFloatAttribute.h;/usr/local/include/OpenEXR/ImfFrameBuffer.h;/usr/local/include/OpenEXR/ImfHeader.h;/usr/local/include/OpenEXR/ImfIO.h;/usr/local/include/OpenEXR/ImfInputFile.h;/usr/local/include/OpenEXR/ImfIntAttribute.h;/usr/local/include/OpenEXR/ImfLineOrderAttribute.h;/usr/local/include/OpenEXR/ImfMatrixAttribute.h;/usr/local/include/OpenEXR/ImfOpaqueAttribute.h;/usr/local/include/OpenEXR/ImfOutputFile.h;/usr/local/include/OpenEXR/ImfRgbaFile.h;/usr/local/include/OpenEXR/ImfStringAttribute.h;/usr/local/include/OpenEXR/ImfVecAttribute.h;/usr/local/include/OpenEXR/ImfHuf.h;/usr/local/include/OpenEXR/ImfWav.h;/usr/local/include/OpenEXR/ImfLut.h;/usr/local/include/OpenEXR/ImfArray.h;/usr/local/include/OpenEXR/ImfCompression.h;/usr/local/include/OpenEXR/ImfLineOrder.h;/usr/local/include/OpenEXR/ImfName.h;/usr/local/include/OpenEXR/ImfPixelType.h;/usr/local/include/OpenEXR/ImfVersion.h;/usr/local/include/OpenEXR/ImfXdr.h;/usr/local/include/OpenEXR/ImfConvert.h;/usr/local/include/OpenEXR/ImfPreviewImage.h;/usr/local/include/OpenEXR/ImfPreviewImageAttribute.h;/usr/local/include/OpenEXR/ImfChromaticities.h;/usr/local/include/OpenEXR/ImfChromaticitiesAttribute.h;/usr/local/include/OpenEXR/ImfKeyCode.h;/usr/local/include/OpenEXR/ImfKeyCodeAttribute.h;/usr/local/include/OpenEXR/ImfTimeCode.h;/usr/local/include/OpenEXR/ImfTimeCodeAttribute.h;/usr/local/include/OpenEXR/ImfRational.h;/usr/local/include/OpenEXR/ImfRationalAttribute.h;/usr/local/include/OpenEXR/ImfFramesPerSecond.h;/usr/local/include/OpenEXR/ImfStandardAttributes.h;/usr/local/include/OpenEXR/ImfEnvmap.h;/usr/local/include/OpenEXR/ImfEnvmapAttribute.h;/usr/local/include/OpenEXR/ImfInt64.h;/usr/local/include/OpenEXR/ImfRgba.h;/usr/local/include/OpenEXR/ImfTileDescription.h;/usr/local/include/OpenEXR/ImfTileDescriptionAttribute.h;/usr/local/include/OpenEXR/ImfTiledInputFile.h;/usr/local/include/OpenEXR/ImfTiledOutputFile.h;/usr/local/include/OpenEXR/ImfTiledRgbaFile.h;/usr/local/include/OpenEXR/ImfRgbaYca.h;/usr/local/include/OpenEXR/ImfTestFile.h;/usr/local/include/OpenEXR/ImfThreading.h;/usr/local/include/OpenEXR/ImfB44Compressor.h;/usr/local/include/OpenEXR/ImfStringVectorAttribute.h;/usr/local/include/OpenEXR/ImfMultiView.h;/usr/local/include/OpenEXR/ImfAcesFile.h;/usr/local/include/OpenEXR/ImfMultiPartOutputFile.h;/usr/local/include/OpenEXR/ImfGenericOutputFile.h;/usr/local/include/OpenEXR/ImfMultiPartInputFile.h;/usr/local/include/OpenEXR/ImfGenericInputFile.h;/usr/local/include/OpenEXR/ImfPartType.h;/usr/local/include/OpenEXR/ImfPartHelper.h;/usr/local/include/OpenEXR/ImfOutputPart.h;/usr/local/include/OpenEXR/ImfTiledOutputPart.h;/usr/local/include/OpenEXR/ImfInputPart.h;/usr/local/include/OpenEXR/ImfTiledInputPart.h;/usr/local/include/OpenEXR/ImfDeepScanLineOutputFile.h;/usr/local/include/OpenEXR/ImfDeepScanLineOutputPart.h;/usr/local/include/OpenEXR/ImfDeepScanLineInputFile.h;/usr/local/include/OpenEXR/ImfDeepScanLineInputPart.h;/usr/local/include/OpenEXR/ImfDeepTiledInputFile.h;/usr/local/include/OpenEXR/ImfDeepTiledInputPart.h;/usr/local/include/OpenEXR/ImfDeepTiledOutputFile.h;/usr/local/include/OpenEXR/ImfDeepTiledOutputPart.h;/usr/local/include/OpenEXR/ImfDeepFrameBuffer.h;/usr/local/include/OpenEXR/ImfDeepCompositing.h;/usr/local/include/OpenEXR/ImfCompositeDeepScanLine.h;/usr/local/include/OpenEXR/ImfNamespace.h;/usr/local/include/OpenEXR/ImfMisc.h;/usr/local/include/OpenEXR/ImfDeepImageState.h;/usr/local/include/OpenEXR/ImfDeepImageStateAttribute.h;/usr/local/include/OpenEXR/ImfFloatVectorAttribute.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/OpenEXR" TYPE FILE FILES
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfForward.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfExport.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfBoxAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCRgbaFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChannelList.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChannelListAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCompressionAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDoubleAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFloatAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFrameBuffer.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfHeader.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfIO.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfInputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfIntAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfLineOrderAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMatrixAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfOpaqueAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfOutputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRgbaFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfStringAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfVecAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfHuf.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfWav.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfLut.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfArray.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCompression.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfLineOrder.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfName.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPixelType.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfVersion.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfXdr.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfConvert.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPreviewImage.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPreviewImageAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChromaticities.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfChromaticitiesAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfKeyCode.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfKeyCodeAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTimeCode.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTimeCodeAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRational.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRationalAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFramesPerSecond.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfStandardAttributes.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfEnvmap.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfEnvmapAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfInt64.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRgba.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTileDescription.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTileDescriptionAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledInputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledOutputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledRgbaFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfRgbaYca.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTestFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfThreading.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfB44Compressor.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfStringVectorAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMultiView.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfAcesFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMultiPartOutputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfGenericOutputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMultiPartInputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfGenericInputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPartType.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfPartHelper.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfOutputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledOutputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfInputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfTiledInputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineOutputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineOutputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineInputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepScanLineInputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledInputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledInputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledOutputFile.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepTiledOutputPart.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepFrameBuffer.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepCompositing.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfCompositeDeepScanLine.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfNamespace.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfMisc.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepImageState.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfDeepImageStateAttribute.h"
    "/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/IlmImf/ImfFloatVectorAttribute.h"
    )
endif()

