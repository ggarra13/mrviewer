#-*-cmake-*-
#
# This auxiliary CMake file helps in creating Pixie plug-ins easily.
#
# It determines maya version, compiler and good settings for compiling a
# maya plug-in.
#
#
# Pixie_FOUND            set if maya is found.
# PIXIE_HOME             install location of Pixie.
# Pixie_INCLUDE_DIR      maya's include directory
# Pixie_LIBRARY_DIR      maya's library directory
# Pixie_LIBRARIES        all maya libraries (sans Pixie_IMAGE_LIBRARIES)
#
# Pixie_RIB_LIBRARIES
# Pixie_SHADER_LIBRARIES
# Pixie_BASE_LIBRARIES
#
#

SET( Pixie_FOUND "NO" )

##
## Obtain Pixie install location
##
FIND_PATH( PIXIE_HOME include/ri.h
  "$ENV{PIXIE_HOME}"
  /usr/local/pixie
  DOC "Root directory of Pixie"
  )


SET( Pixie_INCLUDE_DIR       "${PIXIE_HOME}/include" )
SET( Pixie_LIBRARY_DIR       "${PIXIE_HOME}/lib" )


FIND_LIBRARY( Pixie_common_LIBRARY pixiecommon
  ${Pixie_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( Pixie_ri_LIBRARY ri
  ${Pixie_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( Pixie_shader_LIBRARY sdr
  ${Pixie_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )


SET( Pixie_BASE_LIBRARIES   ${Pixie_common_LIBRARY} )
SET( Pixie_SHADER_LIBRARIES ${Pixie_BASE_LIBRARIES} ${Pixie_shader_LIBRARY} )
SET( Pixie_RIB_LIBRARIES    ${Pixie_BASE_LIBRARIES} ${Pixie_ri_LIBRARY} )

SET( Pixie_LIBRARIES  ${Pixie_SHADER_LIBRARIES} ${Pixie_RIB_LIBRARIES}  )



ADD_DEFINITIONS( 
  -D_BOOL
  -DFUNCPROTO
  -DREQUIRE_IOSTREAM 
 )

#
# Add Pixie include directory
#
INCLUDE_DIRECTORIES( "${Pixie_INCLUDE_DIR}" )


#
# Macro used to set Pixie's link variables
#
MACRO( Pixie_DEFINITIONS )

  IF( WIN32 )

    ADD_DEFINITIONS( 
      -nologo -D_WIN32 -DWIN32 -DNT_PLUGIN -D_WINDOWS -D_AFXDLL -D_MBCS -D_WINDLL 
      )

    SET( Pixie_LINK_FLAGS "-nologo " )
    SET( OS_LIBRARIES "ws2_32.lib user32.lib mpr.lib" )
    SET( Pixie_EXTENSION ".mll" )


    ADD_DEFINITIONS( -DMLIBRARY_DONTUSE_MFC_MANIFEST )

  ELSE( WIN32 )

    #
    # These switches are mainly taken from Pixie's demos
    #
    ADD_DEFINITIONS( 
      -D_GNU_SOURCE
      -Wall
      -Wextra
      -Wno-unused-parameter
      -fno-strict-aliasing
      -funsigned-char # as per devkit Makefile
      # -Wshadow
      # -Wwrite-strings
      # -Wfloat-equal
      # -Wpadded      # use it to reduce struct size
      )

    #
    # These switches are specific to C++, so we don't use ADD_DEFINITIONS
    #
    SET( CMAKE_CXX_FLAGS 
      "-fno-gnu-keywords -ftemplate-depth-25 ${CMAKE_CXX_FLAGS}"
      )

    SET( OS_LIBRARIES "" )
    SET( Pixie_EXTENSION ".so" )

    IF( CMAKE_BUILD_TYPE STREQUAL "Release" OR
	CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" )

#       # Strip symbols on release builds
#       ADD_DEFINITIONS( -s )

#       # Make symbols hidden by default
#       IF( NOT Pixie_NO_HIDDEN_VISIBILITY )
# 	ADD_DEFINITIONS( -fvisibility=hidden )
#       ENDIF( NOT Pixie_NO_HIDDEN_VISIBILITY )

    ENDIF( CMAKE_BUILD_TYPE STREQUAL "Release" OR
      CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"  )



    IF ( APPLE )

      #
      # If Pixie version is less than 8.0, then we compile as 32-bit for sure.
      #
      IF( Pixie_MAJOR_VERSION LESS 8 )

	FIND_FILE( G33 g++-3.3 g++33 g++-33
	  DOC "gcc-3.3 compiler for maya"
	  )

	IF (NOT G33)
	  MESSAGE( FATAL_ERROR 
	    "gcc-3.3 compiler needed for Pixie not found." )
	ENDIF(NOT G33)
	
	SET( CMAKE_CXX_COMPILER ${G33} )
      ELSE( Pixie_MAJOR_VERSION LESS 8 )
	SET( CMAKE_CXX_COMPILER "g++" )
      ENDIF( Pixie_MAJOR_VERSION LESS 8 )
      
      
      ADD_DEFINITIONS( -fno-common -DMACOSX -D_REENTRANT  -DOSX )
      SET( Pixie_LINK_FLAGS "-flat_namespace -undefined suppress" )
      SET( OS_LIBRARIES "nspr4 plds4 m c" )

    ELSE( APPLE )


      ADD_DEFINITIONS( -pthread -DLINUX )

      #
      # Do *NOT* set this as this creates problems with SWIG
      #
      # SET( Pixie_LINK_FLAGS "-Wl,-Bsymbolic" )


      SET( OS_LIBRARIES "" )

    ENDIF( APPLE )


  ENDIF( WIN32 )


ENDMACRO( Pixie_DEFINITIONS )



IF(NOT PIXIE_HOME)

  IF(NOT Pixie_FIND_QUIETLY)
    IF(Pixie_FIND_REQUIRED)
      MESSAGE( FATAL_ERROR "Environment variable PIXIE_HOME not defined." )
    ELSE(Pixie_FIND_REQUIRED)
      MESSAGE( WARNING ": Environment variable PIXIE_HOME not defined." )
    ENDIF(Pixie_FIND_REQUIRED)
  ENDIF(NOT Pixie_FIND_QUIETLY)

ENDIF(NOT PIXIE_HOME)

