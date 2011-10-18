#-*-cmake-*-
#
# This auxiliary CMake file helps in creating Maya plug-ins easily.
#
# It determines maya version, compiler and good settings for compiling a
# maya plug-in.
#
#
# MAYA_FOUND            set if maya is found.
# MAYA_LOCATION         install location of maya.
# MAYA_INCLUDE_DIR      maya's include directory
# MAYA_LIBRARY_DIR      maya's library directory
# MAYA_LIBRARIES        all maya libraries (sans MAYA_IMAGE_LIBRARIES)
#
# MAYA_API_VERSION      maya's API version integer  (710)
# MAYA_VERSION          maya's full version name    (7.1 or 8.5-x64)
# MAYA_MAJOR_VERSION    maya's version major number (7)
# MAYA_MINOR_VERSION    maya's version minor number (1)
# MAYA_ARCH_VERSION     maya's version architecture (32/64)
# MAYA_LINK_FLAGS       maya's linker flags
# MAYA_APP_LIBRARIES    maya's app    libraries (for stand-alone application)
# MAYA_BASE_LIBRARIES   maya's base   libraries
# MAYA_ANIM_LIBRARIES   maya's anim   libraries
# MAYA_FX_LIBRARIES     maya's fx     libraries
# MAYA_UI_LIBRARIES     maya's ui     libraries
# MAYA_RENDER_LIBRARIES maya's render libraries
# MAYA_CLOTH_LIBRARIES  maya's cloth  libraries
# MAYA_IMAGE_LIBRARIES  maya's image  libraries
#
# MAYA_MENTALRAY_LIBRARIES  maya's mental ray libraries (mayabase for example)
#
# We also define some simple macros called CREATE_MAYA_PLUG_IN(),
# CREATE_MAYA_PLUG_IN_SOG() and CREATE_MAYA_LIBRARY().
#
#

SET( MAYA_FOUND "NO" )

##
## Obtain Maya install location
##
FIND_PATH( MAYA_LOCATION include/maya/MLibrary.h
  "$ENV{MAYA_LOCATION}"
  /usr/autodesk/maya8.5-x64
  /usr/autodesk/maya8.0-x64
  /usr/aw/maya8.0-x64
  /usr/autodesk/maya8.5
  /usr/autodesk/maya8.0
  /usr/aw/maya8.0
  /usr/aw/maya7.0
  /usr/aw/maya6.5
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.5-x64"
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.5"
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.0-x64"
  "$ENV{PROGRAMFILES}/Autodesk/Maya8.0"
  "$ENV{PROGRAMFILES}/Alias/Maya8.0-x64"
  "$ENV{PROGRAMFILES}/Alias/Maya8.0"
  "$ENV{PROGRAMFILES}/Alias/Maya7.0"
  "$ENV{PROGRAMFILES}/Alias/Maya6.5"
  DOC "Root directory of Maya"
  )

IF( WIN32 )
  FIND_LIBRARY( MAYABASE_MENTALRAY_LIB mayabase
    "${MAYA_LOCATION}/devkit/mentalray/lib/nt"
    NO_DEFAULT_PATH
    DOC "Directory for Maya's mentalray auxiliary mayabase library"
    )
ELSE( WIN32 )
  SET( MAYABASE_MENTALRAY_LIB "" )
ENDIF( WIN32 )


SET( MAYA_MENTALRAY_LIBRARIES "${MAYABASE_MENTALRAY_LIB}" )

SET( MAYA_INCLUDE_DIR       "${MAYA_LOCATION}/include" )
SET( MAYA_LIBRARY_DIR       "${MAYA_LOCATION}/lib" )

IF(UNIX)
  SET( MAYA_APP_LIBRARIES OpenMayalib )
ELSE(UNIX)
  SET( MAYA_APP_LIBRARIES "" )
ENDIF(UNIX)

FIND_LIBRARY( MAYA_Foundation_LIBRARY Foundation
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( MAYA_OpenMaya_LIBRARY OpenMaya
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( MAYA_OpenMayaAnim_LIBRARY OpenMayaAnim
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( MAYA_OpenMayaUI_LIBRARY OpenMayaUI
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( MAYA_OpenMayaRender_LIBRARY OpenMayaRender
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( MAYA_OpenMayaFX_LIBRARY OpenMayaFX
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( MAYA_Cloth_LIBRARY Cloth
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

FIND_LIBRARY( MAYA_Image_LIBRARY Image
  ${MAYA_LIBRARY_DIR}
  NO_DEFAULT_PATH
  )

SET( MAYA_BASE_LIBRARIES    
  ${MAYA_Foundation_LIBRARY}
  ${MAYA_OpenMaya_LIBRARY} )
SET( MAYA_ANIM_LIBRARIES    
  ${MAYA_BASE_LIBRARIES} 
  ${MAYA_OpenMayaAnim_LIBRARY}  )
SET( MAYA_UI_LIBRARIES      
  ${MAYA_BASE_LIBRARIES} ${MAYA_OpenMayaUI_LIBRARY} )
SET( MAYA_RENDER_LIBRARIES  
  ${MAYA_BASE_LIBRARIES} ${MAYA_OpenMayaRender_LIBRARY} )
SET( MAYA_FX_LIBRARIES      
  ${MAYA_BASE_LIBRARIES} ${MAYA_OpenMayaFX_LIBRARY} )
SET( MAYA_CLOTH_LIBRARIES   
  ${MAYA_BASE_LIBRARIES} ${MAYA_Cloth_LIBRARY} )
SET( MAYA_IMAGE_LIBRARIES   ${MAYA_Image_LIBRARY} )


SET( MAYA_LIBRARIES 
  ${MAYA_BASE_LIBRARIES} ${MAYA_ANIM_LIBRARIES} ${MAYA_UI_LIBRARIES} 
  ${MAYA_FX_LIBRARIES} ${MAYA_IMAGE_LIBRARIES} 
  )



ADD_DEFINITIONS( 
  -D_BOOL
  -DFUNCPROTO
  -DREQUIRE_IOSTREAM 
 )


#
# Add Maya include directory
#
INCLUDE_DIRECTORIES( "${MAYA_INCLUDE_DIR}" )

#
# Set compiler flags to compile in 64-bits in a 64-bit system
#
MACRO( MAYA_64_BITS )
  ADD_DEFINITIONS( -DBits64_ )
ENDMACRO( MAYA_64_BITS )

#
# Set compiler flags to compile in 32-bits in a 64-bit system
#
MACRO( MAYA_32_BITS )
  ADD_DEFINITIONS( -m32 -I/usr/include/i486-linux-gnu )
  SET( MAYA_LINK_FLAGS "-m32 -Wl,-melf_i386 -Wl,-rpath,/usr/lib32:/lib32  -Wl,-L/usr/lib32 -Wl,-L/lib32 ${MAYA_LINK_FLAGS}" )
ENDMACRO( MAYA_32_BITS )


#
# Macro used to set MAYA's link variables
#
MACRO( MAYA_DEFINITIONS )

  IF( WIN32 )

    ADD_DEFINITIONS( 
      -nologo -D_WIN32 -DWIN32 -DNT_PLUGIN -D_WINDOWS -D_AFXDLL -D_MBCS -D_WINDLL 
      )

    SET( MAYA_LINK_FLAGS "-nologo " )
    SET( OS_LIBRARIES "ws2_32.lib user32.lib mpr.lib" )
    SET( MAYA_EXTENSION ".mll" )


    ADD_DEFINITIONS( -DMLIBRARY_DONTUSE_MFC_MANIFEST )

  ELSE( WIN32 )

    #
    # These switches are mainly taken from Maya's demos
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
    SET( MAYA_EXTENSION ".so" )



    IF ( APPLE )

      #
      # If Maya version is less than 8.0, then we compile as 32-bit for sure.
      #
      IF( MAYA_MAJOR_VERSION LESS 8 )

	FIND_FILE( G33 g++-3.3 g++33 g++-33
	  DOC "gcc-3.3 compiler for maya"
	  )

	IF (NOT G33)
	  MESSAGE( FATAL_ERROR 
	    "gcc-3.3 compiler needed for Maya not found." )
	ENDIF(NOT G33)
	
	SET( CMAKE_CXX_COMPILER ${G33} )
      ELSE( MAYA_MAJOR_VERSION LESS 8 )
	SET( CMAKE_CXX_COMPILER "g++" )
      ENDIF( MAYA_MAJOR_VERSION LESS 8 )
      
      
      ADD_DEFINITIONS( -fno-common -DMACOSX -D_REENTRANT  -DOSX )
      SET( MAYA_LINK_FLAGS 
	"-flat_namespace -undefined suppress ${MAYA_LINK_FLAGS" )
      SET( OS_LIBRARIES "nspr4 plds4 m c" )

    ELSE( APPLE )


      ADD_DEFINITIONS( -pthread -DLINUX )

      #
      # Do *NOT* set this as this creates problems with SWIG
      #
      # SET( MAYA_LINK_FLAGS "-Wl,-Bsymbolic" )

      #
      # If Maya version is less than 8.0, then we compile as 32-bit for sure.
      #
      IF( MAYA_MAJOR_VERSION LESS 8 )

        MAYA_32_BITS()

	ADD_DEFINITIONS( -Wno-deprecated )

	FIND_FILE( G34 g++-3.4 g++34 g++-34
	  PATHS /usr/bin /bin /usr/local/bin
	  DOC "gcc-3.4 compiler for maya"
	  )

	IF (NOT G34)
	  MESSAGE( FATAL_ERROR "gcc-3.4 compiler needed for Maya not found." )
	ENDIF(NOT G34)

	SET( CMAKE_CXX_COMPILER ${G34} )
      ELSE( MAYA_MAJOR_VERSION LESS 8 )

	IF( MAYA_ARCH_VERSION EQUAL 64 )
	  MAYA_64_BITS()
	  ADD_DEFINITIONS( -DLINUX_64 )
	ELSE( MAYA_ARCH_VERSION EQUAL 64 )
	  MAYA_32_BITS()
	ENDIF( MAYA_ARCH_VERSION EQUAL 64 )

	SET( CMAKE_CXX_COMPILER "g++" )
      ENDIF( MAYA_MAJOR_VERSION LESS 8 )

      SET( OS_LIBRARIES "" )

    ENDIF( APPLE )


  ENDIF( WIN32 )


ENDMACRO( MAYA_DEFINITIONS )


#
# A simple macro to create a maya standalone command.
#
# Usage is:
#
# CREATE_MAYA_STANDALONE(
#                      "imgconvert" "0.7" 
#                      "test.cpp hello.cpp" 
#                      "OpenMaya OpenMayaAnim"
#                     )
#
MACRO(CREATE_MAYA_STANDALONE NAME VERSION SOURCES LIBRARIES)


  MAYA_DEFINITIONS()

  SET( CREATE YES )

  IF( CMAKE_BUILD_ARCH EQUAL 64 )
    IF( NOT MAYA_ARCH_VERSION EQUAL 64 )
      MESSAGE( STATUS "# Skipping Maya standalone ${NAME} for 64-bit architecture #" )
      SET( CREATE NO )
    ENDIF( NOT MAYA_ARCH_VERSION EQUAL 64 )
  ENDIF( CMAKE_BUILD_ARCH EQUAL 64 )

  IF( CMAKE_BUILD_ARCH EQUAL 32 )
    IF( MAYA_ARCH_VERSION EQUAL 64 )
      MESSAGE( STATUS "# Skipping Maya standalone ${NAME} for 32-bit architecture #" )
      SET( CREATE NO )
    ENDIF( MAYA_ARCH_VERSION EQUAL 64 )
  ENDIF( CMAKE_BUILD_ARCH EQUAL 32 )

  IF ( CREATE )

    GET_FILENAME_COMPONENT( EXENAME "${NAME}" NAME )
    GET_FILENAME_COMPONENT( ANYPATH "${NAME}" PATH )

    SET( OUTPUT_DIR "maya${MAYA_VERSION}/${ANYPATH}" )

    FILE( MAKE_DIRECTORY 
      "${EXECUTABLE_OUTPUT_PATH}"
      "${EXECUTABLE_OUTPUT_PATH}/maya${MAYA_VERSION}"
      "${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_DIR}"
      )



    MESSAGE( STATUS "# Creating Maya${MAYA_VERSION} standalone ${EXENAME}" )


    ADD_EXECUTABLE( ${EXENAME} WIN32 ${SOURCES} )
    TARGET_LINK_LIBRARIES( ${EXENAME} ${MAYA_APP_LIBRARIES} ${LIBRARIES} )
    GET_TARGET_PROPERTY( OLD_FLAGS ${EXENAME} LINK_FLAGS )

    IF( NOT OLD_FLAGS )
      SET( OLD_FLAGS "" )
    ENDIF( NOT OLD_FLAGS )


    SET_TARGET_PROPERTIES( 
      "${EXENAME}"
      PROPERTIES 
      PREFIX     ""
      SUFFIX     "${MAYA_EXTENSION}"
      VERSION    "${VERSION}"
      SOVERSION  "${VERSION}"
      LINK_FLAGS "${MAYA_LINK_FLAGS} ${OLD_FLAGS}"
      RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/${OUTPUT_DIR}"
      )

    #
    # Install stuff correctly
    #
    INSTALL(TARGETS ${EXENAME} 
      RUNTIME DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      )

  ENDIF( CREATE )
ENDMACRO(CREATE_MAYA_STANDALONE)


#
# A simple macro to create a maya plugin.
#
# Usage is:
#
# CREATE_MAYA_PLUG_IN(
#                      "mrLiquid" "0.7" 
#                      "test.cpp hello.cpp" 
#                      "OpenMaya OpenMayaAnim"
#                     )
#
MACRO(CREATE_MAYA_PLUG_IN NAME VERSION SOURCES LIBRARIES)


  MAYA_DEFINITIONS()

  SET( CREATE YES )

  IF( CMAKE_BUILD_ARCH EQUAL 64 )
    IF( NOT MAYA_ARCH_VERSION EQUAL 64 )
      MESSAGE( STATUS "# Skipping Maya plug-in ${NAME} for 64-bit architecture #" )
      SET( CREATE NO )
    ENDIF( NOT MAYA_ARCH_VERSION EQUAL 64 )
  ENDIF( CMAKE_BUILD_ARCH EQUAL 64 )

  IF( CMAKE_BUILD_ARCH EQUAL 32 )
    IF( MAYA_ARCH_VERSION EQUAL 64 )
      MESSAGE( STATUS "# Skipping Maya plug-in ${NAME} for 32-bit architecture #" )
      SET( CREATE NO )
    ENDIF( MAYA_ARCH_VERSION EQUAL 64 )
  ENDIF( CMAKE_BUILD_ARCH EQUAL 32 )

  IF ( CREATE )

    GET_FILENAME_COMPONENT( PLUGNAME "${NAME}" NAME )
    GET_FILENAME_COMPONENT( ANYPATH  "${NAME}" PATH )

    SET( OUTPUT_DIR "maya${MAYA_VERSION}/${ANYPATH}" )

    FILE( MAKE_DIRECTORY 
      "${LIBRARY_OUTPUT_PATH}"
      "${LIBRARY_OUTPUT_PATH}/maya${MAYA_VERSION}"
      "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
      )



    MESSAGE( STATUS "# Creating Maya${MAYA_VERSION} plug-in ${PLUGNAME} in ${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}" )



    ADD_LIBRARY( ${PLUGNAME} SHARED ${SOURCES} )
    TARGET_LINK_LIBRARIES( ${PLUGNAME} ${LIBRARIES} )
    GET_TARGET_PROPERTY( OLD_FLAGS "${PLUGNAME}" LINK_FLAGS )

    IF( NOT OLD_FLAGS )
      SET( OLD_FLAGS "" )
    ENDIF( NOT OLD_FLAGS )


    SET_TARGET_PROPERTIES( 
      "${PLUGNAME}"
      PROPERTIES 
      PREFIX     ""
      SUFFIX     "${MAYA_EXTENSION}"
      VERSION    "${VERSION}"
      SOVERSION  "${VERSION}"
      LINK_FLAGS "${MAYA_LINK_FLAGS} ${OLD_FLAGS}"
      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
      )

    #
    # Remove symbols on release builds
    #
    IF( CMAKE_BUILD_TYPE STREQUAL "Release" OR
	CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" )

#       IF( UNIX )
# 	ADD_CUSTOM_COMMAND(
# 	  TARGET  "${PLUGNAME}" POST_BUILD
# 	  COMMAND strip
# 	  ARGS    -x 
# 	  "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}/${PLUGIN_NAME}${MAYA_EXTENSION}"
# 	  COMMENT "Stripping ${PLUGNAME} symbols"
# 	  )
#       ELSE( UNIX )
# 	# @todo: strip symbols on windoze
#       ENDIF( UNIX )


    ENDIF( CMAKE_BUILD_TYPE STREQUAL "Release" OR
      CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" )

    #
    # Install stuff correctly
    #
    INSTALL(TARGETS ${PLUGNAME} 
      LIBRARY DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      RUNTIME DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      )

  ENDIF( CREATE )
ENDMACRO(CREATE_MAYA_PLUG_IN)


#
# A simple macro to add a custom unix command on unix platforms to
# create a plugin.sog file.
# This file is needed on Unix platforms to force Maya to use RLTD_GLOBAL
# when loading the plug-in and expose the plug-in's symbols to other
# plug-ins or tools beside maya.
# For example, rubyMEL needs to expose all of ruby's symbols this way
# so that ruby extensions will work properly.
#
# Usage is:
#
# CREATE_MAYA_PLUG_IN_SOG(
#                      "mrLiquid"
#                     )
MACRO( CREATE_MAYA_PLUG_IN_SOG NAME )

    IF(UNIX)
      
      GET_FILENAME_COMPONENT( PLUGNAME "${NAME}" NAME )
      GET_FILENAME_COMPONENT( ANYPATH  "${NAME}" PATH )

      ADD_CUSTOM_COMMAND( 
	TARGET  "${PLUGNAME}"
	POST_BUILD
	COMMAND echo "This file is needed for Unix platforms.  "
	"Place in the same directory as ${PLUGNAME}.so." > 
	"${LIBRARY_OUTPUT_PATH}/maya${MAYA_VERSION}/${ANYPATH}/${PLUGNAME}.sog" 
	COMMENT "Create .sog file ${PLUGNAME}.sog"
	)

      #
      # @todo: Install .sog correctly
      #
      #       INSTALL(TARGETS "${PLUGNAME}.sog"
      # 	LIBRARY DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      # 	ARCHIVE DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      # 	)
    ENDIF(UNIX)

ENDMACRO( CREATE_MAYA_PLUG_IN_SOG )


#
# A simple macro to create an auxiliary maya library for
# other plug-ins or tools to use.
#
# Usage is:
#
# CREATE_MAYA_LIBRARY(
#                      "mrLiquid" "0.7" 
#                      "test.cpp hello.cpp" 
#                      "OpenMaya OpenMayaAnim"
#                     )
#
MACRO(CREATE_MAYA_LIBRARY NAME VERSION SOURCES LIBRARIES)

  MAYA_DEFINITIONS()

  SET( CREATE YES )

  IF( CMAKE_BUILD_ARCH EQUAL 64 )
    IF( NOT MAYA_ARCH_VERSION EQUAL 64 )
      MESSAGE( STATUS "# Skipping Maya library ${NAME} for 64-bit architecture #" )
      SET( CREATE NO )
    ENDIF( NOT MAYA_ARCH_VERSION EQUAL 64 )
  ENDIF( CMAKE_BUILD_ARCH EQUAL 64 )

  IF( CMAKE_BUILD_ARCH EQUAL 32 )
    IF( MAYA_ARCH_VERSION EQUAL 64 )
      MESSAGE( STATUS "# Skipping Maya library ${NAME} for 32-bit architecture #" )
      SET( CREATE NO )
    ENDIF( MAYA_ARCH_VERSION EQUAL 64 )
  ENDIF( CMAKE_BUILD_ARCH EQUAL 32 )

  IF ( CREATE )
    MESSAGE( STATUS "# Creating Maya${MAYA_VERSION} library ${NAME} in ${LIBRARY_OUTPUT_PATH} #" )

    GET_FILENAME_COMPONENT( PLUGNAME "${NAME}" NAME )
    GET_FILENAME_COMPONENT( ANYPATH  "${NAME}" PATH )

    SET( OUTPUT_DIR "maya${MAYA_VERSION}/${ANYPATH}" )
    FILE( MAKE_DIRECTORY 
      "${LIBRARY_OUTPUT_PATH}"
      "${LIBRARY_OUTPUT_PATH}/maya${MAYA_VERSION}"
      "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
      )

    IF(UNIX)
      SET( PLUGNAME "lib${PLUGNAME}" )
    ELSE(UNIX)
      SET( PLUGNAME "${PLUGNAME}" )
    ENDIF(UNIX)
    

    ADD_LIBRARY( ${PLUGNAME} SHARED ${SOURCES} )
    TARGET_LINK_LIBRARIES( ${PLUGNAME} ${LIBRARIES} )

    GET_TARGET_PROPERTY( OLD_FLAGS "${PLUGNAME}" LINK_FLAGS )

    IF( NOT OLD_FLAGS )
      SET( OLD_FLAGS "" )
    ENDIF( NOT OLD_FLAGS )
    
    SET_TARGET_PROPERTIES( 
      "${PLUGNAME}"
      PROPERTIES 
      PREFIX     ""
      VERSION    "${VERSION}"
      SOVERSION  "${VERSION}"
      LINK_FLAGS "${MAYA_LINK_FLAGS} ${OLD_FLAGS}"
      LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}" 
      RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
      ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
      )

    #
    # Install stuff correctly
    #
    INSTALL(TARGETS ${PLUGNAME} 
      LIBRARY DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      ARCHIVE DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      RUNTIME DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
      )
  ENDIF( CREATE )

ENDMACRO(CREATE_MAYA_LIBRARY)


IF(NOT MAYA_LOCATION)

  IF(NOT Maya_FIND_QUIETLY)
    IF(Maya_FIND_REQUIRED)
      MESSAGE( FATAL_ERROR "Environment variable MAYA_LOCATION not defined." )
    ELSE(Maya_FIND_REQUIRED)
      MESSAGE( WARNING ": Environment variable MAYA_LOCATION not defined." )
    ENDIF(Maya_FIND_REQUIRED)
  ENDIF(NOT Maya_FIND_QUIETLY)

ELSE(NOT MAYA_LOCATION)

  SET( MAYA_FOUND "YES" )

  #
  # EXTRACT maya version information from MAYA_LOCATION
  #
  STRING(REGEX REPLACE 
    "^.*[Mm]aya([0-9\\.x\\-]+).*$" 
    "\\1" 
    MAYA_VERSION 
    "${MAYA_LOCATION}" 
    )


  STRING(REGEX REPLACE 
    "^([^\\.]+).*" "\\1" 
    MAYA_MAJOR_VERSION "${MAYA_VERSION}" )

  STRING(REGEX REPLACE 
    "^${MAYA_MAJOR_VERSION}\\.([^\\.\\-])+.*" "\\1" 
    MAYA_MINOR_VERSION "${MAYA_VERSION}" 
    )

  STRING(REGEX REPLACE 
    "^.*-x([0-9]+)" "\\1" 
    MAYA_ARCH_VERSION "${MAYA_VERSION}" 
    )


  #
  # Open MTypes.h to extract MAYA_API_VERSION
  #
  FILE( READ "${MAYA_INCLUDE_DIR}/maya/MTypes.h" DUMMY )
  

  STRING(REGEX REPLACE 
    ".*#define[ \t]+MAYA_API_VERSION[ \t]+([0-9]+).*$" 
    "\\1" 
    MAYA_API_VERSION 
    "${DUMMY}" 
    )

#   #
#   # Maya8.0 and on ship with OpenEXR support built-in.
#   # We have to make sure we are linking against that library if we use
#   # OpenEXR.
#   #
#   IF( MAYA_MAJOR_VERSION GREATER 7 )
#     SET( OPENEXR_LIBRARY_DIR "${MAYA_LOCATION}/lib" )
#   ENDIF( MAYA_MAJOR_VERSION GREATER 7 )

ENDIF(NOT MAYA_LOCATION)

