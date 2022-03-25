#-*-cmake-*-
#
# Test for TCLAP (Templated Command Line Argument Parser)
#
# Once loaded this will define
#  TCLAP_FOUND        - system has TCLAP
#  TCLAP_INCLUDE_DIR  - include directory for TCLAP
#

SET(TCLAP_FOUND "NO")


FIND_PATH( TCLAP_INCLUDE_DIR tclap/CmdLine.h
  "$ENV{TCLAP_ROOT}/include"
  "$ENV{TCLAP_ROOT}"
  /usr/local/include
  /usr/include
  DOC   "TCLAP includes"
  )

IF(NOT TCLAP_FOUND)
  IF (TCLAP_INCLUDE_DIR)
    SET(TCLAP_FOUND "YES")
  ENDIF(TCLAP_INCLUDE_DIR)
ENDIF(NOT TCLAP_FOUND)

IF(NOT TCLAP_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT TCLAP_FIND_QUIETLY)
    IF(TCLAP_FIND_REQUIRED)
      MESSAGE( STATUS "TCLAP_INCLUDE_DIR ${TCLAP_INCLUDE_DIR}" )
      MESSAGE(FATAL_ERROR
              "TCLAP required, please specify its location with TCLAP_ROOT.")
    ELSE(TCLAP_FIND_REQUIRED)
      MESSAGE(STATUS "TCLAP was not found.")
    ENDIF(TCLAP_FIND_REQUIRED)
  ENDIF(NOT TCLAP_FIND_QUIETLY)
ENDIF(NOT TCLAP_FOUND)

#####

