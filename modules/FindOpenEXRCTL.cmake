#-*-cmake-*-
#
# Test for CTL (Color Transform Language)
#
# Once loaded this will define
#  OpenEXRCTL_FOUND        - system has OpenEXR
#  OpenEXRCTL_INCLUDE_DIR  - include directory for OpenEXR
#  OpenEXRCTL_LIBRARIES    - libraries you need to link to
#

SET(OpenEXRCTL_FOUND "NO")

IF( OpenEXRCTL_LIBRARY_DIR )
  SET( SEARCH_DIRS "${OpenEXRCTL_LIBRARY_DIR}" )
ELSE( OpenEXRCTL_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{OpenEXRCTL_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Release"
    "$ENV{OpenEXRCTL_ROOT}/lib/Release"
    "$ENV{OpenEXRCTL_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Debug"
    "$ENV{OpenEXRCTL_ROOT}/lib/Debug"
    "$ENV{OpenEXRCTL_ROOT}/lib"
    "$ENV{CTL_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Release"
    "$ENV{CTL_ROOT}/lib/Release"
    "$ENV{CTL_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Debug"
    "$ENV{CTL_ROOT}/lib/Debug"
    "$ENV{CTL_ROOT}/lib"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/lib${CMAKE_BUILD_ARCH}
    )
ENDIF( OpenEXRCTL_LIBRARY_DIR )


FIND_PATH( OpenEXRCTL_INCLUDE_DIR ImfCtlApplyTransforms.h
  "$ENV{OpenEXRCTL_ROOT}/include/OpenEXR"
  "$ENV{OpenEXRCTL_ROOT}/include"
  "$ENV{OpenEXRCTL_ROOT}"
  "$ENV{CTL_ROOT}/include/OpenEXR"
  "$ENV{CTL_ROOT}/include"
  "$ENV{CTL_ROOT}"
  /usr/local/include/OpenEXR
  /usr/include/OpenEXR
  DOC   "OpenEXRCTL includes"
  )

FIND_LIBRARY( IlmImfCtl
  NAMES IlmImfCtl_dll IlmImfCtl_dll_d IlmImfCtl IlmImfCtld libIlmImfCtl 
  PATHS ${SEARCH_DIRS}
  DOC   "OpenEXRCTL IlmImfCtl library"
)

FIND_LIBRARY( IlmImfCtlSimd
  NAMES IlmImfCtlSimd_dll IlmImfCtlSimd_dll_d IlmImfCtlSimd IlmImfCtlSimdd libIlmImfCtlSimd 
  PATHS ${SEARCH_DIRS}
  DOC   "OpenEXRCTL IlmImfCtlSimd library"
)

MESSAGE( ${SEARCH_DIRS} )

SET(OpenEXRCTL_LIBRARIES ${IlmImfCtl} )


IF(NOT OpenEXRCTL_FOUND)
  IF (OpenEXRCTL_INCLUDE_DIR)
    IF(OpenEXRCTL_LIBRARIES)
      SET(OpenEXRCTL_FOUND "YES")
    ENDIF(OpenEXRCTL_LIBRARIES)
  ENDIF(OpenEXRCTL_INCLUDE_DIR)
ENDIF(NOT OpenEXRCTL_FOUND)

IF(NOT OpenEXRCTL_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT OpenEXRCTL_FIND_QUIETLY)
    IF(OpenEXRCTL_FIND_REQUIRED)
      MESSAGE( STATUS "OpenEXRCTL_INCLUDE_DIR ${OpenEXRCTL_INCLUDE_DIR}" )
      MESSAGE( STATUS "OpenEXRCTL_LIBRARIES ${OpenEXRCTL_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "OpenEXRCTL required, please specify its location with OpenEXRCTL_ROOT.")
    ELSE(OpenEXRCTL_FIND_REQUIRED)
      MESSAGE(STATUS "OpenEXRCTL was not found.")
    ENDIF(OpenEXRCTL_FIND_REQUIRED)
  ENDIF(NOT OpenEXRCTL_FIND_QUIETLY)
ENDIF(NOT OpenEXRCTL_FOUND)

#####

