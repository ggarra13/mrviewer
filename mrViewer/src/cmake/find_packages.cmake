if( NOT CMAKE_MODULE_PATH )
  set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../modules )
endif( NOT CMAKE_MODULE_PATH )

#
# These are the libraries we will depend on
#
set( OpenGL_GL_PREFERENCE LEGACY )
#set( OpenGL_GL_PREFERENCE GLVND )

# For window management
find_package( BuildDir    REQUIRED )    # for 32/64 bits handling (WIN64)
find_package( OpenGL      REQUIRED )    # for drawing images/shapes
find_package( Boost       REQUIRED )    # for file system support
add_definitions( -D BOOST_ALL_DYN_LINK ) # ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} )

if (APPLE)
  set(FLTK_DIR /Users/gga/code/lib/fltk/build CACHE FILEPATH
    "fltk build dir" FORCE )

   set( OSX_FRAMEWORKS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio" )
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OSX_FRAMEWORKS}" )
   set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}
    ${OSX_FRAMEWORKS}")

  # stop OpenGL deprecation warnings on MacOSX > 10.14
  add_definitions(-DGL_SILENCE_DEPRECATION=1)

  # set( FLTK_FLUID_EXECUTABLE /usr/local/bin/fluid.app/Contents/MacOS/fluid )
else()
  set( FLTK_DIR "~/code/lib/fltk-x11/build-linux" CACHE FILEPATH
    "fltk build dir" FORCE )
endif()

find_package( FLTK        REQUIRED NO_MODULE)
set( FLTK_LIBRARIES       fltk fltk_gl fltk_images )

find_package( ImageMagick REQUIRED )    # for image formats
find_package( OpenEXR     REQUIRED )    # for EXR image loading
find_package( CTL  REQUIRED )           # for CTL
find_package( OpenEXRCTL  REQUIRED )    # for CTL <-> EXR
find_package( OCIO        REQUIRED )    # for OCIO color correction
find_package( OIIO        REQUIRED )    # for OIIO image loading
find_package( FFMPEG      REQUIRED )    # for mpeg, avi, quicktime, wmv
find_package( Gettext     REQUIRED )    # for translations
find_package( TCLAP       REQUIRED )    # for command-line parsing
find_package( GLEW        REQUIRED )    # for opengl features
find_package( LibIntl     REQUIRED )
find_package( SampleICC   REQUIRED )    # For ICC reading
find_package( LibRaw      REQUIRED )    # for libraw files
find_package( TinyXML2    REQUIRED )    # for xml reading/writing
find_package( R3DSDK      REQUIRED )    # for R3D format
find_package( BlackMagicRAW  REQUIRED )    # for BRAW format
find_package( OpenTimelineIO REQUIRED )  # for OpenTimelineIO


add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=32 -DMAGICKCORE_HDRI_ENABLE=1 -DUSE_GETTEXT -D_CRT_SECURE_NO_WARNINGS -DFFMPEG_VERSION=${FFMPEG_VERSION} )

if( APPLE )
  set( USE_PORTAUDIO 1 )
endif()

if( USE_PORTAUDIO )
  find_package( PortAudio REQUIRED )
  if( PORTAUDIO_FOUND )
    add_definitions( -DPORTAUDIO )
  endif()
endif()


add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=32 -DMAGICKCORE_HDRI_ENABLE=1 -DUSE_GETTEXT -D_CRT_SECURE_NO_WARNINGS )


if(WIN32 OR WIN64)

# add_definitions( -DMR_SSE -DWIN32  )
  add_definitions( -DMR_SSE -DWIN32 -D_WIN32_WINNT=0x0601 )

  set( OS_LIBRARIES GDIplus Winmm ws2_32 Psapi ${GLEW_LIBRARIES} )

  set( LINK_FLAGS "${LINK_FLAGS} -OPT:NOREF" )

else()

  if(APPLE)
    # Media libraries for Apple
    add_definitions( -DOSX -DMR_SSE -D__MACOSX_CORE__ )
    add_compile_options( -msse -msse2 -Wno-format-security -Wno-deprecated-declarations -I/usr/local/include/ImageMagick-7/MagickCore )


    link_directories( /usr/local/lib )
    set( OS_LIBRARIES ${OS_LIBRARIES} ass ${Xpm} ${png} ${jpeg} ${Zlib} pthread fontconfig GLEW lzma mp3lame theoraenc theoradec theora vorbisenc vorbis x264 vpx )


    if( CMAKE_BUILD_TYPE STREQUAL "Debug" OR
	CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" )

      add_compile_options( -fsanitize=address )
      set(LIBRARIES -fsanitize=address ${LIBRARIES} )
    endif()

  else()
    add_definitions( -DMR_SSE )
    add_definitions( -D_GLIBCXX_USE_CXX11_ABI=1 )
    add_compile_options( -Wall -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable -Wno-unused-function -Wno-switch -Wno-deprecated-declarations)


  # Media libraries for Linux
  # find_package( ALSA        REQUIRED )
  find_library( Xpm Xpm
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )

  find_library( png png
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )

  find_library( Zlib z
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )

  find_library( jpeg jpeg
    PATHS
    /usr/local/lib
    /usr/lib/x86_64-linux-gnu/
    /usr/lib
    )


  add_definitions( -DLINUX )
  add_compile_options( -O3 -msse )
  link_directories( /usr/local/lib )
  set(OS_LIBRARIES
    asound ass ${Xpm} ${png} ${jpeg} ${Zlib} dl X11 Xext pthread Xinerama Xfixes Xcursor Xft Xrender Xss m fontconfig dl Xi Xext GLEW lzma mp3lame theoraenc theoradec theora vorbisenc vorbis x264 vpx   ### dvdnav dvdread
    )

   if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
     add_compile_options( -g -fsanitize=address -rdynamic )
     set(LIBRARIES asan ${LIBRARIES} )
     set( $ENV{ASAN_SYMBOLIZER_PATH}=/usr/bin/llvm-symbolizer-3.4 )
   endif( CMAKE_BUILD_TYPE STREQUAL "Debug" )

  endif()
endif()


#
# List directories for -I options.
#
include_directories(
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


if( NOT WIN32 )

  find_library( Boost_locale_LIBRARY
  NAMES boost_locale boost_locale-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  find_library( Boost_system_LIBRARY
  NAMES boost_system boost_system-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )


  find_library( Boost_filesystem_LIBRARY
  NAMES boost_filesystem boost_filesystem-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  find_library( Boost_regex_LIBRARY
  NAMES boost_regex boost_regex-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  find_library( Boost_thread_LIBRARY
  NAMES boost_thread boost_thread-mt
  PATHS ${Boost_LIBRARY_DIRS}
  )

  set( BOOST_LIBRARIES
  ${Boost_locale_LIBRARY}
  ${Boost_regex_LIBRARY}
  ${Boost_system_LIBRARY}
  ${Boost_filesystem_LIBRARY}
  ${Boost_thread_LIBRARY}
  )

else()
  #
  # Under windows, boost .h files select the appropriate boost static library
  # automatically to handle the correct linking.
  #
  link_directories( ${Boost_LIBRARY_DIRS} )
endif()


set( LIBRARIES
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
  ${GLEW_LIBRARIES}
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
# Print out some status to verify configuration
#
message( STATUS
  "-------------------------------------------------------------" )
message( STATUS "Summary for mrViewer:" )
message( STATUS
  "-------------------------------------------------------------" )

message( STATUS "FLTK:          ${FLTK_FOUND} ${FLTK_VERSION}" )
message( STATUS "OpenEXR:       ${OPENEXR_FOUND} ${OPENEXR_VERSION}" )
message( STATUS "ImageMagick:   ${MAGICK_FOUND} ${MAGICK_VERSION}" )
message( STATUS "OIIO:          ${OIIO_FOUND} ${OIIO_VERSION}" )
message( STATUS "ffmpeg:        ${FFMPEG_FOUND} ${FFMPEG_VERSION}" )
message( STATUS "Boost:         ${Boost_FOUND} ${Boost_VERSION}" )
message( STATUS "TCLAP:         ${TCLAP_FOUND} ${TCLAP_VERSION}" )
message( STATUS "R3DSDK:        ${R3DSDK_FOUND} ${R3SDK_VERSION}" )
message( STATUS "libintl:       ${LIBINTL_FOUND} ${LIBINTL_VERSION}" )
message( STATUS "TinyXML2:      ${TINYXML2_FOUND} ${TINYXML2_VERSION}" )
message( STATUS "PortAudio:     ${PORTAUDIO_FOUND} ${PORTAUDIO_VERSION}" )

if(OPENTIMELINEIO_FOUND)
  message( STATUS "OTIO INCLUDE DIR=${OPENTIMELINEIO_INCLUDE_DIR}" )
  message( STATUS "OTIO LIBRARIES=${OPENTIMELINEIO_LIBRARIES}" )
endif()

if(R3DSDK_FOUND)
  message( STATUS "R3DSDK INCLUDE DIR=${R3DSDK_INCLUDE_DIR}" )
  message( STATUS "R3DSDK LIBRARIES=${R3DSDK_LIBRARIES}" )
endif()

if(MAGICK_FOUND)
  message( STATUS "ImageMagick   include: ${MAGICK_INCLUDE_DIR}" )
  message( STATUS "ImageMagick   library: ${MAGICK_LIBRARY_DIR}" )
endif()


if(OPENEXR_FOUND)
  message( STATUS "OpenEXR include:       ${OPENEXR_INCLUDE_DIR}" )
  message( STATUS "OpenEXR library:       ${OPENEXR_LIBRARY_DIR}" )
  message( STATUS "OpenEXR libs:          ${OPENEXR_LIBRARIES}" )
endif()

if(FLTK_FOUND)
  message( STATUS "FLTK include:         ${FLTK_INCLUDE_DIR}" )
  message( STATUS "FLTK library:         ${FLTK_LIBRARY_DIR}" )
  message( STATUS "FLTK libs:            ${FLTK_LIBRARIES}" )
endif()

if(Boost_FOUND)
  message( STATUS "Boost include:         ${Boost_INCLUDE_DIR}" )
  message( STATUS "Boost library:         ${BOOST_LIBRARIES}" )
endif()

if(FFMPEG_FOUND)
  message( STATUS "ffmpeg include:        ${FFMPEG_INCLUDE_DIR}" )
  message( STATUS "ffmpeg BSD  codecs:    ${FFMPEG_BSD_LIBRARIES}" )
  message( STATUS "ffmpeg LGPL codecs:    ${FFMPEG_LGPL_LIBRARIES}" )
  message( STATUS "ffmpeg libraries:      ${FFMPEG_LIBRARIES}" )
endif()

if(TCLAP_FOUND)
  message( STATUS "TCLAP include:    ${TCLAP_INCLUDE_DIR}" )
endif()

if(LibRaw_FOUND)
  message( STATUS "LibRaw include:     ${LibRaw_INCLUDE_DIR}" )
  message( STATUS "LibRaw library dir: ${LibRaw_LIBRARY_DIR}" )
  message( STATUS "LibRaw libraries:   ${LibRaw_LIBRARIES}" )
endif()

if(LIBINTL_FOUND)
  message( STATUS "LibIntl include:     ${LIBINTL_INCLUDE_DIR}" )
  message( STATUS "LibIntl library dir: ${LIBINTL_LIBRARY_DIR}" )
  message( STATUS "LibIntl libraries:   ${LIBINTL_LIBRARIES}" )
endif()

if(TINYXML2_FOUND)
  message( STATUS "TinyXML2 include:    ${TINYXML2_INCLUDE_DIR}" )
  message( STATUS "TinyXML2 library:    ${TINYXML2_LIBRARY_DIR}" )
  message( STATUS "TinyXML2 libraries:  ${TINYXML2_LIBRARIES}" )
endif()

message( STATUS "mrViewer version: " ${SHORTVERSION} )
