
# if(COMMAND cmake_policy)
#   cmake_policy(SET CMP0003 NEW)
# endif(COMMAND cmake_policy)

FIND_PACKAGE( OpenEXR REQUIRED )
FIND_PACKAGE( CTL REQUIRED )

MESSAGE( "CTL LIBRARIES " ${CTL_LIBRARIES} )

INCLUDE_DIRECTORIES( ${CTL_INCLUDE_DIR} ${OPENEXR_INCLUDE_DIR} )

IF(WIN32)
SET( LIBRARIES 
  ${CTL_LIBRARIES} 
  ${OPENEXR_LIBRARIES} 
)
ELSE(WIN32)
SET( LIBRARIES 
  ${CTL_LIBRARIES} 
  ${OPENEXR_LIBRARIES} 
  pthread 
  )
#ADD_DEFINITIONS( --std=c++98 -D_GLIBCXX_USE_CXX11_ABI=0 )
ENDIF()


MESSAGE( STATUS "CTL_INCLUDE_DIR=${CTL_INCLUDE_DIR}" )
MESSAGE( STATUS "OPENEXR_INCLUDE_DIR=${OPENEXR_INCLUDE_DIR}" )
MESSAGE( STATUS "LIBRARIES=${LIBRARIES}" )

  EXECUTE_PROCESS( COMMAND xgettext -s -c++ -k_ main.cpp -o po/messages.pot 
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE ok_xgettext
    ERROR_VARIABLE  error_xgettext)


ADD_EXECUTABLE( ctlparser main.cpp )
TARGET_LINK_LIBRARIES( ctlparser ${LIBRARIES} )
