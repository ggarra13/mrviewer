SET(UNIX 1)

# also add the install directory of the running cmake to the search directories
# CMAKE_ROOT is CMAKE_INSTALL_PREFIX/share/cmake, so we need to go two levels up
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${CMAKE_ROOT}" PATH)
GET_FILENAME_COMPONENT(_CMAKE_INSTALL_DIR "${_CMAKE_INSTALL_DIR}" PATH)

SET(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
  # Locals first
  /usr/local/include /opt/local/include 
 
  # Windows API on Cygwin
  /usr/include/w32api

  # X11
  /usr/X11R6/include /usr/include/X11

  # Other
  /opt/local/include /usr/pkg/include
  /opt/csw/include /opt/include  
  /usr/openwin/include
  "${_CMAKE_INSTALL_DIR}/include"
  "${CMAKE_INSTALL_PREFIX}/include"

  # Standard
  /usr/include /include 
  )

SET(CMAKE_SYSTEM_LIBRARY_PATH

  # Platforms first
  /usr/local/lib${CMAKE_BUILD_ARCH}
  /opt/local/lib${CMAKE_BUILD_ARCH}
  /opt/lib${CMAKE_BUILD_ARCH}
  /usr/lib${CMAKE_BUILD_ARCH}
  /lib${CMAKE_BUILD_ARCH}

  # Locals first
  /usr/local/lib /opt/local/lib 

  # Windows API on Cygwin
  /usr/lib/w32api

  # X11
  /usr/X11R6/lib /usr/lib/X11

  # Other
  /opt/local/lib /usr/pkg/lib
  /opt/csw/lib /opt/lib 
  /usr/openwin/lib
  "${_CMAKE_INSTALL_DIR}/lib"
  "${CMAKE_INSTALL_PREFIX}/lib"

  # Standard
  /usr/lib /lib
  )

SET(CMAKE_SYSTEM_PROGRAM_PATH
  /usr/local/bin${CMAKE_BUILD_ARCH} 
  /usr/pkg/bin${CMAKE_BUILD_ARCH} 
  /usr/bin${CMAKE_BUILD_ARCH} 
  /sbin${CMAKE_BUILD_ARCH} 
  /bin${CMAKE_BUILD_ARCH}

  /usr/local/bin /usr/pkg/bin /usr/bin /sbin /bin
  "${_CMAKE_INSTALL_DIR}/bin"
  "${CMAKE_INSTALL_PREFIX}/bin"
  )

SET(CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
  ${CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES}
  /usr/lib64 /usr/lib32 /usr/lib /lib 
  )

# Enable use of lib64 search path variants by default.
# SET_PROPERTIES(GLOBAL PROPERTIES FIND_LIBRARY_USE_LIB64_PATHS TRUE)

