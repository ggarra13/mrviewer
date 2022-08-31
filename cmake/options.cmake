#
# CMake options for FMmpeg
#
option( USE_PORTAUDIO    "Use PortAudio interface" OFF )
option( USE_FFMPEG_GPL   "Use FFMPEG GPL codecs" ON )
option( USE_X264         "Use x264 library for encoding movies" ON )
option( USE_X265         "Use x265 library for encoding movies" ON )
option( USE_VPX          "Use VPX codec" ON  )
option( USE_BRAW         "Use BlackMagic RAW codec"  ON)
option( USE_R3DSDK       "Use RED 3D SDK codec" ON  )






#  OIIO needs c++14, so let's do that by default
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ ISO Standard")

# but switch gnu++11 or other extensions off for portability
set(CMAKE_CXX_EXTENSIONS OFF)


# This sets -fPIC on all libraries and code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# use, i.e. don't skip the full RPATH for the build tree

# set @rpaths for libraries to link against

set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_SKIP_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_PREFIX_PATH}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)



# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_SKIP_INSTALL_RPATH TRUE)

# the RPATH to be used when installing
set(CMAKE_INSTALL_RPATH "")


message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")
message("-- CXX STANDARD:           ${CMAKE_CXX_STANDARD}")
message("-- C   STANDARD:           ${CMAKE_C_STANDARD}")

string (REGEX MATCH "\\.el[1-9]" os_version_suffix ${CMAKE_SYSTEM})
message("-- os_version_suffix:      ${os_version_suffix}")

set( CENTOS7 OFF )
if ( "${os_version_suffix}" STREQUAL ".el7" )
  set( CENTOS7 ON )
endif()
