project( mrViewer )
cmake_policy( SET CMP0074 NEW )


configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/gui/mrvVersion.cpp.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/gui/mrvVersion.cpp" @ONLY )

include( cmake/functions.cmake )
include( cmake/find_packages.cmake )




set( FL_SOURCES
  gui/aviSave.fl
  gui/AboutUI.fl
  gui/mrvAudioOffset.fl
  gui/mrvGl3dView.fl
  gui/EXROptionsUI.fl
  gui/OIIOOptionsUI.fl
  gui/mrvEDLWindowUI.fl
  gui/mrvColorAreaUI.fl
  gui/mrvIccProfileUI.fl
  gui/mrvSOPNode.fl
  gui/mrvHotkeyUI.fl
  gui/keyboard_ui.fl
  gui/mrvImageInfo.fl
  gui/mrvReelUI.fl
  gui/mrvStereoUI.fl
  gui/mrvPreferencesUI.fl
  gui/mrvHistogramUI.fl
  gui/mrvVectorscopeUI.fl
  gui/mrvColorControlsUI.fl
  gui/mrvWaveformUI.fl
  gui/WandOptionsUI.fl
  gui/make_ocio_chooser.fl
  gui/mrViewer.fl
)

FLTK_RUN_FLUID( mrViewer_FLTK_UI_SRCS "${FL_SOURCES}" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )

# For windows, this is a non-MFC application
set( CMAKE_MFC_FLAG 0 )

set( SOURCES

  ${mrViewer_FLTK_UI_SRCS}

  audio/mrvPortAudioEngine.cpp
  audio/mrvWaveEngine.cpp
  audio/mrvALSAEngine.cpp

  # Image files
  core/ctlToLut.cpp
  core/CMedia.cpp
  core/CMedia_audio.cpp
  core/mrvColorProfile.cpp
  core/mrvFrame.cpp
  core/mrvHome.cpp
  core/guessImage.cpp
  core/aviImage.cpp
  core/aviImage_save.cpp
  core/clonedImage.cpp
  core/ddsImage.cpp
 # core/dvdImage.cpp
  core/exrImage.cpp
  core/hdrImage.cpp
  core/iffImage.cpp
  core/mapImage.cpp
  core/mrayImage.cpp
  core/mrvBlackImage.cpp
  core/mrvColorBarsImage.cpp
  core/pxrzImage.cpp
  core/shmapImage.cpp
  core/slateImage.cpp
  core/smpteImage.cpp
  core/stubImage.cpp
  core/wandImage.cpp
  core/oiioImage.cpp
  core/rawImage.cpp
  core/picImage.cpp
  core/R3dImage.cpp
  core/brawImage.cpp

  core/mrSocket.cpp
  core/mrvClient.cpp
  core/mrvServer.cpp
  core/mrvAudioEngine.cpp
  core/mrvColor.cpp
  core/mrvColorSpaces.cpp
  core/mrvLicensing.cpp
  core/mrvPacketQueue.cpp
  core/mrvPlayback.cpp
  # core/mrvScale.cpp
  core/mrvString.cpp
  core/mrvTimer.cpp
  # core/mrvOSX.cpp
  core/mrvACES.cpp
  # core/generic/mrEvent.cpp

  #  core/mrvPC1.cpp
  core/Sequence.cpp
  core/mrStackTrace.cpp
  core/mrvCPU.cpp
  core/mrvImageOpts.cpp
  core/mrvColorOps.cpp
  core/mrvFrame.cpp
  core/mrvOS.cpp
  core/mrvOTIO.cpp

  gui/FLU/Flu_Combo_Box.cpp       # needed
  gui/FLU/Flu_Combo_Tree.cpp      # needed
  gui/FLU/Flu_Combo_List.cpp      # needed
  gui/FLU/Flu_Separator.cpp       # needed
  #gui/FLU/Flu_Tree_Browser.cpp   # not needed- replaced by Fl_Tree
  #gui/FLU/FluSimpleString.cpp    # not needed
  gui/FLU/Flu_Button.cpp          # needed
  gui/FLU/Flu_Return_Button.cpp   # needed
  gui/FLU/Flu_Label.cpp           # needed
  # gui/FLU/flu_filename_list.cxx # not needed - replaced by fl_filename_list
  # gui/FLU/flu_scandir.c         # not needed - not used
  # gui/FLU/flu_numericsort.c     # not needed - not used
  gui/FLU/flu_file_chooser_pixmaps.cpp # needed
  gui/FLU/Flu_Wrap_Group.cpp      # needed
  gui/FLU/Flu_File_Chooser.cpp    # needed
  gui/FLU/flu_pixmaps.cpp         # needed

  gui/MyPack.cpp
  gui/mrvButton.cpp
  gui/mrvActionWindow.cpp
  gui/mrvAsk.cpp
  gui/mrvCTLBrowser.cpp
  gui/mrvOCIOBrowser.cpp

  gui/mrvColorSchemes.cpp
  gui/mrvDoubleWindow.cpp
  gui/mrvGlWindow3d.cpp
  gui/mrvHotkey.cpp
  gui/mrvColorOps.cpp
  gui/mrvMedia.cpp
  gui/mrvBrowser.cpp
  gui/mrvCollapsibleGroup.cpp
  gui/mrvColorInfo.cpp
  gui/mrvChoice.cpp
  gui/mrvEDLGroup.cpp
  gui/mrvElement.cpp
  gui/mrvFileRequester.cpp
  gui/mrvFLTKHandler.cpp
  gui/mrvFontsWindowUI.cpp
  gui/mrvHistogram.cpp
  gui/mrvImageView.cpp
  gui/mrvImageView_shapes.cpp
  gui/mrvImageBrowser.cpp
  gui/mrvImageInformation.cpp
  gui/mrvIO.cpp
  gui/mrvLanguages.cpp
  gui/mrvLogDisplay.cpp
  gui/mrvMediaTrack.cpp
  gui/mrvMainWindow.cpp
  gui/mrvMultilineInput.cpp
  gui/mrvPopupMenu.cpp
  gui/mrvPreferences.cpp
  gui/mrvProgressReport.cpp
  gui/mrvReel.cpp
  gui/mrvSave.cpp
  gui/mrvSlider.cpp
  gui/mrvStereoWindow.cpp
  gui/mrvTable.cpp
  gui/mrvTextDisplay.cpp
  gui/mrvTimecode.cpp
  gui/mrvTimeline.cpp
  gui/mrvTree.cpp
  gui/mrvVectorscope.cpp
  gui/mrvVersion.cpp
  gui/mrvVolumeSlider.cpp
  gui/mrvWaveform.cpp

  video/mrvDrawEngine.cpp
  video/mrvGLEngine.cpp
  video/mrvGLShader.cpp
  video/mrvGLQuad.cpp
  video/mrvGLCube.cpp
  video/mrvGLSphere.cpp
  video/mrvCSPUtils.cpp
  video/mrvGLLut3d.cpp
  video/mrvGLShape.cpp

  standalone/mrvRoot.cpp
  standalone/mrvCommandLine.cpp
  standalone/main.cpp

  ${BlackMagicRAW_SOURCES}
  )

  include( cmake/translations.cmake )

  if (APPLE)
    list( append SOURCES ${SOURCES} gui/mrvMainWindow.mm )
  endif()

  if( R3DSDK_FOUND )
    list( APPEND SOURCES core/R3dImage.cpp )
  endif()

IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
  list( APPEND SOURCES ${PROJECT_IDL_FILES} ${GENERATED_FILES_IDL} ${SOURCES} )
  set( SOURCES gui/resource.rc ${SOURCES} )
ENDIF()


#
# RELEASE VERSION
#
if(NOT WIN32)
  if(COMMAND cmake_policy)
       cmake_policy(SET CMP0003 NEW)
  endif()
endif()


ADD_EXECUTABLE( mrViewer WIN32 ${SOURCES} )
TARGET_LINK_LIBRARIES( mrViewer LINK_PUBLIC ${LIBRARIES} ACESclip AMF )

#
# CMake 3.15 new variable for windows runtime library
#
set_TARGET_PROPERTIES( mrViewer
  PROPERTIES
  LINK_FLAGS "${LINK_FLAGS}"
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
  )


######################################################
#
# Installation
#
######################################################

if( WIN32 )
    set(EXE_DIR $ENV{DIST}/bin )
else()
    set(EXE_DIR ${CMAKE_PREFIX_PATH}/bin )
endif()
set( SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

if(UNIX AND NOT APPLE)

  set( LINK_FLAGS ${LINK_FLAGS})

  execute_process( COMMAND "dos2unix" "mrViewer.sh" "environment.sh" "../utils/libs.rb" WORKING_DIRECTORY "${SOURCE_DIR}/bin" )
  file( COPY ${SOURCE_DIR}/bin/mrViewer.sh ${SOURCE_DIR}/bin/environment.sh
	DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
endif()





if( UNIX )
  set( FFMPEG_ROOT ${EXE_DIR} )
else()
  set( FFMPEG_ROOT $ENV{FFMPEG_ROOT}/bin )
endif()

message( STATUS "Copying ${EXE_DIR} files..." )
FILE(GLOB scriptfiles ${SOURCE_DIR}/bin/*.rb ${SOURCE_DIR}/bin/*.py )
FILE(GLOB ffmpegfiles ${FFMPEG_ROOT}/ff*.exe ${FFMPEG_ROOT}/av*.dll ${FFMPEG_ROOT}/sw*.dll ${FFMPEG_ROOT}/pos*.dll)
FILE(COPY ${ffmpegfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB exrfiles ${EXE_DIR}/exr* )
FILE(COPY ${exrfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB ociofiles ${EXE_DIR}/ocio* ${EXE_DIR}/OpenColor*.dll )
FILE(COPY ${ociofiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB oiiofiles ${EXE_DIR}/oiio* ${EXE_DIR}/OpenImage*.dll )
FILE(COPY ${oiiofiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/../otio ${CMAKE_BINARY_DIR}/../otio/adapters )
FILE(GLOB otiofiles "${SOURCE_DIR}/otio/adapters/*" )
FILE(COPY ${otiofiles} DESTINATION ${CMAKE_BINARY_DIR}/../otio/adapters )
FILE(GLOB otiofiles "${SOURCE_DIR}/otio/*" )
FILE(COPY ${otiofiles} DESTINATION ${CMAKE_BINARY_DIR}/../otio/ )
IF (WIN32)
  FILE(GLOB dllfiles ${CMAKE_PREFIX_PATH}/bin/*.dll ${CMAKE_PREFIX_PATH}/lib/*.dll ${CMAKE_BINARY_DIR}/../lib/*.dll )
  LIST( FILTER dllfiles EXCLUDE REGEX .*fltk.*.dll )

  FILE( COPY ${dllfiles} DESTINATION ${EXE_DIR} )
  FILE( GLOB dllfiles ${EXE_DIR}/*.dll )
  FILE( COPY ${dllfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
ENDIF()


FILE( COPY ${SOURCE_DIR}/colors/
    DESTINATION ${CMAKE_BINARY_DIR}/../colors )

  INSTALL( DIRECTORY ${SOURCE_DIR}/colors/ DESTINATION colors )
  INSTALL( DIRECTORY ${SOURCE_DIR}/shaders/ DESTINATION shaders )
  INSTALL( DIRECTORY ${SOURCE_DIR}/ctl/ DESTINATION ctl )
  INSTALL( DIRECTORY ${SOURCE_DIR}/otio/ DESTINATION otio )
  INSTALL( DIRECTORY ${SOURCE_DIR}/ocio/ DESTINATION ocio )
  INSTALL( DIRECTORY ${SOURCE_DIR}/docs/en DESTINATION docs )
  INSTALL( DIRECTORY ${SOURCE_DIR}/docs/es DESTINATION docs )
IF( WIN32 OR WIN64 )
  FILE( COPY ${SOURCE_DIR}/share/
  DESTINATION ${CMAKE_BINARY_DIR}/../share )
  INSTALL( DIRECTORY ${CMAKE_BINARY_DIR}/../bin/ DESTINATION bin )
ENDIF()
  INSTALL( DIRECTORY ${SOURCE_DIR}/share/ DESTINATION share )
if (NOT APPLE )
  INSTALL( FILES
     ${SOURCE_DIR}/docs/HISTORY.txt
     ${SOURCE_DIR}/docs/LICENSE.txt
     DESTINATION ./ )
endif()

INSTALL( PROGRAMS ${ffmpegfiles} ${scriptfiles} ${exrfiles}
      ${oiiofiles} ${ociofiles} ${dllfiles}
   DESTINATION bin )
INSTALL( TARGETS mrViewer DESTINATION bin )
INSTALL( DIRECTORY ${CMAKE_BINARY_DIR}/../lib/ DESTINATION lib )

IF(WIN32)

  # set the type of package to build.

  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  set(CPACK_NSIS_MODIFY_PATH ON)

  #  set(CPACK_GENERATOR NSIS )

  set(CPACK_GENERATOR ZIP NSIS )

  #  set(CMAKE_INSTALL_DEBUG_LIBRARIES true)


  set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/icons/viewer.ico)
  set(CPACK_NSIS_MUI_UNICON ${PROJECT_SOURCE_DIR}/icons/viewer.ico)

  if (CMAKE_BUILD_ARCH EQUAL 32)

  set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES" )

  elseif (CMAKE_BUILD_ARCH EQUAL 64)

  set( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" )

  endif (CMAKE_BUILD_ARCH EQUAL 32)

  set( CPACK_NSIS_DISPLAY_NAME "mrViewer-${CMAKE_BUILD_ARCH} v${SHORTVERSION}" )
  set( CPACK_NSIS_PACKAGE_NAME "mrViewer" )
  set( CPACK_PACKAGE_VENDOR "FilmAura" )
  set( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrViewerPackageName})
  set( CPACK_PACKAGE_EXECUTABLES "mrViewer" "mrViewer-${CMAKE_BUILD_ARCH}-v${SHORTVERSION}" )
  set( CPACK_CREATE_DESKTOP_LINKS "mrViewer" "mrViewer-${CMAKE_BUILD_ARCH}-v${SHORTVERSION}" )

  set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

  INCLUDE( cmake/NSISRegistry.cmake )

ELSEIF ( APPLE )
  set(CPACK_GENERATOR Bundle )

  set( INSTALL_NAME ${PROJECT_NAME} )

  configure_file(
  ../../etc/OSX/startup.sh.in
  ${PROJECT_BINARY_DIR}/startup.sh)

  configure_file(
  ../../etc/OSX/Info.plist.in
  ${PROJECT_BINARY_DIR}/Info.plist )

  file( COPY ${PROJECT_BINARY_DIR}/Info.plist
    DESTINATION ${PROJECT_BINARY_DIR}/bin/mrViewer.app/Contents/ )
  set(CPACK_PACKAGE_ICON ${SOURCE_DIR}/etc/OSX/mrViewer.icns )
  set(CPACK_BUNDLE_NAME ${INSTALL_NAME} )
  set(CPACK_BUNDLE_ICON ${SOURCE_DIR}/etc/OSX/mrViewer.icns )
  set(CPACK_BUNDLE_PLIST ${PROJECT_BINARY_DIR}/Info.plist )
  set(CPACK_BUNDLE_STARTUP_COMMAND ${PROJECT_BINARY_DIR}/startup.sh)


ELSEIF (UNIX)

  # set the type of package to build.
  INSTALL( DIRECTORY ${SOURCE_DIR}/bin/ DESTINATION bin )



  CONFIGURE_FILE( ${SOURCE_DIR}/etc/Linux/mrViewer.desktop.in
      "${SOURCE_DIR}/etc/mrViewer-v${SHORTVERSION}.desktop" )

  set(MRV_DESKTOP_DIR     /usr/share/applications/)
  set(MRV_PIXMAPS_DIR     /usr/share/icons/hicolor/32x32/apps/)

  INSTALL(FILES "${SOURCE_DIR}/etc/mrViewer-v${SHORTVERSION}.desktop"
    DESTINATION ${MRV_DESKTOP_DIR})
  INSTALL(FILES ${SOURCE_DIR}/etc/mrViewer.png
    DESTINATION ${MRV_PIXMAPS_DIR})

  # try to set up the menu sys

  set(CPACK_GENERATOR DEB RPM TGZ )

  # set the installation directory.

  set(CPACK_SET_DESTDIR true) # Needed
  set(CPACK_INSTALL_PREFIX /usr/local/${mrViewerShortName})

  # Configure install/uninstall scripts

  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/../../etc/Linux/install.sh.in
  ${PROJECT_BINARY_DIR}/etc/Linux/install.sh
  @ONLY)
  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/../../etc/Linux/uninstall.sh.in
  ${PROJECT_BINARY_DIR}/etc/Linux/uninstall.sh
  @ONLY)
  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/../../etc/Linux/postinst.in
  ${PROJECT_BINARY_DIR}/etc/Linux/postinst
  @ONLY)
  CONFIGURE_FILE(
  ${PROJECT_SOURCE_DIR}/../../etc/Linux/postrm.in
  ${PROJECT_BINARY_DIR}/etc/Linux/postrm
  @ONLY)

    # set RPM options.

  set(CPACK_RPM_PACKAGE_NAME ${PROJECT_NAME})
  set(CPACK_RPM_PACKAGE_RELOCATABLE true)
  set(CPACK_RPM_PACKAGE_AUTOREQ false)
  set(CPACK_RPM_PACKAGE_AUTOPROV true)
  set(CPACK_RPM_COMPRESSION_TYPE gzip )


  # set Debian options.

  set(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})

  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  ${PROJECT_BINARY_DIR}/etc/Linux/postinst
  ${PROJECT_BINARY_DIR}/etc/Linux/postrm)

  set(
  CPACK_RPM_POST_INSTALL_SCRIPT_FILE
  ${PROJECT_BINARY_DIR}/etc/Linux/install.sh)
  set(
  CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
  ${PROJECT_BINARY_DIR}/etc/Linux/uninstall.sh)


ENDIF(WIN32)

set(CPACK_PACKAGE_VENDOR "Film Aura, LLC")
set(CPACK_PACKAGE_VERSION_MAJOR ${VersionMajor})
set(CPACK_PACKAGE_VERSION_MINOR ${VersionMinor})
set(CPACK_PACKAGE_VERSION_PATCH ${VersionPatch})
set(CPACK_PACKAGE_VERSION "${SHORTVERSION}" )
set(CPACK_PACKAGE_FILE_NAME "${mrViewerPackageName}" )
set(CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")
# set(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/README.txt)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "mrViewer provides professional flipbook, audio and video playback.")


#set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${mrViewerPackageName} )
set(CPACK_PACKAGE_EXECUTABLES mrViewer mrViewer)
set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/../../docs/LICENSE.txt)


set( CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake" )
include(CPack)

#set( TEST_BUNDLE 1 )

IF( TEST_BUNDLE  )
  # A dummy target so that we can test post_build commands faster
  MESSAGE( STATUS "NO CPACK RUN" )
  ADD_CUSTOM_TARGET( bundle ALL
    COMMAND echo here we would run cpack
    DEPENDS translations
  )
ELSE()
  ADD_CUSTOM_TARGET( bundle utils/libs.rb
    COMMAND "${CMAKE_CPACK_COMMAND}"
    "-C" "$<CONFIGURATION>"
    "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../.."
    COMMENT "Running CPack. Please wait..."
    USES_TERMINAL
    DEPENDS translations )
ENDIF()

IF (WIN32)
  ADD_CUSTOM_COMMAND( TARGET bundle
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -D CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -D VERSION=${VERSION} -D SHORTVERSION=${SHORTVERSION} "-P" "${SOURCE_DIR}/chocolatey/mrViewer/CMakeLists.txt"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
ENDIF( WIN32 )
