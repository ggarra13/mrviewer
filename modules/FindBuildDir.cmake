#-*-cmake-*-
#
# This simple CMake extension makes sure that builds get
# created inside a BUILD/os-osversion-arch directory
# and that executables, obj files and lib files get placed
# correctly in subdirectories.
#
#
# Macro to check architecture
#
MACRO( CHECK_ARCHITECTURE )

  SET( CMAKE_BUILD_ARCH $ENV{CMAKE_BUILD_ARCH} )

  IF( NOT CMAKE_BUILD_ARCH )
    SET( CMAKE_BUILD_ARCH "Native" )
  ENDIF( NOT CMAKE_BUILD_ARCH )

  IF( NOT CMAKE_BUILD_ARCH MATCHES "^(Native|64|32)$" )
    MESSAGE( FATAL_ERROR 
      "CMAKE_BUILD_ARCH set but invalid.  "
      "Only Native, 64 and 32 are valid settings" )
  ENDIF( NOT CMAKE_BUILD_ARCH MATCHES "^(Native|64|32)$" )

  #
  # Set Native architecture based on void* size
  #
  INCLUDE(CheckTypeSize)
  CHECK_TYPE_SIZE(void*  SIZEOF_VOID_PTR)

  IF( ${SIZEOF_VOID_PTR} MATCHES "^8$" )
    SET( OS_32_BITS 0 )
    SET( OS_64_BITS 1 )
    SET( CMAKE_NATIVE_ARCH 64 )
  ELSE( ${SIZEOF_VOID_PTR} MATCHES "^8$" )
    SET( OS_32_BITS 1 )
    SET( OS_64_BITS 0 )
    SET( CMAKE_NATIVE_ARCH 32 )
  ENDIF( ${SIZEOF_VOID_PTR} MATCHES "^8$" )

  IF( UNIX )
    EXECUTE_PROCESS( 
      COMMAND uname -a
      OUTPUT_VARIABLE OS_ARCH 
      )

    #
    # For Linux
    #
    IF( OS_ARCH MATCHES ".*Linux.*" )
      IF( OS_ARCH MATCHES ".*x86_64.*" )
	SET( OS_32_BITS 1 )
      ELSEIF( OS_ARCH MATCHES ".*ia64.*" )
	SET( OS_32_BITS 0 )
      ENDIF( OS_ARCH MATCHES ".*x86_64.*" )
    ENDIF( OS_ARCH MATCHES ".*Linux.*" )

    #
    # For SUN
    #
    IF( OS_ARCH MATCHES ".*SunOS.*" )
      EXECUTE_PROCESS( 
	COMMAND isainfo -v
	OUTPUT_VARIABLE SUNOS_ARCH 
	)

      IF ( SUNOS_ARCH MATCHES ".*64-bit.*" )
	SET( OS_32_BITS 1 )
      ENDIF(  SUNOS_ARCH MATCHES ".*64-bit.*" )

    ENDIF( OS_ARCH MATCHES ".*SunOS.*" )

    IF( APPLE )
      #
      # @todo: add Apple 64-bit OS detection here
      #
    ENDIF( APPLE )

  ELSE( UNIX )

    #
    # @todo: add windows 64-bit OS detection here
    #
    IF (WIN64)
       SET( OS_32_BITS 1 )
       SET( OS_64_BITS 1 )
    ELSE(WIN64)
       SET( OS_32_BITS 1 )
       SET( OS_64_BITS 0 )
    ENDIF(WIN64)

  ENDIF( UNIX )


  IF ( CMAKE_BUILD_ARCH STREQUAL "Native" )
    SET( CMAKE_BUILD_ARCH ${CMAKE_NATIVE_ARCH} )
  ENDIF( CMAKE_BUILD_ARCH STREQUAL "Native" )

  IF( CMAKE_BUILD_ARCH EQUAL 32 )

    IF( NOT OS_32_BITS )
      MESSAGE( FATAL_ERROR 
	"Sorry, but this platform cannot compile 32-bit applications" )
    ENDIF( NOT OS_32_BITS )

    IF( CMAKE_COMPILER_IS_GNUCXX )
      ADD_DEFINITIONS( -m32 )
      SET( LINK_FLAGS "-m32 ${LINK_FLAGS}" )
    ELSE( CMAKE_COMPILER_IS_GNUCXX )

      #
      # @todo: add 32-bit compile flags for non-GNU compilers here
      #

    ENDIF( CMAKE_COMPILER_IS_GNUCXX )

  ENDIF( CMAKE_BUILD_ARCH EQUAL 32 )

ENDMACRO( CHECK_ARCHITECTURE )


CHECK_ARCHITECTURE()

#
# Store build type
#
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

IF( CMAKE_BUILD_TYPE STREQUAL "Debug" )
  ADD_DEFINITIONS( -DDEBUG )
  IF( CMAKE_COMPILER_IS_GNUCXX )
    ADD_DEFINITIONS( -g )
    SET( LINK_FLAGS "-g ${LINK_FLAGS}" )
  ENDIF( CMAKE_COMPILER_IS_GNUCXX )
ENDIF( CMAKE_BUILD_TYPE STREQUAL "Debug" )

IF( NOT CMAKE_SYSTEM )
  MESSAGE( FATAL_ERROR "CMAKE_SYSTEM was not set" )
ENDIF( NOT CMAKE_SYSTEM )

#
# @bug in cmake2.5 in windows (workaround)
#


IF( NOT CMAKE_BUILD_TYPE )
  MESSAGE( FATAL_ERROR "CMAKE_BUILD_TYPE was not set" )
ENDIF( NOT CMAKE_BUILD_TYPE )

IF( NOT CMAKE_BUILD_ARCH )
  MESSAGE( FATAL_ERROR "CMAKE_BUILD_ARCH was not set" )
ENDIF( NOT CMAKE_BUILD_ARCH )


SET( BUILD_DIR "${CMAKE_SOURCE_DIR}/BUILD/${CMAKE_SYSTEM}-${CMAKE_BUILD_ARCH}/${CMAKE_BUILD_TYPE}" )

SET( PROJECT_BINARY_DIR "${BUILD_DIR}" )
SET( EXECUTABLE_OUTPUT_PATH "${BUILD_DIR}/bin" )
# SET( CMAKE_BINARY_DIR "${BUILD_DIR}/obj" )
SET( LIBRARY_OUTPUT_PATH "${BUILD_DIR}/lib" )
SET( CMAKE_LIBRARY_PATH ${LIBRARY_OUTPUT_PATH} ${CMAKE_LIBRARY_PATH} )

FILE( MAKE_DIRECTORY "${BUILD_DIR}" )
FILE( MAKE_DIRECTORY "${PROJECT_BINARY_DIR}" )
FILE( MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}" )
FILE( MAKE_DIRECTORY "${LIBRARY_OUTPUT_PATH}" )


#
# Turn off crappy cmake's use of RPATH, which prevents the proper use
# of LD_LIBRARY_PATH
#
SET( CMAKE_SKIP_RPATH "ON" )


#
# If compiling in 32-bits mode on a 64-bits machine, place 32-bits
# libraries first
#
IF( ${CMAKE_CXX_FLAGS} MATCHES "-m32" )
  MESSAGE( STATUS ${CMAKE_SYSTEM_LIBRARY_PATH} )

  #
  # Add 32-bits paths first
  #
  SET( CMAKE_SYSTEM_LIBRARY_PATH 
    /usr/local/lib32 /usr/X11R6/lib32 /usr/lib32 /lib32 
    ${CMAKE_SYSTEM_LIBRARY_PATH} )

  #
  # Remove 64-bits paths
  #
  LIST( REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH 
    /usr/local/lib64 /opt/local/lib64 /usr/X11R6/lib64 /usr/lib64/X11 
    /usr/lib64 /lib64 /lib /usr/lib )

  # MESSAGE( FATAL_ERROR ${CMAKE_SYSTEM_LIBRARY_PATH} )

ENDIF( ${CMAKE_CXX_FLAGS} MATCHES "-m32" )


#
# Macro used to easily add linker flags to a target
#
MACRO( TARGET_LINK_FLAGS TARGET FLAGS )
  GET_TARGET_PROPERTY( OLD_FLAGS ${TARGET} LINK_FLAGS )
  SET_TARGET_PROPERTIES( 
    ${TARGET}
    PROPERTIES 
    LINK_FLAGS "${FLAGS} ${OLD_FLAGS}"
    )
ENDMACRO( TARGET_LINK_FLAGS )
