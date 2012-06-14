#-*-cmake-*-
#
# Test for CTL (Color Transform Language)
#
# Once loaded this will define
#  CTL_FOUND        - system has OpenEXR
#  CTL_INCLUDE_DIR  - include directory for OpenEXR
#  CTL_LIBRARIES    - libraries you need to link to
#

SET(CTL_FOUND "NO")

IF( CTL_LIBRARY_DIR )
  SET( SEARCH_DIRS "${CTL_LIBRARY_DIR}" )
ELSE( CTL_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    "$ENV{CTL_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Release"
    "$ENV{CTL_ROOT}/lib/Release"
    "$ENV{CTL_ROOT}/lib"
    "$ENV{CTL_ROOT}/lib/x${CMAKE_BUILD_ARCH}/Debug"
    "$ENV{CTL_ROOT}/lib/Debug"
    "$ENV{CTL_ROOT}/bin/x${CMAKE_BUILD_ARCH}/Release"
    "$ENV{CTL_ROOT}/bin/Release"
    "$ENV{CTL_ROOT}/bin/x${CMAKE_BUILD_ARCH}/Debug"
    "$ENV{CTL_ROOT}/bin/Debug"
    "$ENV{CTL_ROOT}/bin"
    /usr/local/lib${CMAKE_BUILD_ARCH}
    /usr/lib${CMAKE_BUILD_ARCH}
    )
ENDIF( CTL_LIBRARY_DIR )


FIND_PATH( CTL_INCLUDE_DIR CtlModule.h
  "$ENV{CTL_ROOT}/include/CTL"
  "$ENV{CTL_ROOT}/include"
  /usr/local/include/CTL
  /usr/include/CTL
  DOC   "CTL includes"
  )

FIND_LIBRARY( IlmCtl
  NAMES IlmCtl_dll IlmCtl_dll_d IlmCtl IlmCtld libIlmCtl 
  PATHS ${SEARCH_DIRS}
  DOC   "CTL IlmCtl library"
)

FIND_LIBRARY( IlmCtlMath
  NAMES IlmCtlMath_dll IlmCtlMath_dll_d IlmCtlMath IlmCtlMathd libIlmCtlMath
  PATHS ${SEARCH_DIRS}
  DOC   "CTL IlmCtlMath library"
)

FIND_LIBRARY( IlmCtlSimd
  NAMES IlmCtlSimd_dll IlmCtlSimd_dll_d IlmCtlSimd IlmCtlSimdd libIlmCtlSimd 
  PATHS ${SEARCH_DIRS}
  DOC   "CTL IlmCtlSimd library"
)


SET(CTL_LIBRARIES ${IlmCtlSimd} ${IlmCtlMath} ${IlmCtl} )

IF(WIN32 OR WIN64)
  ADD_DEFINITIONS( "-DCTL_DLL" )
ENDIF(WIN32 OR WIN64)


IF(NOT CTL_FOUND)
  IF (CTL_INCLUDE_DIR)
    IF(CTL_LIBRARIES)
      SET(CTL_FOUND "YES")
    ENDIF(CTL_LIBRARIES)
  ENDIF(CTL_INCLUDE_DIR)
ENDIF(NOT CTL_FOUND)

IF(NOT CTL_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT CTL_FIND_QUIETLY)
    IF(CTL_FIND_REQUIRED)
      MESSAGE( STATUS "CTL_INCLUDE_DIR ${CTL_INCLUDE_DIR}" )
      MESSAGE( STATUS "CTL_LIBRARIES   ${CTL_LIBRARIES}" )
      MESSAGE(FATAL_ERROR
              "CTL required, please specify its location with CTL_ROOT.")
    ELSE(CTL_FIND_REQUIRED)
      MESSAGE(STATUS "CTL was not found.")
    ENDIF(CTL_FIND_REQUIRED)
  ENDIF(NOT CTL_FIND_QUIETLY)
ENDIF(NOT CTL_FOUND)

#####

