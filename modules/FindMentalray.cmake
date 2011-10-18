#-*-cmake-*-
#
# Test for mentalray in either maya or stand-alone locations.
#
# Once loaded this will define
#  MENTALRAY_FOUND        - system has mental ray
#  MENTALRAY_VERSION      - mental ray version being used (taken
#                           from shader.h)
#  MENTALRAY_INCLUDE_DIR  - include directory for mental ray
#  MENTALRAY_LIBRARY_DIR  - library directory for mental ray
#  MENTALRAY_LIBRARIES    - libraries you need to link to
#  MENTALRAY_LINK_FLAGS   - mental ray flags passed to the linker
#
# We also define two simple macros called CREATE_MENTALRAY_SHADER()
# and a CREATE_MENTALRAY_LIBRARY().
#

FIND_PACKAGE( Maya )


SET(MENTALRAY_FOUND "NO")


FIND_PATH( MENTALRAY_LOCATION include/shader.h
  "${MAYA_LOCATION}/devkit/mentalray"
  "$ENV{MENTALRAY_LOCATION}"
  "$ENV{MI_ROOT}"
  )

SET( MENTALRAY_INCLUDE_DIR "${MENTALRAY_LOCATION}/include" )

IF(WIN32)

  ADD_DEFINITIONS( -nologo -DWIN_NT -DEVIL_ENDIAN -D_WIN32_WINNT=0x0400 -DX86 -DWIN32 )
  SET( MENTALRAY_LINK_FLAGS "-nologo -stack:0x200000,0x1000" )
  SET( MENTALRAY_LIBRARIES shader.lib ws2_32.lib user32.lib gdi32.lib )

ELSE(WIN32)


  IF( APPLE )

    ADD_DEFINITIONS( -DOSX -DMACOSX )
    SET( MENTALRAY_LINK_FLAGS "-flat_namespace -undefined suppress -dynamic" )

  ELSE( APPLE )

    ADD_DEFINITIONS( -ftemplate-depth-25 -fexpensive-optimizations -finline-functions -funroll-loops -fomit-frame-pointer -frerun-cse-after-loop -fstrength-reduce -fforce-addr   -DLINUX -DLINUX_X86 -DX86 -DBIT64 -DEVIL_ENDIAN -D_GNU_SOURCE -D_THREAD_SAVE -D_REENTRANT -DSYSV -DSVR4 -Dinline=__inline__ -DSSE_INTRINSICS )
    SET( MENTALRAY_LINK_FLAGS "-export-dynamic --whole-archive -Bsymbolic" )


    IF( CMAKE_BUILD_ARCH EQUAL 32 )
      ADD_DEFINITIONS( "-m32 -I/usr/include/i486-linux-gnu" )
      SET( MENTALRAY_LINK_FLAGS  "-m32 -L/lib32 -L/usr/lib32 ${MENTALRAY_LINK_FLAGS}" )
    ENDIF( CMAKE_BUILD_ARCH EQUAL 32 )

  ENDIF( APPLE )
ENDIF(WIN32)



IF (MENTALRAY_INCLUDE_DIR)
  #
  # Extract mentalray version.  Currently, shader.h does not contain
  # version information, so we just use the '#define RAY35' lines as version info
  #
  FILE( READ "${MENTALRAY_INCLUDE_DIR}/shader.h" SHADER_H )
  STRING( REGEX MATCHALL "#define RAY[0-9]+" MENTALRAY_VERSIONS "${SHADER_H}" )
  STRING( REGEX REPLACE "#define RAY([0-9]+)" "\\1" MENTALRAY_VERSIONS "${MENTALRAY_VERSIONS}" )
  LIST(GET MENTALRAY_VERSIONS -1 MENTALRAY_VERSION )

  ###
  FILE( TO_CMAKE_PATH "${MENTALRAY_INCLUDE_DIR}/../lib"  MENTALRAY_LIBRARY_DIR )

  IF(WIN32)
    FILE( TO_CMAKE_PATH "${MENTALRAY_INCLUDE_DIR}/../lib/nt" 
      MENTALRAY_NT_LIBRARY_DIR )
    SET( MENTALRAY_LIBRARY_DIR 
      ${MENTALRAY_NT_LIBRARY_DIR} 
      ${MENTALRAY_LIBRARY_DIR} )
  ENDIF(WIN32)

  SET( MENTALRAY_LIBRARY_DIR ${MENTALRAY_LIBRARY_DIR}
    "${LIBRARY_OUTPUT_PATH}/mentalray${MENTALRAY_VERSION}" 
    )
  SET(MENTALRAY_FOUND "YES")
ENDIF(MENTALRAY_INCLUDE_DIR)

IF(NOT MENTALRAY_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT Mentalray_FIND_QUIETLY)
    IF(Mentalray_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "mentalray required, please specify its location with MENTALRAY_LOCATION or MI_ROOT.")
    ELSE(Mentalray_FIND_REQUIRED)
      MESSAGE(STATUS "mentalray was not found.")
    ENDIF(Mentalray_FIND_REQUIRED)
  ENDIF(NOT Mentalray_FIND_QUIETLY)
ENDIF(NOT MENTALRAY_FOUND)

#
# Set -DMR_BIG_ENDIAN if appropiate
#
INCLUDE(TestBigEndian)

TEST_BIG_ENDIAN( BIG_ENDIAN )
IF( BIG_ENDIAN )
  ADD_DEFINITIONS( -DMR_BIG_ENDIAN )
ENDIF( BIG_ENDIAN )

#####


#
# A simple macro to create a mental ray shader.
#
# Usage is:
#
# CREATE_MENTALRAY_SHADER(
#                      "exr_shader" "0.7" 
#                      "test.cpp hello.cpp" 
#                      "shader.lib IlmImf"
#                      ""
#                     )
#
MACRO(CREATE_MENTALRAY_SHADER 
    NAME VERSION SOURCES LIBRARIES DEPENDENCIES)

  GET_FILENAME_COMPONENT( SHADERNAME "${NAME}" NAME )
  GET_FILENAME_COMPONENT( ANYPATH "${NAME}" PATH )

  SET( OUTPUT_DIR "mentalray${MENTALRAY_VERSION}/${ANYPATH}" )

  FILE( MAKE_DIRECTORY 
    "${LIBRARY_OUTPUT_PATH}"
    "${LIBRARY_OUTPUT_PATH}/mentalray${MENTALRAY_VERSION}"
    "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
    )



  ADD_LIBRARY( "${SHADERNAME}" SHARED ${SOURCES} )

  SET( LIBS ${LIBRARIES} ${MENTALRAY_LIBRARIES} )
  IF( LIBS )
    TARGET_LINK_LIBRARIES( ${SHADERNAME} ${LIBS} )
  ENDIF( LIBS )

  #
  # Libraries are created without "lib" prefix
  #
  GET_TARGET_PROPERTY( OLD_FLAGS "${SHADERNAME}" LINK_FLAGS )
  IF( NOT OLD_FLAGS )
    SET( OLD_FLAGS "" )
  ENDIF( NOT OLD_FLAGS )

  MESSAGE( STATUS 
    "# Creating mentalray${MENTALRAY_VERSION} shader '${SHADERNAME}'" )

  SET_TARGET_PROPERTIES( 
    "${SHADERNAME}"
    PROPERTIES 
    PREFIX     "" 
    VERSION    "${VERSION}"
    SOVERSION  "${VERSION}"
    LINK_FLAGS "${MENTALRAY_LINK_FLAGS} ${OLD_FLAGS}"
    LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}" 
    RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
  )

  IF( DEPENDENCIES )
    MESSAGE( STATUS "#          with dependencies ${DEPENDENCIES}" )
    ADD_DEPENDENCIES( "${SHADERNAME}" "${DEPENDENCIES}" )
  ENDIF( DEPENDENCIES )

  #
  # Install shader
  #
  INSTALL(TARGETS ${SHADERNAME} 
    LIBRARY DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
    RUNTIME DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
    )
ENDMACRO( CREATE_MENTALRAY_SHADER )




#
# A simple macro to create a mental ray auxiliary shader library.
#
# Usage is:
#
# CREATE_MENTALRAY_LIBRARY(
#                      "mrLibrary" "0.7" 
#                      "test.cpp hello.cpp" 
#                      "shader.lib IlmImf"
#                     )
#
MACRO(CREATE_MENTALRAY_LIBRARY NAME VERSION SOURCES LIBRARIES)

  GET_FILENAME_COMPONENT( LIBNAME "${NAME}" NAME )
  GET_FILENAME_COMPONENT( ANYPATH "${NAME}" PATH )

  SET( OUTPUT_DIR "mentalray${MENTALRAY_VERSION}/${ANYPATH}" )
  FILE( MAKE_DIRECTORY 
    "${LIBRARY_OUTPUT_PATH}"
    "${LIBRARY_OUTPUT_PATH}/mentalray${MENTALRAY_VERSION}"
    "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
    )

  IF(UNIX)
    SET( LIBNAME "lib${LIBNAME}" )
  ELSE(UNIX)
    SET( LIBNAME "${LIBNAME}" )
  ENDIF(UNIX)


  MESSAGE( STATUS 
    "# Creating mentalray${MENTAL_RAY_VERSION} shader library '${LIBNAME}'" )
  ADD_LIBRARY( ${LIBNAME} SHARED ${SOURCES} )


  SET( LIBS ${LIBRARIES} ${MENTALRAY_LIBRARIES} )
  IF( LIBS )
    TARGET_LINK_LIBRARIES( ${LIBNAME} ${LIBS} )
  ENDIF( LIBS )


  #
  # Libraries are created with "lib" prefix
  #
  GET_TARGET_PROPERTY( OLD_FLAGS "${LIBNAME}" LINK_FLAGS )

  IF( NOT OLD_FLAGS )
    SET( OLD_FLAGS "" )
  ENDIF( NOT OLD_FLAGS )

  SET_TARGET_PROPERTIES( 
    "${LIBNAME}"
    PROPERTIES 
    PREFIX     ""
    VERSION    "${VERSION}"
    SOVERSION  "${VERSION}"
    LINK_FLAGS "${MENTALRAY_LINK_FLAGS} ${OLD_FLAGS}"
    LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_PATH}/${OUTPUT_DIR}" 
  )

  #
  # Install library
  #
  INSTALL(TARGETS ${LIBNAME} 
    LIBRARY DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
    ARCHIVE DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
    RUNTIME DESTINATION "${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_ARCH}/lib/${OUTPUT_DIR}"
    )

ENDMACRO( CREATE_MENTALRAY_LIBRARY )


MARK_AS_ADVANCED(
  MENTALRAY_INCLUDE_DIR 
)
