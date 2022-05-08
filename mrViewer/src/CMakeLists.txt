project( mrViewer )
cmake_policy( SET CMP0074 NEW )

IF( NOT CMAKE_MODULE_PATH )
  SET( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../modules )
ENDIF( NOT CMAKE_MODULE_PATH )


CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/gui/mrvVersion.cpp.in
  "${CMAKE_CURRENT_SOURCE_DIR}/gui/mrvVersion.cpp" @ONLY )

#
# These are the libraries we will depend on
#
set( OpenGL_GL_PREFERENCE LEGACY )
#set( OpenGL_GL_PREFERENCE GLVND )

# For window management
FIND_PACKAGE( BuildDir    REQUIRED )    # for 32/64 bits handling (WIN64)
FIND_PACKAGE( OpenGL      REQUIRED )    # for drawing images/shapes
FIND_PACKAGE( Boost       REQUIRED )    # for file system support
ADD_DEFINITIONS( -D BOOST_ALL_DYN_LINK ) # ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} )

if (APPLE)

   set(FLTK_DIR /Users/gga/code/lib/fltk/build CACHE FILEPATH "fltk build dir" FORCE )
   find_package(FLTK REQUIRED NO_MODULE)
   include_directories(${FLTK_INCLUDE_DIRS})
   set( FLTK_LIBRARIES fltk fltk_gl fltk_images )
   include_directories(${OPENGL_INCLUDE_DIRS})

   set( OSX_FRAMEWORKS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio" )
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OSX_FRAMEWORKS}" )
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}
    ${OSX_FRAMEWORKS}")

  # stop OpenGL deprecation warnings on MacOSX > 10.14
  add_definitions(-DGL_SILENCE_DEPRECATION=1)

  set( FLTK_FLUID_EXECUTABLE /usr/local/bin/fluid.app/Contents/MacOS/fluid )
else()

set( FLTK_DIR "~/code/lib/fltk-x11/build-linux" CACHE FILEPATH "fltk build dir" FORCE )

FIND_PACKAGE( FLTK        REQUIRED NO_MODULE )    # for window management

set( FLTK_LIBRARIES fltk fltk_gl fltk_images )


endif()
FIND_PACKAGE( ImageMagick REQUIRED )    # for image formats
FIND_PACKAGE( OpenEXR     REQUIRED )    # for EXR image loading
FIND_PACKAGE( CTL  REQUIRED )           # for CTL
FIND_PACKAGE( OpenEXRCTL  REQUIRED )    # for CTL <-> EXR
FIND_PACKAGE( OCIO        REQUIRED )    # for OCIO color correction
FIND_PACKAGE( OIIO        REQUIRED )    # for OIIO image loading
FIND_PACKAGE( FFMPEG      REQUIRED )    # for mpeg, avi, quicktime, wmv
FIND_PACKAGE( Gettext     REQUIRED )    # for translations
FIND_PACKAGE( TCLAP       REQUIRED )    # for command-line parsing
FIND_PACKAGE( GLEW        REQUIRED )    # for opengl features
FIND_PACKAGE( LibIntl     REQUIRED )
FIND_PACKAGE( SampleICC   REQUIRED )    # For ICC reading
FIND_PACKAGE( LibRaw      REQUIRED )    # for libraw files
FIND_PACKAGE( TinyXML2    REQUIRED )    # for xml reading/writing
FIND_PACKAGE( R3DSDK      REQUIRED )    # for R3D format
FIND_PACKAGE( BlackMagicRAW  REQUIRED )    # for BRAW format
FIND_PACKAGE( OpenTimelineIO REQUIRED )  # for OpenTimelineIO

if( APPLE )
set( USE_PORTAUDIO 1 )
endif()

if( USE_PORTAUDIO )
    FIND_PACKAGE( PortAudio REQUIRED )
endif()

# SET( OpenTime_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../modules )
# FIND_PACKAGE( OpenTime REQUIRED )  # for OTIO
# SET( OpenTimelineIO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../modules )
# FIND_PACKAGE( OpenTimelineIO REQUIRED )  # for OTIO

ADD_DEFINITIONS( -DMAGICKCORE_QUANTUM_DEPTH=32 -DMAGICKCORE_HDRI_ENABLE=1 -DUSE_GETTEXT -D_CRT_SECURE_NO_WARNINGS )

if( PORTAUDIO_FOUND )
    ADD_DEFINITIONS( -DPORTAUDIO )
endif()

IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)

 # ADD_DEFINITIONS( -DMR_SSE -DWIN32  )
  ADD_DEFINITIONS( -DMR_SSE -DWIN32 -D_WIN32_WINNT=0x0601 )

  SET(OS_LIBRARIES GDIplus Winmm ws2_32 Psapi ${GLEW_LIBRARIES} )

  SET( LINK_FLAGS "${LINK_FLAGS} -OPT:NOREF" )

ELSE(WIN32 OR WIN64 OR CYGWIN OR MINGW)

  IF(APPLE)
  # Media libraries for Apple
  ADD_DEFINITIONS( -DOSX -DMR_SSE -D__MACOSX_CORE__ )
  ADD_COMPILE_OPTIONS( -msse -msse2 -Wno-format-security -Wno-deprecated-declarations -I/usr/local/include/ImageMagick-7/MagickCore )


  LINK_DIRECTORIES( /usr/local/lib )
  set( OS_LIBRARIES ${OS_LIBRARIES} ass ${Xpm} ${png} ${jpeg} ${Zlib} pthread fontconfig GLEW lzma mp3lame theoraenc theoradec theora vorbisenc vorbis x264 vpx )


  IF( RT_AUDIO_ENGINE )
    ADD_DEFINITIONS( -DRT_AUDIO_ENGINE )
  ENDIF()

   IF( CMAKE_BUILD_TYPE STREQUAL "Debug" OR
       CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" )
     #       ADD_DEFINITIONS(  -I/usr/local/sanitize/include -I/usr/local/sanitize/include/OpenEXR )
     # ADD_DEFINITIONS( -DTHREAD_SANITIZER )
     # ADD_COMPILE_OPTIONS( -fsanitize=thread )
     # SET(OS_LIBRARIES -fsanitize=thread ${OS_LIBRARIES} )

     ADD_COMPILE_OPTIONS( -fsanitize=address )
     SET(LIBRARIES -fsanitize=address ${LIBRARIES} )
   ENDIF()

  ELSE()
    ADD_DEFINITIONS( -DMR_SSE )
    ADD_DEFINITIONS( -D_GLIBCXX_USE_CXX11_ABI=1 )
    ADD_COMPILE_OPTIONS( -Wall -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable -Wno-unused-function -Wno-switch -Wno-deprecated-declarations)


  # Media libraries for Linux
  # FIND_PACKAGE( ALSA        REQUIRED )
  FIND_LIBRARY( Xpm Xpm
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )

  FIND_LIBRARY( png png
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )

  FIND_LIBRARY( Zlib z
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )

  FIND_LIBRARY( jpeg jpeg
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )


  ADD_DEFINITIONS( -DLINUX )
  ADD_COMPILE_OPTIONS( -O3 -msse )
  LINK_DIRECTORIES( /usr/local/lib )
  SET(OS_LIBRARIES
    asound ass ${Xpm} ${png} ${jpeg} ${Zlib} dl X11 Xext pthread Xinerama Xfixes Xcursor Xft Xrender Xss m fontconfig dl Xi Xext GLEW lzma mp3lame theoraenc theoradec theora vorbisenc vorbis x264 vpx   ### dvdnav dvdread
    )

   IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
     #       ADD_DEFINITIONS(  -I/usr/local/sanitize/include -I/usr/local/sanitize/include/OpenEXR )
     # ADD_COMPILE_OPTIONS( -g -fsanitize=thread )
     # ADD_DEFINITIONS( -DTHREAD_SANITIZER )
     # SET(OS_LIBRARIES tsan ${OS_LIBRARIES} )

     ADD_COMPILE_OPTIONS( -g -fsanitize=address -rdynamic )
     SET(LIBRARIES asan ${LIBRARIES} )
     SET( $ENV{ASAN_SYMBOLIZER_PATH}=/usr/bin/llvm-symbolizer-3.4 )
   ENDIF( CMAKE_BUILD_TYPE STREQUAL "Debug" )

  ENDIF()
ENDIF(WIN32 OR WIN64 OR CYGWIN OR MINGW)


#
# List directories for -I options.
#
INCLUDE_DIRECTORIES(
  .
  ./core
  ./db
  ./gui
  ./video
  ../../libACESclip/include
  ../../libAMF/src
  "${BlackMagicRAW_INCLUDE_DIR}"
  ${R3DSDK_INCLUDE_DIR}
  ${FFMPEG_INCLUDE_DIR}
  ${LIBINTL_INCLUDE_DIR}
  ${LibRaw_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${FLTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${MAGICK_INCLUDE_DIR}
  ${TINYXML2_INCLUDE_DIR}
  ${CTL_INCLUDE_DIR}
  ${OPENEXR_INCLUDE_DIR}
  ${OPENEXRCTL_INCLUDE_DIR}
  ${OIIO_INCLUDE_DIR}
  ${OCIO_INCLUDE_DIR}
  ${OPENTIMELINEIO_INCLUDE_DIR}
  ${SampleICC_INCLUDE_DIR}
  ${TCLAP_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  )

if( PORTAUDIO_FOUND )
  INCLUDE_DIRECTORIES( ${PORTAUDIO_INCLUDE_DIR} )
endif()

FLTK_WRAP_UI( mrViewer
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



# For windows, this is a non-MFC application
SET( CMAKE_MFC_FLAG 0 )

SET( SOURCES

  ${mrViewer_FLTK_UI_SRCS}

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

if (APPLE)
   set( SOURCES ${SOURCES} gui/mrvMainWindow.mm )
endif()

#
# Audio Engines
#
SET( SOURCES audio/mrvPortAudioEngine.cpp audio/mrvWaveEngine.cpp audio/mrvALSAEngine.cpp ${SOURCES} )


SET( _absPotFile "${CMAKE_CURRENT_SOURCE_DIR}/po/messages.pot" )


ADD_CUSTOM_COMMAND( OUTPUT "${_absPotFile}"
COMMAND xgettext
ARGS --package-name=mrViewer --package-version="$VERSION" --copyright-holder="Film Aura, LLC" --msgid-bugs-address=ggarra13@gmail.com -d mrViewer -s -c++ -k_ ${SOURCES} -o po/messages.pot
DEPENDS mrViewer
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
  SET( SOURCES ${PROJECT_IDL_FILES} ${GENERATED_FILES_IDL} ${SOURCES} )
  SET( SOURCES gui/resource.rc ${SOURCES} )
ENDIF()

IF( NOT WIN32 )

  FIND_LIBRARY( Boost_locale_LIBRARY
  NAMES boost_locale boost_locale-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  FIND_LIBRARY( Boost_system_LIBRARY
  NAMES boost_system boost_system-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )


  FIND_LIBRARY( Boost_filesystem_LIBRARY
  NAMES boost_filesystem boost_filesystem-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  FIND_LIBRARY( Boost_regex_LIBRARY
  NAMES boost_regex boost_regex-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  FIND_LIBRARY( Boost_thread_LIBRARY
  NAMES boost_thread boost_thread-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  SET( BOOST_LIBRARIES
  ${Boost_locale_LIBRARY}
  ${Boost_regex_LIBRARY}
  ${Boost_system_LIBRARY}
  ${Boost_filesystem_LIBRARY}
  ${Boost_thread_LIBRARY}
  )

ELSE(NOT WIN32)
  #
  # Under windows, boost .h files select the appropriate boost static library
  # automatically to handle the correct linking.
  #
  LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )
ENDIF(NOT WIN32)


SET( LIBRARIES
  ${LIBRARIES}
  ${BlackMagicRAW_LIBRARIES}
  ${R3DSDK_LIBRARIES}
  ${LIBINTL_LIBRARIES}
  ${FFMPEG_LIBRARIES}
  ${MAGICK++_LIBRARIES}
  ${OPENEXR_LIBRARIES}
  ${FLTK_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${CTL_LIBRARIES}
  ${OpenEXRCTL_LIBRARIES}
  ${OCIO_LIBRARIES}
  ${OIIO_LIBRARIES}
  ${SampleICC_LIBRARIES}
  ${LibRaw_LIBRARIES}
  ${OPENTIMELINEIO_LIBRARIES}
  ${TINYXML2_LIBRARIES}
  ${OS_LIBRARIES}
  )

if( PORTAUDIO_FOUND )
    set( LIBRARIES ${PORTAUDIO_LIBRARIES} ${LIBRARIES} )
endif()

#
# RELEASE VERSION
#
if(NOT WIN32)
  if(COMMAND cmake_policy)
       cmake_policy(SET CMP0003 NEW)
  endif()
endif()


ADD_EXECUTABLE( mrViewer WIN32 ${SOURCES} )
TARGET_LINK_LIBRARIES( mrViewer ${LIBRARIES} ACESclip AMF )

#
# CMake 3.15 new variable for windows runtime library
#
SET_TARGET_PROPERTIES( mrViewer
  PROPERTIES
  LINK_FLAGS "${LINK_FLAGS}"
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
  )


######################################################
#
# Installation
#
######################################################

IF(WIN32)
  SET(EXE_DIR D:/code/lib/vc14_Windows_32/bin/ )
ENDIF(WIN32)

IF(WIN64) # defined in FindBuilDir
  SET(EXE_DIR D:/code/lib/vc14_Windows_64/bin/ )
ENDIF(WIN64)

IF(APPLE)
  SET(EXE_DIR /usr/local/bin )
ELSE()
  SET(EXE_DIR /usr/local/bin )
ENDIF()

SET( SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

IF(UNIX AND NOT APPLE)

  set( LINK_FLAGS ${LINK_FLAGS})

  IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    EXECUTE_PROCESS( COMMAND "dos2unix" "mrViewer-dbg.sh" "../utils/libs.rb" WORKING_DIRECTORY "${SOURCE_DIR}/bin" )
    FILE( COPY ${SOURCE_DIR}/bin/mrViewer.sh
    DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
  ELSE()
    EXECUTE_PROCESS( COMMAND "dos2unix" "mrViewer.sh" "../utils/libs.rb" WORKING_DIRECTORY "${SOURCE_DIR}/bin" )
    FILE( COPY ${SOURCE_DIR}/bin/mrViewer.sh
    DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
  ENDIF()
ENDIF(UNIX AND NOT APPLE)





if( UNIX )
  set( FFMPEG_ROOT ${EXE_DIR} )
else()
  set( FFMPEG_ROOT $ENV{FFMPEG_ROOT}/bin )
endif()


FILE(GLOB scriptfiles ${SOURCE_DIR}/bin/*.rb ${SOURCE_DIR}/bin/*.py )
FILE(GLOB ffmpegfiles ${FFMPEG_ROOT}/ff*.exe ${FFMPEG_ROOT}/av*.dll ${FFMPEG_ROOT}/sw*.dll ${FFMPEG_ROOT}/pos*.dll)
FILE(COPY ${ffmpegfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB exrfiles ${EXE_DIR}/exr* )
FILE(COPY ${exrfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB ociofiles ${EXE_DIR}/ocio* ${EXE_DIR}/OpenColor*.dll )
FILE(COPY ${ociodlls} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB oiiofiles ${EXE_DIR}/oiio* ${EXE_DIR}/OpenImage*.dll )
FILE(COPY ${oiiofiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/../otio ${CMAKE_BINARY_DIR}/../otio/adapters )
FILE(GLOB otiofiles "${SOURCE_DIR}/otio/adapters/*" )
FILE(COPY ${otiofiles} DESTINATION ${CMAKE_BINARY_DIR}/../otio/adapters )
FILE(GLOB otiofiles "${SOURCE_DIR}/otio/*" )
FILE(COPY ${otiofiles} DESTINATION ${CMAKE_BINARY_DIR}/../otio/ )
IF (WIN32 OR WIN64)
  FILE(GLOB dllfiles ${EXE_DIR}/*.dll ${CMAKE_BINARY_DIR}/../lib/ )
  LIST( FILTER dllfiles EXCLUDE REGEX .*fltk.*.dll )
  MESSAGE( STATUS "Copying DLLs..." )
  FILE(COPY ${dllfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
ELSE(WIN32 OR WIN64)
#  MESSAGE( "Running utils/libs.rb in ${CMAKE_CURRENT_SOURCE_DIR}/../../" )
#  EXECUTE_PROCESS( COMMAND utils/libs.rb
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../ )
ENDIF(WIN32 OR WIN64)


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

set( LANGUAGES "ar" "cs" "de" "en" "es" "fr" "gr" "it" "ja"
	       "ko" "nl" "pl" "pt" "ro" "ru" "sv" "tr" "zh" )

SET( output_files "${_absPotFile}" )

FOREACH( lang ${LANGUAGES} )

  SET( _moDir "${SOURCE_DIR}/share/locale/${lang}/LC_MESSAGES/" )
  SET( _moFile "${_moDir}/mrViewer${SHORTVERSION}.mo" )

  SET( output_files ${output_files} ${_moFile} )

  FILE( REMOVE_RECURSE "${_moDir}" ) # Remove dir to remove old .mo files
  FILE( MAKE_DIRECTORY "${_moDir}" ) # Recreate dir to place new .mo file

 SET( _absFile "${CMAKE_CURRENT_SOURCE_DIR}/po/${lang}.po" )

  ADD_CUSTOM_COMMAND( OUTPUT "${_moFile}"
  COMMAND msgmerge --quiet --update --backup=none
  "${_absFile}" "${_absPotFile}"
  COMMAND msgfmt -v "${_absFile}" -o "${_moFile}"
  DEPENDS ${_absFile} ${_absPotFile}
  )

ENDFOREACH( lang )

ADD_CUSTOM_TARGET(
  "translations" ALL
  DEPENDS ${output_files} ${PROJECT_NAME}
  )

IF(WIN32)

  # SET the type of package to build.

  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one SET of four (4) backlasshes.
  SET(CPACK_NSIS_MODIFY_PATH ON)

  #  SET(CPACK_GENERATOR NSIS )

  SET(CPACK_GENERATOR ZIP NSIS )

  #  SET(CMAKE_INSTALL_DEBUG_LIBRARIES true)


  SET(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/icons/viewer.ico)
  SET(CPACK_NSIS_MUI_UNICON ${PROJECT_SOURCE_DIR}/icons/viewer.ico)

  if (CMAKE_BUILD_ARCH EQUAL 32)

  SET( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES" )

  elseif (CMAKE_BUILD_ARCH EQUAL 64)

  SET( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" )

  endif (CMAKE_BUILD_ARCH EQUAL 32)

  SET( CPACK_NSIS_DISPLAY_NAME "mrViewer-${CMAKE_BUILD_ARCH} v${SHORTVERSION}" )
  SET( CPACK_NSIS_PACKAGE_NAME "mrViewer" )
  SET( CPACK_PACKAGE_VENDOR "FilmAura" )
  SET( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrViewerPackageName})
  SET( CPACK_PACKAGE_EXECUTABLES "mrViewer" "mrViewer-${CMAKE_BUILD_ARCH}-v${SHORTVERSION}" )
  SET( CPACK_CREATE_DESKTOP_LINKS "mrViewer" "mrViewer-${CMAKE_BUILD_ARCH}-v${SHORTVERSION}" )

  SET( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

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

  # SET the type of package to build.
  INSTALL( DIRECTORY ${SOURCE_DIR}/bin/ DESTINATION bin )

  SET(MRV_DESKTOP_DIR     /usr/share/applications/)
  SET(MRV_PIXMAPS_DIR     /usr/share/icons/hicolor/32x32/apps/)



  CONFIGURE_FILE( ${SOURCE_DIR}/etc/Linux/mrViewer.desktop.in
      "${SOURCE_DIR}/etc/mrViewer-v${SHORTVERSION}.desktop" )


  INSTALL(FILES "${SOURCE_DIR}/etc/mrViewer-v${SHORTVERSION}.desktop"
    DESTINATION ${MRV_DESKTOP_DIR})
  INSTALL(FILES ${SOURCE_DIR}/etc/mrViewer.png
    DESTINATION ${MRV_PIXMAPS_DIR})

  # try to set up the menu sys

  SET(CPACK_GENERATOR DEB RPM TGZ )
  # SET(CPACK_GENERATOR TGZ )

  # SET the installation directory.

  SET(CPACK_SET_DESTDIR true) # Needed
  SET(CPACK_INSTALL_PREFIX /usr/local/${mrViewerShortName})

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

    # SET RPM options.

  SET(CPACK_RPM_PACKAGE_NAME ${PROJECT_NAME})
  SET(CPACK_RPM_PACKAGE_RELOCATABLE true)
  set(CPACK_RPM_PACKAGE_AUTOREQ false)
  set(CPACK_RPM_PACKAGE_AUTOPROV true)
  SET(CPACK_RPM_COMPRESSION_TYPE gzip )


  # SET Debian options.

  set(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})

  SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  ${PROJECT_BINARY_DIR}/etc/Linux/postinst
  ${PROJECT_BINARY_DIR}/etc/Linux/postrm)

  SET(
  CPACK_RPM_POST_INSTALL_SCRIPT_FILE
  ${PROJECT_BINARY_DIR}/etc/Linux/install.sh)
  SET(
  CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
  ${PROJECT_BINARY_DIR}/etc/Linux/uninstall.sh)


ENDIF(WIN32)

  SET(CPACK_PACKAGE_VENDOR "Film Aura, LLC")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${VersionMajor})
  SET(CPACK_PACKAGE_VERSION_MINOR ${VersionMinor})
  SET(CPACK_PACKAGE_VERSION_PATCH ${VersionPatch})
  SET(CPACK_PACKAGE_VERSION "${SHORTVERSION}" )
  SET(CPACK_PACKAGE_FILE_NAME "${mrViewerPackageName}" )
  SET(CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")
# SET(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/README.txt)
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "mrViewer provides professional flipbook, audio and video playback.")


#SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${mrViewerPackageName} )
set(CPACK_PACKAGE_EXECUTABLES mrViewer mrViewer)
SET(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/../../docs/LICENSE.txt)

#
# Print out some status to verify configuration
#
MESSAGE( STATUS
  "-------------------------------------------------------------" )
MESSAGE( STATUS "Summary for mrViewer:" )
MESSAGE( STATUS
  "-------------------------------------------------------------" )

MESSAGE( STATUS "FLTK:          ${FLTK_FOUND} ${FLTK_VERSION}" )
MESSAGE( STATUS "OpenEXR:       ${OPENEXR_FOUND} ${OPENEXR_VERSION}" )
MESSAGE( STATUS "ImageMagick:   ${MAGICK_FOUND} ${MAGICK_VERSION}" )
MESSAGE( STATUS "OIIO:          ${OIIO_FOUND} ${OIIO_VERSION}" )
MESSAGE( STATUS "ffmpeg:        ${FFMPEG_FOUND} ${FFMPEG_VERSION}" )
MESSAGE( STATUS "Boost:         ${Boost_FOUND} ${Boost_VERSION}" )
MESSAGE( STATUS "TCLAP:         ${TCLAP_FOUND} ${TCLAP_VERSION}" )
MESSAGE( STATUS "R3DSDK:        ${R3DSDK_FOUND} ${R3SDK_VERSION}" )
MESSAGE( STATUS "libintl:       ${LIBINTL_FOUND} ${LIBINTL_VERSION}" )
MESSAGE( STATUS "TinyXML2:      ${TINYXML2_FOUND} ${TINYXML2_VERSION}" )
MESSAGE( STATUS "PortAudio:     ${PORTAUDIO_FOUND} ${PORTAUDIO_VERSION}" )

IF(OPENTIMELINEIO_FOUND)
  MESSAGE( STATUS "OTIO INCLUDE DIR=${OPENTIMELINEIO_INCLUDE_DIR}" )
  MESSAGE( STATUS "OTIO LIBRARIES=${OPENTIMELINEIO_LIBRARIES}" )
ENDIF()

IF(R3DSDK_FOUND)
  MESSAGE( STATUS "R3DSDK INCLUDE DIR=${R3DSDK_INCLUDE_DIR}" )
  MESSAGE( STATUS "R3DSDK LIBRARIES=${R3DSDK_LIBRARIES}" )
ENDIF()

IF(FLTK_FOUND)
  MESSAGE( STATUS "FLTK INCLUDE DIRS=${FLTK_INCLUDE_DIRS}" )
  MESSAGE( STATUS "FLTK LIBRARIES=${FLTK_LIBRARIES}" )
ENDIF(FLTK_FOUND)

IF(MAGICK_FOUND)
  MESSAGE( STATUS "ImageMagick   include: ${MAGICK_INCLUDE_DIR}" )
  MESSAGE( STATUS "ImageMagick   library: ${MAGICK_LIBRARY_DIR}" )
ENDIF(MAGICK_FOUND)


IF(OPENEXR_FOUND)
  MESSAGE( STATUS "OpenEXR include:       ${OPENEXR_INCLUDE_DIR}" )
  MESSAGE( STATUS "OpenEXR library:       ${OPENEXR_LIBRARY_DIR}" )
  MESSAGE( STATUS "OpenEXR libs:          ${OPENEXR_LIBRARIES}" )
ENDIF(OPENEXR_FOUND)

IF(FLTK_FOUND)
  MESSAGE( STATUS "FLTK include:         ${FLTK_INCLUDE_DIR}" )
  MESSAGE( STATUS "FLTK library:         ${FLTK_LIBRARY_DIR}" )
  MESSAGE( STATUS "FLTK libs:            ${FLTK_LIBRARIES}" )
ENDIF(FLTK_FOUND)

IF(Boost_FOUND)
  MESSAGE( STATUS "Boost include:         ${Boost_INCLUDE_DIR}" )
  MESSAGE( STATUS "Boost library:         ${BOOST_LIBRARIES}" )
ENDIF(Boost_FOUND)

IF(FFMPEG_FOUND)
  MESSAGE( STATUS "ffmpeg include:        ${FFMPEG_INCLUDE_DIR}" )
  MESSAGE( STATUS "ffmpeg BSD  codecs:    ${FFMPEG_BSD_LIBRARIES}" )
  MESSAGE( STATUS "ffmpeg LGPL codecs:    ${FFMPEG_LGPL_LIBRARIES}" )
  MESSAGE( STATUS "ffmpeg libraries:      ${FFMPEG_LIBRARIES}" )
ENDIF(FFMPEG_FOUND)

IF(TCLAP_FOUND)
  MESSAGE( STATUS "TCLAP include:    ${TCLAP_INCLUDE_DIR}" )
ENDIF(TCLAP_FOUND)

IF(LibRaw_FOUND)
  MESSAGE( STATUS "LibRaw include:     ${LibRaw_INCLUDE_DIR}" )
  MESSAGE( STATUS "LibRaw library dir: ${LibRaw_LIBRARY_DIR}" )
  MESSAGE( STATUS "LibRaw libraries:   ${LibRaw_LIBRARIES}" )
ENDIF(LibRaw_FOUND)

IF(LIBINTL_FOUND)
  MESSAGE( STATUS "LibIntl include:     ${LIBINTL_INCLUDE_DIR}" )
  MESSAGE( STATUS "LibIntl library dir: ${LIBINTL_LIBRARY_DIR}" )
  MESSAGE( STATUS "LibIntl libraries:   ${LIBINTL_LIBRARIES}" )
ENDIF(LIBINTL_FOUND)

IF(TINYXML2_FOUND)
  MESSAGE( STATUS "TinyXML2 include:    ${TINYXML2_INCLUDE_DIR}" )
  MESSAGE( STATUS "TinyXML2 library:    ${TINYXML2_LIBRARY_DIR}" )
  MESSAGE( STATUS "TinyXML2 libraries:  ${TINYXML2_LIBRARIES}" )
ENDIF(TINYXML2_FOUND)

MESSAGE( STATUS "mrViewer version: " ${SHORTVERSION} )

SET( CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake" )
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
