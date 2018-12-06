
cmake_minimum_required( VERSION 3.1 FATAL_ERROR )

PROJECT( mrViewer )


IF( NOT CMAKE_MODULE_PATH )
  SET( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../modules )
ENDIF( NOT CMAKE_MODULE_PATH )


CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/gui/mrvVersion.cpp.in
  "${CMAKE_CURRENT_SOURCE_DIR}/gui/mrvVersion.cpp" @ONLY )

#
# These are the libraries we will depend on
#

# For window management
FIND_PACKAGE( BuildDir    REQUIRED )    # for 32/64 bits handling (WIN64)
FIND_PACKAGE( OpenGL      REQUIRED )    # for drawing images/shapes
FIND_PACKAGE( GLUT        REQUIRED )    # for geometric fonts
FIND_PACKAGE( Boost       REQUIRED )    # for file system support
FIND_PACKAGE( FLTK2       REQUIRED )    # for window management
FIND_PACKAGE( ImageMagick REQUIRED )    # for image formats
FIND_PACKAGE( OpenEXR     REQUIRED )    # for EXR image loading
FIND_PACKAGE( CTL         REQUIRED )    # for CTL color language
FIND_PACKAGE( OpenEXRCTL  REQUIRED )    # for OpenEXRCTL color language add-ons
FIND_PACKAGE( FFMPEG      REQUIRED )    # for mpeg, avi, quicktime, wmv
FIND_PACKAGE( Gettext     REQUIRED )    # for translations
FIND_PACKAGE( TCLAP       REQUIRED )    # for command-line parsing
FIND_PACKAGE( GLEW        REQUIRED )    # for opengl features
FIND_PACKAGE( LibIntl     REQUIRED )
FIND_PACKAGE( SampleICC   REQUIRED )    # for ICC loading/processing
FIND_PACKAGE( TinyXML2    REQUIRED )
FIND_PACKAGE( OCIO        REQUIRED )
FIND_PACKAGE( OIIO        REQUIRED )
FIND_PACKAGE( LibRaw      REQUIRED )    # for cr2/dng/nikon formats

ADD_DEFINITIONS( -DMAGICKCORE_QUANTUM_DEPTH=32 -DMAGICKCORE_HDRI_ENABLE=1 -DUSE_GETTEXT -DMR_SSE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D_CRT_SECURE_NO_WARNINGS  )


IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)

     ADD_DEFINITIONS( -DFREEGLUT_STATIC -DFREEGLUT_LIB_PRAGMAS -DWIN32 -D_WIN32_WINNT=0x0501 )
     ADD_COMPILE_OPTIONS( -W1 )

IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    ADD_DEFINITIONS( -DDEBUG -D_DEBUG )
    ADD_COMPILE_OPTIONS( -W1 -MDd )
ENDIF()


  SET(OS_LIBRARIES
    Winmm ws2_32 Psapi ${GLEW_LIBRARIES}
    )


   IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
      SET( LINK_FLAGS "${LINK_FLAGS}  /SUBSYSTEM:WINDOWS,6.00 -OPT:NOREF -NODEFAULTLIB:MSVCRT -NODEFAULTLIB:MSVCP140D -NODEFAULTLIB:MSVCPRTD -NODEFAULTLIB:LIBCPMTD -NODEFAULTLIB:msvcprtd.lib -NODEFAULTLIB:MSVCPRTD -NODEFAULTLIB:LIBCMTD" )
      MESSAGE( STATUS ${LINK_FLAGS} )
   ELSE()
     SET( LINK_FLAGS "${LINK_FLAGS} /SUBSYSTEM:WINDOWS,6.00 -OPT:NOREF -NODEFAULTLIB:LIBC -NODEFAULTLIB:LIBCPMTD -NODEFAULTLIB:LIBCPMT -NODEFAULTLIB:LIBCMT" )
   ENDIF()


ELSE(WIN32 OR WIN64 OR CYGWIN OR MINGW)

  IF(APPLE)
    # Media libraries for Apple
    ADD_DEFINITIONS( -DOSX )
    SET(OS_LIBRARIES
      )
  ELSE(APPLE)

    IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
      SET( CG_ROOT /usr/local/sanitized/ )
    ENDIF()

    # Media libraries for Linux
    # FIND_PACKAGE( ALSA        REQUIRED )
    FIND_LIBRARY( Xpm Xpm
      PATHS
      /usr/local/lib
      /usr/lib/x86_64-linux-gnu/
      /usr/lib
      )
    #
    # ADD_DEFINITIONS( -DLINUX -msse -std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 )
    # ADD_DEFINITIONS( -DLINUX -msse -fdiagnostics-color=always  )
    #ADD_DEFINITIONS( -DLINUX -g -msse -w -fsanitize=thread )
    # ADD_DEFINITIONS( -DLINUX -msse -w --std=c++11)
    # ADD_DEFINITIONS( -DLINUX -msse -w --std=c++0x)
    ADD_DEFINITIONS( -DLINUX -DNDEBUG )
    ADD_COMPILE_OPTIONS( -msse -O3 -w -fdiagnostics-color=always  )
    SET(OS_LIBRARIES
      asound ass ${Xpm} png X11 Xi Xft Xrender Xinerama Xext stdc++ ${GLEW_LIBRARIES} fontconfig lzma mp3lame vpx theoraenc theoradec theora vorbisenc vorbis x264 vpx pthread m
      )
    IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
      ADD_DEFINITIONS( -fsanitize=thread -DTHREAD_SANITIZER )
      SET(OS_LIBRARIES tsan ${OS_LIBRARIES} )
      # ADD_DEFINITIONS( -fsanitize=address )
      # SET(LIBRARIES asan ${LIBRARIES} )
    ENDIF( CMAKE_BUILD_TYPE STREQUAL "Debug" )


  ENDIF(APPLE)
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
  ${FFMPEG_INCLUDE_DIR}
  ${LIBINTL_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${FLTK2_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${MAGICK_INCLUDE_DIR}
  ${OpenEXRCTL_INCLUDE_DIR}
  ${CTL_INCLUDE_DIR}
  ${TINYXML2_INCLUDE_DIR}
  ${OPENEXR_INCLUDE_DIR}
  ${TCLAP_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${SampleICC_INCLUDE_DIR}
  ${OCIO_INCLUDE_DIR}
  ${OIIO_INCLUDE_DIR}
  ${LibRaw_INCLUDE_DIR}
  )



FLTK_WRAP_UI( mrViewer
  gui/aviSave.fl
  gui/mrvStereoUI.fl
  gui/OIIOOptionsUI.fl
  gui/WandOptionsUI.fl
  gui/EXROptionsUI.fl
  gui/mrvAudioOffset.fl
  gui/mrvColorAreaUI.fl
  gui/mrvEDLWindowUI.fl
  gui/mrvIccProfileUI.fl
  gui/mrvSOPNode.fl
  gui/mrvWaveformUI.fl
  gui/mrViewer.fl
  gui/keyboard_ui.fl
  )



# For windows, this is a non-MFC application
SET( CMAKE_MFC_FLAG 0 )

SET( SOURCES

  ${mrViewer_FLTK_UI_SRCS}

  # Image files
  core/CMedia.cpp
  core/CMedia_audio.cpp
  core/mrvFrame.cpp
  core/mrvHome.cpp
  core/guessImage.cpp
  core/aviImage.cpp
  core/aviImage_save.cpp
  core/clonedImage.cpp
  core/ddsImage.cpp
  core/exrImage.cpp
  core/hdrImage.cpp
  core/iffImage.cpp
  core/mapImage.cpp
  core/mrayImage.cpp
  core/mrvColorBarsImage.cpp
  core/mrvColorOps.cpp
  core/oiioImage.cpp
  core/pxrzImage.cpp
  core/rawImage.cpp
  core/shmapImage.cpp
  core/slateImage.cpp
  core/smpteImage.cpp
  core/stubImage.cpp
  core/wandImage.cpp
  core/picImage.cpp

  core/mrSocket.cpp
  core/mrvClient.cpp
  core/mrvServer.cpp
  core/mrvAudioEngine.cpp
  core/mrvColor.cpp
  core/mrvColorSpaces.cpp
  core/ctlToLut.cpp
  core/mrvLicensing.cpp
  core/mrvPacketQueue.cpp
  core/mrvPlayback.cpp
  core/mrvString.cpp
  core/mrvTimer.cpp
  core/mrvACES.cpp

  core/Sequence.cpp
  core/mrStackTrace.cpp
  core/mrvCPU.cpp
  core/mrvColorProfile.cpp
  core/mrvImageOpts.cpp


  gui/FLU/Flu_Combo_Box.cpp      # needed?
  gui/FLU/Flu_Combo_Tree.cpp     # needed?
  gui/FLU/flu_file_chooser_pixmaps.cpp
  gui/FLU/Flu_Wrap_Group.cpp
  gui/FLU/Flu_File_Chooser.cpp
  gui/FLU/flu_pixmaps.cpp
  gui/FLU/GridGroup.cpp

  gui/mrvActionWindow.cpp
  gui/mrvGlWindow3d.cpp
  gui/mrvHotkey.cpp
  gui/mrvColorOps.cpp
  gui/mrvBrowser.cpp
  gui/mrvCTLBrowser.cpp
  gui/mrvCollapsableGroup.cpp
  gui/mrvColorInfo.cpp
  gui/mrvEDLGroup.cpp
  gui/mrvFileRequester.cpp
  gui/mrvFLTKHandler.cpp
  gui/mrvFontsWindowUI.cpp
  gui/mrvHistogram.cpp
  gui/mrvImageView.cpp
  gui/mrvImageBrowser.cpp
  gui/mrvImageInformation.cpp
  gui/mrvIO.cpp
  gui/mrvLogDisplay.cpp
  gui/mrvMediaTrack.cpp
  gui/mrvMainWindow.cpp
  gui/mrvMedia.cpp
  gui/mrvOCIOBrowser.cpp
  gui/mrvPopupMenu.cpp
  gui/mrvPreferences.cpp
  gui/mrvPreferencesBrowser.cpp
  gui/mrvProgressReport.cpp
  gui/mrvReel.cpp
  gui/mrvSave.cpp
  gui/mrvStereoWindow.cpp
  gui/mrvTimecode.cpp
  gui/mrvTimeline.cpp
  gui/mrvVectorscope.cpp
  gui/mrvVersion.cpp
  gui/mrvWaveform.cpp

  video/mrvDrawEngine.cpp
  video/mrvGLEngine.cpp
  video/mrvGLShader.cpp
  video/mrvGLQuad.cpp
  video/mrvGLSphere.cpp
  video/mrvGLCube.cpp
  video/mrvGLLut3d.cpp
  video/mrvGLShape.cpp
  video/mrvCSPUtils.cpp

  standalone/mrvRoot.cpp
  standalone/mrvCommandLine.cpp
  standalone/main.cpp
  )

IF( OIIO_FOUND )
  SET( SOURCES core/oiioImage.cpp ${SOURCES} )
ENDIF()

IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
  SET( SOURCES audio/mrvWaveEngine.cpp ${SOURCES} )
ELSE( WIN32 OR WIN64 OR CYGWIN OR MINGW )
  SET( SOURCES audio/mrvALSAEngine.cpp ${SOURCES} )
ENDIF(WIN32 OR WIN64 OR CYGWIN OR MINGW)

SET( _absPotFile "${CMAKE_CURRENT_SOURCE_DIR}/po/messages.pot" )


ADD_CUSTOM_COMMAND( OUTPUT "${_absPotFile}"
		    COMMAND xgettext
		    ARGS --package-name=mrViewer --package-version="$VERSION" --copyright-holder="Film Aura, LLC" --msgid-bugs-address=ggarra13@gmail.com  -d mrViewer -s -c++ -k_ ${SOURCES} -o po/messages.pot
		    DEPENDS mrViewer
		    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		  )

 SET( SIMPLE_SOURCES ${SOURCES} )
IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
 SET( SOURCES gui/resource.rc ${SOURCES} )
ENDIF( WIN32 OR WIN64 OR CYGWIN OR MINGW )

IF(NOT WIN32)



  FIND_LIBRARY( Boost_locale_LIBRARY
    NAMES boost_locale boost_locale_vc100_mt_1_47
    PATHS ${Boost_LIBRARY_DIRS}
    )

  FIND_LIBRARY( Boost_system_LIBRARY
    NAMES boost_system boost_system_vc100_mt_1_47
    PATHS ${Boost_LIBRARY_DIRS}
    )


  FIND_LIBRARY( Boost_filesystem_LIBRARY
    NAMES boost_filesystem boost_filesystem_vc100_mt_1_47
    PATHS ${Boost_LIBRARY_DIRS}
    )

  FIND_LIBRARY( Boost_thread_LIBRARY
    NAMES boost_thread boost_thread_vc100_mt_1_47
    PATHS ${Boost_LIBRARY_DIRS}
    )

  SET( BOOST_LIBRARIES
    ${Boost_locale_LIBRARY}
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
  ${LIBINTL_LIBRARIES}
  ${FFMPEG_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${MAGICK++_LIBRARIES}
  ${OpenEXRCTL_LIBRARIES}
  ${CTL_LIBRARIES}
  ${OPENEXR_LIBRARIES}
  ${FLTK2_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${SampleICC_LIBRARIES}
  ${TINYXML2_LIBRARIES}
  ${OCIO_LIBRARIES}
  ${LibRaw_LIBRARIES}
  ${OS_LIBRARIES}
  )

IF(OIIO_LIBRARIES)
  SET( LIBRARIES ${LIBRARIES} ${OIIO_LIBRARIES} )
ENDIF(OIIO_LIBRARIES)


#
# RELEASE VERSION
#
if(WIN32)
	# if(COMMAND cmake_policy)
	#          cmake_policy(SET CMP0003 OLD)  # Needed for freeglut
	# endif(COMMAND cmake_policy)
else(WIN32)
	if(COMMAND cmake_policy)
		   cmake_policy(SET CMP0003 NEW)
	endif(COMMAND cmake_policy)
endif(WIN32)


# VP18 switches to c++14, so let's do that by default
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ ISO Standard")
# but switch gnu++14 or other extensions off for portability
set(CMAKE_CXX_EXTENSIONS OFF)

ADD_EXECUTABLE( mrViewer WIN32 ${SOURCES} )
TARGET_LINK_LIBRARIES( mrViewer PRIVATE ${LIBRARIES} ACESclip )


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

FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/3dl2ctl.rb
  ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/cube2ctl.rb
  ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/3dl2ctl.py
  ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/cube2ctl.py
  DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )

IF(UNIX)
  SET(EXE_DIR /usr/local/bin )
  IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/mrViewer-dbg.sh
      DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
  ELSE()
    FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/mrViewer.sh
      DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
  ENDIF()
ENDIF(UNIX)

IF(WIN32 OR WIN64)
	FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/fonts
	      DESTINATION ${CMAKE_BINARY_DIR}/../bin )
ENDIF( WIN32 OR WIN64 )

FILE(GLOB ffmpegfiles ${EXE_DIR}/ff* ${EXE_DIR}/av* ${EXE_DIR}/sw* ${EXE_DIR}/pos*)
FILE(COPY ${ffmpegfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB exrfiles ${EXE_DIR}/exr* )
FILE(COPY ${exrfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB ociofiles ${EXE_DIR}/ocio* )
FILE(COPY ${ociodlls} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB oiiofiles ${EXE_DIR}/oiio* )
FILE(COPY ${oiiofiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
FILE(GLOB ctlfiles ${EXE_DIR}/ctl* )
FILE(COPY ${ctlfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
IF (WIN32 OR WIN64)
  FILE(GLOB dllfiles ${EXE_DIR}/*.dll )
  MESSAGE( STATUS "Copying DLLs..." )
  FILE(COPY ${dllfiles} DESTINATION ${CMAKE_BINARY_DIR}/../bin/ )
ELSE(WIN32 OR WIN64)
  #MESSAGE( "Running utils/libs.rb in ${CMAKE_CURRENT_SOURCE_DIR}/../../" )
  #EXECUTE_PROCESS( COMMAND utils/libs.rb
  #  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../ )
ENDIF(WIN32 OR WIN64)


FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../ctl/
      DESTINATION ${CMAKE_BINARY_DIR}/../ctl )

FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../shaders/
      DESTINATION ${CMAKE_BINARY_DIR}/../shaders )

FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../ocio/
      DESTINATION ${CMAKE_BINARY_DIR}/../ocio )

FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../docs/
      DESTINATION ${CMAKE_BINARY_DIR}/../docs )

INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../ctl/
	 DESTINATION ctl )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../shaders/
	 DESTINATION shaders )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../ocio/
	 DESTINATION ocio )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../docs/
	 DESTINATION docs )
INSTALL( DIRECTORY ${CMAKE_BINARY_DIR}/../bin/ DESTINATION bin )
IF( NOT WIN32 )
    INSTALL( DIRECTORY ${CMAKE_BINARY_DIR}/../lib/ DESTINATION lib )
ENDIF( NOT WIN32 )

SET( LANGUAGES "es" )


SET( output_files "${_absPotFile}" )

FOREACH( lang ${LANGUAGES} )

  SET( _moDir "${CMAKE_BINARY_DIR}/../share/locale/${lang}/LC_MESSAGES/" )

  SET( _moFile "${_moDir}/mrViewer${VERSION}.mo" )

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

# Install for language files
INSTALL( DIRECTORY ${CMAKE_BINARY_DIR}/../share/ DESTINATION share )


IF(WIN32)

  # SET the type of package to build.

  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one SET of four (4) backlasshes.
  SET(CPACK_NSIS_MODIFY_PATH ON)

  SET(CPACK_GENERATOR ZIP NSIS )

  #  SET(CMAKE_INSTALL_DEBUG_LIBRARIES true)


  SET(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/icons/viewer.ico)
  SET(CPACK_NSIS_MUI_UNICON ${PROJECT_SOURCE_DIR}/icons/viewer.ico)

  if (CMAKE_BUILD_ARCH EQUAL 32)

    SET( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES" )

  elseif (CMAKE_BUILD_ARCH EQUAL 64)

    SET( CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" )

  endif (CMAKE_BUILD_ARCH EQUAL 32)

  SET( CPACK_NSIS_DISPLAY_NAME "mrViewer-${CMAKE_BUILD_ARCH} v${VERSION}" )
  SET( CPACK_NSIS_PACKAGE_NAME "${CPACK_NSIS_DISPLAY_NAME}" )
  SET( CPACK_PACKAGE_INSTALL_DIRECTORY ${mrViewerPackageName})
  SET( CPACK_PACKAGE_EXECUTABLES "mrViewer" "mrViewer-${CMAKE_BUILD_ARCH} v${SHORTVERSION}" )
  SET( CPACK_CREATE_DESKTOP_LINKS "mrViewer" "mrViewer-${CMAKE_BUILD_ARCH} v${SHORTVERSION}" )

  SET( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

  INCLUDE( CMakeRegistry.cmake )



ELSEIF (UNIX)

    # SET the type of package to build.
    #INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/ DESTINATION bin )

    SET(MRV_DESKTOP_DIR     /usr/share/applications/)
    SET(MRV_PIXMAPS_DIR     /usr/share/icons/hicolor/32x32/apps/)



    CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/../../etc/Linux/mrViewer.desktop.in
      "${CMAKE_CURRENT_SOURCE_DIR}/../../etc/mrViewer-v${VERSION}.desktop" )

    INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../etc/mrViewer-v${VERSION}.desktop"
      DESTINATION ${MRV_DESKTOP_DIR})


    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../etc/mrViewer.png
	    DESTINATION ${MRV_PIXMAPS_DIR})


    SET(CPACK_GENERATOR DEB TGZ RPM )

    # SET the installation directory.

    SET(CPACK_SET_DESTDIR ON) # Needed
    set(CPACK_PACKAGING_INSTALL_PREFIX "")
    SET(CPACK_INSTALL_PREFIX /usr/local/${mrViewerShortName})

    # SET Debian options.

    # Configure install/uninstall scripts
    CONFIGURE_FILE(
	${PROJECT_SOURCE_DIR}/../../etc/Linux/install.sh.in
	${PROJECT_BINARY_DIR}/etc/Linux/postinst
	@ONLY)
    CONFIGURE_FILE(
	${PROJECT_SOURCE_DIR}/../../etc/Linux/uninstall.sh.in
	${PROJECT_BINARY_DIR}/etc/Linux/postrm
	@ONLY)

    SET(CPACK_DEBIAN_PACKAGE_NAME mrviewer)
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Gonzalo Garramuño <ggarra13@gmail.com>")

    SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
	${PROJECT_BINARY_DIR}/etc/Linux/postinst
	${PROJECT_BINARY_DIR}/etc/Linux/postrm)

      # SET RPM options.

    # Configure install/uninstall scripts

    CONFIGURE_FILE(
	${PROJECT_SOURCE_DIR}/../../etc/Linux/install.sh.in
	${PROJECT_BINARY_DIR}/etc/Linux/install.sh
	@ONLY)
    CONFIGURE_FILE(
	${PROJECT_SOURCE_DIR}/../../etc/Linux/uninstall.sh.in
	${PROJECT_BINARY_DIR}/etc/Linux/uninstall.sh
	@ONLY)


    SET(CPACK_RPM_PACKAGE_NAME mrViewer)
    SET(CPACK_RPM_PACKAGE_RELOCATABLE OFF)
    SET(CPACK_RPM_PACKAGE_RELEASE 2 )
    # SET(CPACK_RPM_PACKAGE_DEBUG 1 )

    SET(
	CPACK_RPM_POST_INSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/install.sh)
    SET(
	CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
	${PROJECT_BINARY_DIR}/etc/Linux/uninstall.sh)

      set(CPACK_RPM_SPEC_MORE_DEFINE "%define ignore \#")
      set(CPACK_RPM_USER_FILELIST "%ignore /" "%ignore /usr" "%ignore /usr/bin" "%ignore /usr/local" "%ignore /usr/share" "%ignore /usr/share/applications" "%ignore /usr/share/doc" "%ignore /usr/share/icons" "%ignore /usr/share/icons/hicolor" "%ignore /usr/share/icons/hicolor/32x32" "%ignore /usr/share/icons/hicolor/32x32/apps")

ENDIF(WIN32)

  SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  SET(CPACK_PACKAGE_VENDOR "Film Aura, LLC")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${VersionMajor})
  SET(CPACK_PACKAGE_VERSION_MINOR ${VersionMinor})
  SET(CPACK_PACKAGE_VERSION_PATCH ${VersionPatch})
  SET(CPACK_PACKAGE_VERSION "${VERSION}" )
  SET(CPACK_PACKAGE_FILE_NAME "${mrViewerPackageName}" )
  SET(CPACK_PACKAGE_CONTACT "ggarra13@gmail.com")
# SET(CPACK_PACKAGE_DESCRIPTION_FILE ${PROJECT_SOURCE_DIR}/README.txt)
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
	  "mrViewer provides professional flipbook, audio and video playback.")


#SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${mrViewerPackageName} )
SET(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/../../docs/LICENSE.txt)


#
# Print out some status to verify configuration
#
MESSAGE( STATUS
  "-------------------------------------------------------------" )
MESSAGE( STATUS "Summary for mrViewer:" )
MESSAGE( STATUS
  "-------------------------------------------------------------" )

MESSAGE( STATUS "FLTK2:         ${FLTK2_FOUND} ${FLTK2_VERSION}" )
MESSAGE( STATUS "OpenEXR:       ${OPENEXR_FOUND} ${OPENEXR_VERSION}" )
MESSAGE( STATUS "ImageMagick:   ${MAGICK_FOUND} ${MAGICK_VERSION}" )
MESSAGE( STATUS "ffmpeg:        ${FFMPEG_FOUND} ${FFMPEG_VERSION}" )
MESSAGE( STATUS "Boost:         ${Boost_FOUND} ${Boost_VERSION}" )
#MESSAGE( STATUS "PostgreSQL:    ${POSTGRESQL_FOUND} ${POSTGRESQL_VERSION}" )
MESSAGE( STATUS "TCLAP:         ${TCLAP_FOUND} ${TCLAP_VERSION}" )
MESSAGE( STATUS "CTL:           ${CTL_FOUND} ${CTL_VERSION}" )
MESSAGE( STATUS "libintl:       ${LIBINTL_FOUND} ${LIBINTL_VERSION}" )
MESSAGE( STATUS "TinyXML2:      ${TINYXML2_FOUND} ${TINYXML2_VERSION}" )

IF(GLUT_FOUND)
  MESSAGE( STATUS "GLUT INCLUDE DIR=${GLUT_INCLUDE_DIR}" )
  MESSAGE( STATUS "GLUT LIBRARIES=${GLUT_LIBRARIES}" )
ENDIF(GLUT_FOUND)

IF(FLTK2_FOUND)
  MESSAGE( STATUS "FLTK2 INCLUDE DIR=${FLTK2_INCLUDE_DIR}" )
  MESSAGE( STATUS "FLTK2 LIBRARIES=${FLTK2_LIBRARIES}" )
ENDIF(FLTK2_FOUND)

IF(MAGICK_FOUND)
  MESSAGE( STATUS "ImageMagick   include: ${MAGICK_INCLUDE_DIR}" )
  MESSAGE( STATUS "ImageMagick   library: ${MAGICK_LIBRARY_DIR}" )
ENDIF(MAGICK_FOUND)

IF(MAGICK++_FOUND)
  MESSAGE( STATUS "ImageMagick++ include: ${MAGICK++_INCLUDE_DIR}" )
  MESSAGE( STATUS "ImageMagick++ library: ${MAGICK++_LIBRARY_DIR}" )
  MESSAGE( STATUS "ImageMagick++ libs:    ${MAGICK++_LIBRARIES}" )
ENDIF(MAGICK++_FOUND)

IF(OPENEXR_FOUND)
  MESSAGE( STATUS "OpenEXR include:       ${OPENEXR_INCLUDE_DIR}" )
  MESSAGE( STATUS "OpenEXR library:       ${OPENEXR_LIBRARY_DIR}" )
  MESSAGE( STATUS "OpenEXR libs:          ${OPENEXR_LIBRARIES}" )
ENDIF(OPENEXR_FOUND)

IF(FLTK2_FOUND)
  MESSAGE( STATUS "FLTK2 include:         ${FLTK2_INCLUDE_DIR}" )
  MESSAGE( STATUS "FLTK2 library:         ${FLTK2_LIBRARY_DIR}" )
  MESSAGE( STATUS "FLTK2 libs:            ${FLTK2_LIBRARIES}" )
ENDIF(FLTK2_FOUND)

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

#IF(POSTGRESQL_FOUND)
#  MESSAGE( STATUS "PostgreSQL include:    ${POSTGRESQL_INCLUDE_DIR}" )
#  MESSAGE( STATUS "PostgreSQL library:    ${POSTGRESQL_LIBRARY_DIR}" )
#ENDIF(POSTGRESQL_FOUND)

IF(TCLAP_FOUND)
  MESSAGE( STATUS "TCLAP include:    ${TCLAP_INCLUDE_DIR}" )
ENDIF(TCLAP_FOUND)

IF(CTL_FOUND)
  MESSAGE( STATUS "CTL include:    ${CTL_INCLUDE_DIR}" )
  MESSAGE( STATUS "CTL library:    ${CTL_LIBRARY_DIR}" )
  MESSAGE( STATUS "CTL libraries:  ${CTL_LIBRARIES}" )
ENDIF(CTL_FOUND)

IF(LIBINTL_FOUND)
  MESSAGE( STATUS "LibIntl include:     ${LIBINTL_INCLUDE_DIR}" )
  MESSAGE( STATUS "LibIntl library dir: ${LIBINTL_LIBRARY_DIR}" )
  MESSAGE( STATUS "LibIntl libraries:   ${LIBINTL_LIBRARIES}" )
ENDIF(LIBINTL_FOUND)

IF(OpenEXRCTL_FOUND)
  MESSAGE( STATUS "OpenEXR CTL include:    ${OpenEXRCTL_INCLUDE_DIR}" )
  MESSAGE( STATUS "OpenEXR CTL library:    ${OpenEXRCTL_LIBRARY_DIR}" )
  MESSAGE( STATUS "OpenEXR CTL libraries:  ${OpenEXRCTL_LIBRARIES}" )
ENDIF(OpenEXRCTL_FOUND)

IF(TINYXML2_FOUND)
  MESSAGE( STATUS "TinyXML2 include:    ${TINYXML2_INCLUDE_DIR}" )
  MESSAGE( STATUS "TinyXML2 library:    ${TINYXML2_LIBRARY_DIR}" )
  MESSAGE( STATUS "TinyXML2 libraries:  ${TINYXML2_LIBRARIES}" )
ENDIF(TINYXML2_FOUND)

IF(LibRaw_FOUND)
  MESSAGE( STATUS "LibRaw include:    ${LibRaw_INCLUDE_DIR}" )
  MESSAGE( STATUS "LibRaw library:    ${LibRaw_LIBRARY_DIR}" )
  MESSAGE( STATUS "LibRaw libraries:  ${LibRaw_LIBRARIES}" )
ENDIF(LibRaw_FOUND)

  MESSAGE( STATUS "OIIO include:    ${OIIO_INCLUDE_DIR}" )
  MESSAGE( STATUS "OIIO library:    ${OIIO_LIBRARY_DIR}" )
  MESSAGE( STATUS "OIIO libraries:  ${OIIO_LIBRARIES}" )

MESSAGE( STATUS "mrViewer version: " ${VERSION} )


# Cmake only adds the default package target if a package config file
# named CPackConfig.cmake exists in the outermost binary build directory.
# By forcing the CPack module to generate the package file under a
# different name, we prevent the "target" path from being generated.
SET( CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/BundleConfig.cmake" )
include(CPack)

IF( TEST_BUNDLE  )
# A dummy target so that we can test post_build commands faster
  MESSAGE( STATUS "NO CPACK RUN" )
  ADD_CUSTOM_TARGET( bundle ALL
			   COMMAND echo here we would run cpack
			   DEPENDS translations
		  )
ELSE()
  ADD_CUSTOM_TARGET( bundle
			   COMMAND "${CMAKE_CPACK_COMMAND}"
			   "-C" "$<CONFIGURATION>"
			   "--config" "${CMAKE_BINARY_DIR}/BundleConfig.cmake"
			   COMMENT "Running CPack. Please wait..."
			   USES_TERMINAL
			   DEPENDS translations )
ENDIF()

IF (WIN32)
   ADD_CUSTOM_COMMAND( TARGET bundle
		      POST_BUILD
		      COMMAND "${CMAKE_COMMAND}"  -D CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} -D VERSION=${VERSION} -D SHORTVERSION=${SHORTVERSION} "-P" "${CMAKE_CURRENT_SOURCE_DIR}/../../chocolatey/mrViewer/CMakeLists.txt"
		      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		      )
ENDIF( WIN32 )
