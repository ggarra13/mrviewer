#-*-cmake-*-
#
# Test for BlackMagicRAW
#
# Once loaded this will define
#  BlackMagicRAW_FOUND        - system has BlackMagicRAW
#  BlackMagicRAW_INCLUDE_DIR  - include directory for BlackMagicRAW
#

include(MacroAddInterfaces)

SET(BlackMagicRAW_FOUND "NO")

IF(APPLE)
   set( platform Mac )
ELSEIF( UNIX )
   set( platform Linux )
ELSE() # Win64
   set( platform Win )
ENDIF()

MESSAGE( STATUS "../../../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/${platform}/Include" )

FIND_PATH( BlackMagicRAW_INCLUDE_DIR
	   NAMES BlackmagicRawAPI.h BlackmagicRawAPI.idl
	   PATHS
	     "$ENV{BlackMagicRAW_ROOT}/Include"
	     "$ENV{BlackMagicRAW_ROOT}"
	     "../../../Blackmagic RAW/BlackmagicRAW/BlackmagicRAWSDK/${platform}/Include"
	     "/Applications/Blackmagic RAW/Blackmagic RAW SDK/${platform}/Include"
  DOC   "BlackMagicRAW includes"
  )

set( BlackMagicRAW_SOURCES ${BlackMagicRAW_INCLUDE_DIR}/BlackmagicRawAPIDispatch.cpp )

get_filename_component( BlackMagicRAW_ROOT ${BlackMagicRAW_INCLUDE_DIR} DIRECTORY )
IF(APPLE)
ELSEIF(UNIX)
  FIND_LIBRARY( BlackMagicRAW_LIBRARIES
		NAMES BlackmagicRawAPI
		PATHS
		${BlackMagicRAW_ROOT}/Libraries/ )
ELSEIF(WIN32)
  set( BlackMagicRAW_LIBRARIES comsuppw.lib )
ENDIF()

set(PROJECT_IDL_FILES ${BlackMagicRAW_INCLUDE_DIR}/BlackmagicRawAPI.idl )

MACRO_ADD_INTERFACES(GENERATED_FILES_IDL ${PROJECT_IDL_FILES})

SOURCE_GROUP("IDL" FILES ${GENERATED_FILES_IDL} ${PROJECT_IDL_FILES})

IF(NOT BlackMagicRAW_FOUND)
  IF (BlackMagicRAW_INCLUDE_DIR)
    SET(BlackMagicRAW_FOUND "YES")
  ENDIF(BlackMagicRAW_INCLUDE_DIR)
ENDIF(NOT BlackMagicRAW_FOUND)

IF(NOT BlackMagicRAW_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT BlackMagicRAW_FIND_QUIETLY)
    IF(BlackMagicRAW_FIND_REQUIRED)
      MESSAGE( STATUS "BlackMagicRAW_INCLUDE_DIR ${BlackMagicRAW_INCLUDE_DIR}" )
      MESSAGE(FATAL_ERROR
	      "BlackMagicRAW required, please specify its location with BlackMagicRAW_ROOT.")
    ELSE(BlackMagicRAW_FIND_REQUIRED)
      MESSAGE(STATUS "BlackMagicRAW was not found.")
    ENDIF(BlackMagicRAW_FIND_REQUIRED)
  ENDIF(NOT BlackMagicRAW_FIND_QUIETLY)
ENDIF(NOT BlackMagicRAW_FOUND)

#####
