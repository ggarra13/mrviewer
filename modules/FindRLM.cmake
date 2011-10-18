# - Find the RLM includes and libraries.
# The following variables are set if RLM is found.  If RLM is not
# found, RLM_FOUND is set to false.
#  RLM_FOUND       - True when the RLM include directory is found.
#  RLM_INCLUDE_DIR - the path to where the rlm include files are.
#  RLM_LIBRARY_DIR - The path to where the rlm library files are.
#  RLM_LIBRARIES   - The actual rlm vendor library.
#


SET(RLM_INCLUDE_PATH_DESCRIPTION "directory containing the rlm files. E.g /usr/local/include/rlm or c:\\rlm")

SET(RLM_DIR_MESSAGE "Set the RLM_INCLUDE_DIR cmake cache entry to the ${RLM_INCLUDE_PATH_DESCRIPTION}")

SET(RLM_DIR_SEARCH $ENV{RLM_ROOT})

IF(RLM_DIR_SEARCH)
  FILE(TO_CMAKE_PATH ${RLM_DIR_SEARCH} RLM_DIR_SEARCH)
  SET(RLM_DIR_SEARCH ${RLM_DIR_SEARCH})
ELSE( RLM_DIR_SEARCH)
  SET(RLM_DIR_SEARCH
    "$ENV{PROGRAMFILES}/rlm"
    "/home/gga/code/licensing/RLM"
    Z:/code/licensing/RLM
    )
ENDIF(RLM_DIR_SEARCH)


#
# Look for an installation.
#
FIND_PATH(RLM_ROOT_DIR
  NAMES src/license_to_run.h
  PATHS

  # Look in other places.
  ${RLM_DIR_SEARCH}

  # Help the user find it if we cannot.
  DOC "The RLM LICENSE manager"
)

# Assume we didn't find it.
SET(RLM_FOUND "NO")

# Now try to get the include and library path.
IF(RLM_ROOT_DIR)


  IF( WIN32 OR WIN64 OR CYGWIN OR MINGW )

    IF ( CMAKE_BUILD_ARCH EQUAL 32 )
      SET(RLM_PREFIX "x86" )
      SET( RLM_SUFFIX "_w1" )
    ELSE( CMAKE_BUILD_ARCH EQUAL 32 )
      SET(RLM_PREFIX "x64" )
      SET( RLM_SUFFIX "_w2" )
    ENDIF ( CMAKE_BUILD_ARCH EQUAL 32 )
  ENDIF( WIN32 OR WIN64 OR CYGWIN OR MINGW )

  IF( UNIX )
    IF ( CMAKE_BUILD_ARCH EQUAL 32 )
      SET(RLM_PREFIX "x86" )
      SET( RLM_SUFFIX "_l2" )
    ELSE( CMAKE_BUILD_ARCH EQUAL 32 )
      SET(RLM_PREFIX "x64" )
      SET( RLM_SUFFIX "_l1" )
    ENDIF ( CMAKE_BUILD_ARCH EQUAL 32 )
  ENDIF( UNIX )


  SET( RLM_INCLUDE_DIR ${RLM_ROOT_DIR}/src )
  SET( RLM_LIBRARY_DIR ${RLM_ROOT_DIR}/${RLM_PREFIX}${RLM_SUFFIX} )

  FIND_LIBRARY( RLM_LIBRARIES 
    NAMES rlm rlmclient_md rlmclient
    PATHS
    ${RLM_LIBRARY_DIR}
    )


  IF( RLM_LIBRARIES AND RLM_INCLUDE_DIR )
    SET(RLM_FOUND "YES")
  ENDIF( RLM_LIBRARIES AND RLM_INCLUDE_DIR )

  IF(WIN32 OR WIN64 OR CYGWIN OR MINGW)
    SET( RLM_LIBRARIES ${RLM_LIBRARIES} ws2_32.lib Advapi32.lib Gdi32.lib User32.lib netapi32.lib kernel32.lib oldnames.lib shell32.lib )
  ENDIF(WIN32 OR WIN64 OR CYGWIN OR MINGW)

ENDIF(RLM_ROOT_DIR)

IF(NOT RLM_FOUND)
  IF(RLM_FIND_QUIETLY)
    MESSAGE(STATUS "RLM was not found. ${RLM_DIR_MESSAGE}")
  ELSE(RLM_FIND_QUIETLY)
    IF(RLM_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "RLM was not found. ${RLM_DIR_MESSAGE} ${RLM_INCLUDE_DIR} ${RLM_LIBRARY_DIR} ${RLM_LIBRARIES}")
    ENDIF(RLM_FIND_REQUIRED)
  ENDIF(RLM_FIND_QUIETLY)
ENDIF(NOT RLM_FOUND)
