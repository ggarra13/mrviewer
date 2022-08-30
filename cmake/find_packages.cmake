#
# These are the libraries we will depend on
#
set( OpenGL_GL_PREFERENCE LEGACY )
#set( OpenGL_GL_PREFERENCE GLVND )

set( Boost_USE_STATIC_LIBS OFF )

include(CMakeFindDependencyMacro)
# For window management
find_package( BuildDir    REQUIRED )    # for 32/64 bits handling (WIN64)
find_package( OpenGL      REQUIRED )    # for drawing images/shapes

set( ZLIB_ROOT ${CMAKE_PREFIX_PATH} )   # Make sure we pick latest zlib not
					# the system one
find_package( ZLIB        REQUIRED )    # for zlib compression

find_package( Boost COMPONENTS thread chrono system
			       filesystem date_time regex
			       REQUIRED )    # for file system support
add_definitions( -D BOOST_ALL_DYN_LINK ) # ${Boost_LIB_DIAGNOSTIC_DEFINITIONS} )

if (APPLE)
  if ( NOT DEFINED FLTK_DIR )
    set(FLTK_DIR /Users/gga/code/lib/fltk/build CACHE FILEPATH
      "fltk build dir" FORCE )
  endif()

  set( OSX_FRAMEWORKS "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio" )
  list( APPEND CMAKE_EXE_LINKER_FLAGS ${OSX_FRAMEWORKS} )
  list( APPEND CMAKE_SHARED_LINKER_FLAGS ${OSX_FRAMEWORKS} )

  # stop OpenGL deprecation warnings on MacOSX > 10.14
  add_definitions(-DGL_SILENCE_DEPRECATION=1)

else()
  if (UNIX)
    if ( NOT DEFINED FLTK_DIR )
	set( FLTK_DIR "${CMAKE_INSTALL_PREFIX}" CACHE FILEPATH
	  "fltk build dir" FORCE )
    endif()
  endif()
endif()

find_package( FLTK        REQUIRED NO_MODULE)
set( FLTK_LIBRARIES       fltk fltk_gl fltk_images )

set( FLTK_FLUID_EXECUTABLE "${FLTK_DIR}/bin/fluid" )

find_package( ImageMagick REQUIRED )    # for image formats
find_package( OpenEXR     REQUIRED )    # for EXR image loading
find_package( CTL	  REQUIRED )    # for CTL
find_package( OpenEXRCTL  REQUIRED )    # for CTL <-> EXR
find_package( OCIO        REQUIRED )    # for OCIO color correction
find_package( OIIO        REQUIRED )    # for OIIO image loading
find_package( FFMPEG      REQUIRED )    # for mpeg, avi, quicktime, wmv
find_package( Gettext     REQUIRED )    # for translations
find_package( TCLAP       REQUIRED )    # for command-line parsing
find_package( GLEW        REQUIRED )    # for opengl features
find_package( TIFF        REQUIRED )
find_package( LibIntl     REQUIRED )
find_package( SampleICC   REQUIRED )    # For ICC reading
find_package( LibRaw      REQUIRED )    # for libraw files
find_package( TinyXML2    REQUIRED )    # for xml reading/writing
find_package( R3DSDK      REQUIRED )    # for R3D format
find_package( BlackMagicRAW REQUIRED )  # for BRAW format

if( UNIX )
    # On Windows, these get compiled with the m-abs-media ffmpeg package
    find_package( LibVPX      REQUIRED )    # for libvpx codec
    find_package( LibOPUS      REQUIRED )   # for libopus codec
    if ( USE_FFMPEG_GPL )
      find_package( X264           REQUIRED )  # for lib264
      find_package( X265           REQUIRED )  # for lib265
    endif()
    find_package( WebP           REQUIRED )  # for webp
endif()

find_package( OpenTimelineIO REQUIRED )  # for OpenTimelineIO


add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=32 -DMAGICKCORE_HDRI_ENABLE=1 -DUSE_GETTEXT -D_CRT_SECURE_NO_WARNINGS -DFFMPEG_VERSION=${FFMPEG_VERSION} )

if( APPLE )
  set( USE_PORTAUDIO 1 )
endif()

if( USE_PORTAUDIO )
  find_package( PortAudio REQUIRED )
  if( PORTAUDIO_FOUND )
    add_definitions( -DMRV_PORTAUDIO )
  endif()
endif()


add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=32 -DMAGICKCORE_HDRI_ENABLE=1
		 -DUSE_GETTEXT -D_CRT_SECURE_NO_WARNINGS )


if(WIN32 OR WIN64)

  add_definitions( -DMR_SSE -DWIN32 -DNOMINMAX -DIMATH_DLL -DOPENEXR_DLL
		   -D_WIN32_WINNT=0x0601 )

  set( OS_LIBRARIES GDIplus Winmm ws2_32 Psapi ${GLEW_LIBRARIES} )

  set( LINK_FLAGS "${LINK_FLAGS} -OPT:NOREF" )

else()

  find_library( png png
    PATHS
    ${CMAKE_PREFIX_PATH}/lib64
    ${CMAKE_PREFIX_PATH}/lib
    NO_SYSTEM_PATH
    )

  find_library( jpeg jpeg
    PATHS
    ${CMAKE_PREFIX_PATH}/lib64
    ${CMAKE_PREFIX_PATH}/lib
    NO_SYSTEM_PATH
    )

  if(APPLE)
    # Media libraries for Apple
    add_definitions( -DOSX -DMR_SSE -D__MACOSX_CORE__ )
    add_compile_options( -msse -msse2 -Wno-format-security -Wno-deprecated-declarations -I/usr/local/include/ImageMagick-7/MagickCore )


    link_directories( ${CMAKE_PREFIX_PATH}/lib /usr/local/lib )
    set( OS_LIBRARIES ${OS_LIBRARIES} ass ${Xpm} ${png} ${jpeg} pthread fontconfig GLEW lzma mp3lame theoraenc theoradec theora vorbisenc vorbis )


    if( CMAKE_BUILD_TYPE STREQUAL "Debug"  )
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


  add_definitions( -DLINUX )
  add_compile_options( -O3 -msse )
  link_directories( "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64" )
  set(OS_LIBRARIES
    asound ass ${Xpm} ${png} ${jpeg} dl X11 Xext pthread Xinerama Xfixes Xcursor Xft Xrender Xss m fontconfig dl Xi Xext GLEW lzma mp3lame theoraenc theoradec theora vorbisenc vorbis ### dvdnav dvdread
    )

   # if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
   #   add_compile_options( -g -fsanitize=address -rdynamic )
   #   set(LIBRARIES asan ${LIBRARIES} )
   # endif( CMAKE_BUILD_TYPE STREQUAL "Debug" )

  endif()
endif()




if( WIN32 )
  #
  # Under windows, boost .h files select the appropriate boost static library
  # automatically to handle the correct linking.
  #
  link_directories( ${Boost_LIBRARY_DIRS} )
endif()

if( R3DSDK_FOUND )
  include_directories( ${R3DSDK_INCLUDE_DIR} )
  list( APPEND LIBRARIES ${R3DSDK_LIBRARIES} )
endif()

if ( BlackMagicRAW_FOUND )
  include_directories( "${BlackMagicRAW_INCLUDE_DIR}" )
  list( APPEND LIBRARIES ${BlackMagicRAW_LIBRARIES} )
endif()

list( APPEND LIBRARIES
  ${LIBINTL_LIBRARIES}
  ${FFMPEG_LIBRARIES}
  ${MAGICK++_LIBRARIES}
  ${OPENEXR_LIBRARIES}
  ${FLTK_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${CTL_LIBRARIES}
  ${OpenEXRCTL_LIBRARIES}
  ${OCIO_LIBRARIES}
  ${OIIO_LIBRARIES}
  ${TIFF_LIBRARIES}
  ${SampleICC_LIBRARIES}
  ${LibRaw_LIBRARIES}
  ${OPENTIMELINEIO_LIBRARIES}
  ${TINYXML2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${OS_LIBRARIES}
  )

if( UNIX )
  list( INSERT LIBRARIES 0 ${LibOPUS_LIBRARIES} ${WEBP_LIBRARIES} )
  if ( USE_VPX )
    list( INSERT LIBRARIES 0 ${LibVPX_LIBRARIES} )
  endif()
  if ( USE_FFMPEG_GPL )
    list( INSERT LIBRARIES 0 ${X264_LIBRARIES} )
    if ( USE_X265 )
      list( INSERT LIBRARIES 0 ${X265_LIBRARIES} )
    endif()
  endif()
endif()

if( PORTAUDIO_FOUND )
    set( LIBRARIES ${PORTAUDIO_LIBRARIES} ${LIBRARIES} )
endif()

DEBUG_LIBRARIES( ${LIBRARIES} )


#
# Print out some status to verify configuration
#
message( STATUS
  "-------------------------------------------------------------" )
message( STATUS "Summary for mrViewer:" )
message( STATUS
  "-------------------------------------------------------------" )
message( STATUS "OpenGL:        ${OPENGL_FOUND} ${OPENGL_LIBRARIES}" )
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

if(MAGICK_FOUND)
  message( STATUS "ImageMagick   include: ${MAGICK_INCLUDE_DIR}" )
  message( STATUS "ImageMagick   library: ${MAGICK_LIBRARY_DIR}" )
endif()


if(OPENEXR_FOUND)
  message( STATUS "OpenEXR include:       ${OPENEXR_INCLUDE_DIR}" )
  message( STATUS "OpenEXR library dir:   ${OPENEXR_LIBRARY_DIR}" )
  message( STATUS "OpenEXR libs:          ${OPENEXR_LIBRARIES}" )
endif()

if(FLTK_FOUND)
  message( STATUS "FLTK include:         ${FLTK_INCLUDE_DIR}" )
  message( STATUS "FLTK library:         ${FLTK_LIBRARY_DIR}" )
  message( STATUS "FLTK libs:            ${FLTK_LIBRARIES}" )
endif()

if(Boost_FOUND)
  message( STATUS "Boost include:         ${Boost_INCLUDE_DIR}" )
  message( STATUS "Boost library dirs:    ${Boost_LIBRARY_DIR}" )
  message( STATUS "Boost library:         ${Boost_LIBRARIES}" )
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

if(LibOPUS_FOUND)
  message( STATUS "LibOPUS include:     ${LibOPUS_INCLUDE_DIR}" )
  message( STATUS "LibOPUS library dir: ${LibOPUS_LIBRARY_DIR}" )
  message( STATUS "LibOPUS libraries:   ${LibOPUS_LIBRARIES}" )
endif()

if(LibVPX_FOUND)
  message( STATUS "LibVPX include:     ${LibVPX_INCLUDE_DIR}" )
  message( STATUS "LibVPX library dir: ${LibVPX_LIBRARY_DIR}" )
  message( STATUS "LibVPX libraries:   ${LibVPX_LIBRARIES}" )
endif()


if(LIBINTL_FOUND)
  message( STATUS "LibIntl include:     ${LIBINTL_INCLUDE_DIR}" )
  message( STATUS "LibIntl library dir: ${LIBINTL_LIBRARY_DIR}" )
  message( STATUS "LibIntl libraries:   ${LIBINTL_LIBRARIES}" )
endif()

if(R3DSDK_FOUND)
  message( STATUS "R3DSDK include:         ${R3DSDK_INCLUDE_DIR}" )
  message( STATUS "R3DSDK libs:            ${R3DSDK_LIBRARIES}" )
endif()


if(TINYXML2_FOUND)
  message( STATUS "TinyXML2 include:    ${TINYXML2_INCLUDE_DIR}" )
  message( STATUS "TinyXML2 library:    ${TINYXML2_LIBRARY_DIR}" )
  message( STATUS "TinyXML2 libraries:  ${TINYXML2_LIBRARIES}" )
endif()

message( STATUS "mrViewer version: " ${SHORTVERSION} )
