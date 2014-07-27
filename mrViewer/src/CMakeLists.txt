


#
# These are the libraries we will depend on
#

# For window management
FIND_PACKAGE( BuildDir    REQUIRED )    # for 32/64 bits handling
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
# FIND_PACKAGE( RLM         REQUIRED )    # for licensing
FIND_PACKAGE( GLU         REQUIRED )    # for gluErrorString
FIND_PACKAGE( PostgreSQL  REQUIRED )    # for postgresql database
FIND_PACKAGE( SampleICC   REQUIRED )    # for ICC loading/processing
# FIND_PACKAGE( AO )    # for ao sound lib

# ADD_DEFINITIONS( -DUSE_GETTEXT -DMR_SSE -D__STDC_CONSTANT_MACROS -D_CRT_SECURE_NO_WARNINGS  )
ADD_DEFINITIONS( -DMR_SSE -D__STDC_CONSTANT_MACROS -D_CRT_SECURE_NO_WARNINGS )


IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
  ADD_DEFINITIONS( -DWIN32 -D_WIN32_WINNT=0x0501 ) 
  SET(OS_LIBRARIES
    Winmm ws2_32 ${GLEW_LIBRARIES}
    )
  SET( LINK_FLAGS "${LINK_FLAGS} -OPT:NOREF -NODEFAULTLIB:LIBC -NODEFAULTLIB:libcpmtd -NODEFAULTLIB:LIBCPMT -NODEFAULTLIB:LIBCMT -NODEFAULTLIB:LIBCMTD" )
ELSE(WIN32 OR WIN64 OR CYGWIN OR MINGW)
  IF(APPLE)
    # Media libraries for Apple
    ADD_DEFINITIONS( -DOSX )
    SET(OS_LIBRARIES
      )
  ELSE(APPLE)


    # Media libraries for Linux
    # FIND_PACKAGE( ALSA        REQUIRED )
    FIND_LIBRARY( Xpm Xpm
      PATHS
      /usr/lib/x86_64-linux-gnu/
      /usr/local/lib
      /usr/lib
      )
    ADD_DEFINITIONS( -Wconversion -DUSE_GETTEXT -DLINUX -msse ) 
    SET(OS_LIBRARIES
      asound
      ${Xpm} png X11 Xi Xft Xinerama pthread m Xext stdc++ GLEW
      )

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
  ../../mrClasses/mrClasses
#  /usr/include/cairo
  ${OPENGL_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${FLTK2_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${MAGICK++_INCLUDE_DIR}
  ${OpenEXRCTL_INCLUDE_DIR}
  ${CTL_INCLUDE_DIR}
  ${OPENEXR_INCLUDE_DIR}
  ${FFMPEG_INCLUDE_DIR} 
  ${POSTGRESQL_INCLUDE_DIR}
  ${TCLAP_INCLUDE_DIR}
  ${RLM_INCLUDE_DIR}
  ${GLU_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${SampleICC_INCLUDE_DIR}
  )


FLTK_WRAP_UI( mrViewer  
  gui/aviSave.fl
  gui/EXROptionsUI.fl
  gui/mrvEDLWindowUI.fl
  gui/mrvIccProfileUI.fl
  gui/mrvColorAreaUI.fl  
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
  core/pxrzImage.cpp
  core/shmapImage.cpp
  core/slateImage.cpp
  core/smpteImage.cpp
  core/stubImage.cpp
  core/wandImage.cpp
  # core/magickImage.cpp

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
  # core/mrvScale.cpp
  core/mrvString.cpp
  core/mrvTimer.cpp
  core/mrvBarrier.cpp
  # core/generic/mrEvent.cpp

  core/mrvPC1.cpp
  core/Sequence.cpp
  core/mrStackTrace.cpp
  core/mrvCPU.cpp
  core/mrvColorProfile.cpp
  core/mrvImageOpts.cpp

  db/mrvDatabase.cpp
  db/mrvPostgreSQL.cpp
  #   #db/mrvMySQL.cpp # unneeded

  gui/FLU/Flu_Combo_Box.cpp      # needed?
  gui/FLU/Flu_Combo_Tree.cpp     # needed?
  gui/FLU/flu_file_chooser_pixmaps.cpp  
  gui/FLU/Flu_Wrap_Group.cpp 
  gui/FLU/Flu_File_Chooser.cpp  
  gui/FLU/flu_pixmaps.cpp               
  gui/FLU/GridGroup.cpp
  #   #   FLU/Flu_Combo_List.cpp # unneeded

  gui/mrvHotkey.cpp
  gui/mrvColorOps.cpp
  gui/mrvMedia.cpp
  gui/mrvBrowser.cpp
  gui/mrvCTLBrowser.cpp
  gui/mrvCollapsableGroup.cpp
  gui/mrvColorInfo.cpp
  gui/mrvDatabaseBrowser.cpp
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
  gui/mrvPopupMenu.cpp
  gui/mrvPreferences.cpp
  gui/mrvPreferencesBrowser.cpp
  gui/mrvReel.cpp
  gui/mrvTimecode.cpp
  gui/mrvTimeline.cpp
  gui/mrvVectorscope.cpp
  gui/mrvVersion.cpp

  video/mrvDrawEngine.cpp
  video/mrvGLEngine.cpp
  video/mrvGLShader.cpp
  video/mrvGLQuad.cpp
  video/mrvGLLut3d.cpp
  video/mrvGLShape.cpp

  standalone/mrvRoot.cpp
  standalone/mrvCommandLine.cpp
  standalone/main.cpp
  )




IF(NOT WIN32 OR WIN64 OR CYGWIN OR MINGW)

  EXECUTE_PROCESS( COMMAND xgettext -d mrViewer -s -c++ -k_ ${SOURCES} -o po/messages.pot 
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE ok_xgettext
    ERROR_VARIABLE  error_xgettext)

  MESSAGE( STATUS ${ok_xgettext} )
  MESSAGE( STATUS ${error_xgettext} )


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

ELSE(NOT WIN32 OR WIN64 OR CYGWIN OR MINGW)
  #
  # Under windows, boost .h files select the appropriate boost static library
  # automatically to handle the correct linking.
  #
  FIND_LIBRARY( libintl_LIBRARY
  		NAMES libintl-8 libintl.lib libintl
		PATHS $ENV{LIBINTL_ROOT} )


  SET( BOOST_LIBRARIES
       	${libintl_LIBRARY} )


  LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )
ENDIF(NOT WIN32 OR WIN64 OR CYGWIN OR MINGW)

IF( AO_FOUND )
  ADD_DEFINITIONS( -DHAVE_LIB_AO )
  SET( SOURCES audio/mrvAOEngine.cpp ${SOURCES} )
  SET( OS_LIBRARIES ${AO_LIBRARIES} ${OS_LIBRARIES} )
ELSE( AO_FOUND )

  IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
    SET( SOURCES audio/mrvWaveEngine.cpp ${SOURCES} )
    SET( SOURCES gui/resource.rc ${SOURCES} )
  ELSE( WIN32 OR WIN64 OR CYGWIN OR MINGW )
    SET( SOURCES audio/mrvALSAEngine.cpp ${SOURCES} )
  ENDIF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
ENDIF( AO_FOUND )

MESSAGE( "OPENGL LIBS=${OPENGL_LIBRARIES}" )


SET( LIBRARIES 
  ${POSTGRESQL_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${MAGICK++_LIBRARIES}
  ${OpenEXRCTL_LIBRARIES}
  ${CTL_LIBRARIES}
  ${OPENEXR_LIBRARIES}
  ${OS_LIBRARIES}
  ${FLTK2_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${SampleICC_LIBRARIES}
  ${FFMPEG_LIBRARIES}
  )


#
# RELEASE VERSION
#
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)


ADD_EXECUTABLE( mrViewer WIN32 ${SOURCES} )
TARGET_LINK_LIBRARIES( mrViewer ${LIBRARIES} )

SET_TARGET_PROPERTIES( mrViewer
  PROPERTIES
  LINK_FLAGS "${LINK_FLAGS}"
  )

#
# DEMO VERSION
#

# IF( CMAKE_BUILD_TYPE STREQUAL "Release" )
#   ADD_EXECUTABLE( mrViewer-demo WIN32 MACOSX_BUNDLE ${SOURCES} )
#   TARGET_LINK_LIBRARIES( mrViewer-demo ${LIBRARIES} )
#   SET_TARGET_PROPERTIES( mrViewer-demo
#     PROPERTIES
#     COMPILE_FLAGS "-DFILMAURA_DEMO" 
#     LINK_FLAGS "${LINK_FLAGS}"
#     )
# ENDIF( CMAKE_BUILD_TYPE STREQUAL "Release" )



######################################################
#
# Installation
#
######################################################
INSTALL( TARGETS mrViewer
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)


  SET( OUTDIR 
    "${LIBRARY_OUTPUT_PATH}/ImageMagick-${MAGICK_VERSION}/modules-Q32/coders" )
  FILE( MAKE_DIRECTORY "${OUTDIR}" )


ELSE(WIN32 OR WIN64 OR CYGWIN OR MINGW)



  SET( OUTDIR
    "${LIBRARY_OUTPUT_PATH}/ImageMagick-${MAGICK_VERSION}/modules-Q32/coders" )
  FILE( MAKE_DIRECTORY ${OUTDIR} )

ENDIF(WIN32 OR WIN64 OR CYGWIN OR MINGW)



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
MESSAGE( STATUS "ImageMagick++: ${MAGICK++_FOUND}" )
MESSAGE( STATUS "ffmpeg:        ${FFMPEG_FOUND} ${FFMPEG_VERSION}" )
MESSAGE( STATUS "Boost:         ${Boost_FOUND} ${Boost_VERSION}" )
MESSAGE( STATUS "PostgreSQL:    ${POSTGRESQL_FOUND} ${POSTGRESQL_VERSION}" )
MESSAGE( STATUS "TCLAP:         ${TCLAP_FOUND} ${TCLAP_VERSION}" )

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

IF(POSTGRESQL_FOUND)
  MESSAGE( STATUS "PostgreSQL include:    ${POSTGRESQL_INCLUDE_DIR}" )
  MESSAGE( STATUS "PostgreSQL library:    ${POSTGRESQL_LIBRARY_DIR}" )
ENDIF(POSTGRESQL_FOUND)

IF(TCLAP_FOUND)
  MESSAGE( STATUS "TCLAP include:    ${TCLAP_INCLUDE_DIR}" )
ENDIF(TCLAP_FOUND)

#IF(RLM_FOUND)
#  MESSAGE( STATUS "RLM include:    ${RLM_INCLUDE_DIR}" )
#  MESSAGE( STATUS "RLM libraries:  ${RLM_LIBRARIES}" )
#ENDIF(RLM_FOUND)
