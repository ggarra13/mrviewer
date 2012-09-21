#-*-cmake-*-
#
# Test for Industrial Light and Magic's Cg
#
# Options:
#
#  CG_ROOT         - root directory of Cg distribution.
#  CG_LIBRARY_DIR  - directory of Cg libraries (optional)
#  CG_BINARY_DIR   - directory of Cg binaries  (optional)
#
# Once loaded this will define:
#
#  CG_FOUND          - system has Cg
#  CG_INCLUDE_DIR    - include directory for Cg
#  CG_LIBRARIES      - Cg main libraries you need to link to
#
#  CGGL_FOUND        - system has CgGL
#  CGGL_INCLUDE_DIR  - include directory for CgGL
#  CGGL_LIBRARIES    - Cg OpenGL libraries 
#
#  CGC_COMPILER                 - cgc compiler (executable)
#  CGC_COMPILE(shader profiles) - macro to compile a .cg shader file
#                                 to one or more profiles.
#


SET(CG_FOUND "NO")
SET(CGGL_FOUND "NO")


IF( NOT CG_ROOT )
  SET( CG_ROOT $ENV{CG_ROOT} )
ENDIF( NOT CG_ROOT )




IF( CG_LIBRARY_DIR )
  SET( SEARCH_DIRS "${CG_LIBRARY_DIR}" )
ELSE( CG_LIBRARY_DIR )
  SET( SEARCH_DIRS 
    ${CG_ROOT}/lib${CMAKE_BUILD_ARCH}
    ${CG_ROOT}/lib
    /usr/lib/x86_64-linux-gnu
    ${CMAKE_SYSTEM_LIBRARY_PATH} )
ENDIF( CG_LIBRARY_DIR )

IF( CG_PROGRAM_DIR )
  SET( SEARCH_BIN_DIRS "${CG_LIBRARY_DIR}" )
ELSE( CG_PROGRAM_DIR )
  SET( SEARCH_BIN_DIRS 
    ${CG_ROOT}/bin${CMAKE_BUILD_ARCH}
    ${CG_ROOT}/bin
    ${CMAKE_SYSTEM_PROGRAM_PATH} )
ENDIF( CG_PROGRAM_DIR )



SET( Cg_NAMES       Cg    )
SET( CgGL_NAMES     CgGL  )



#
# Finally, we are ready to find the stuff...
#

#
# Try to locate the header file
#
FIND_PATH( CG_INCLUDE_DIR cg.h
  "${CG_ROOT}/include/Cg"
  "${CG_ROOT}/include"
  "${CG_ROOT}"
  /usr/local/include/Cg
  /usr/include/Cg
  DOC   "Cg includes"
  )

FIND_PATH( CGGL_INCLUDE_DIR cgGL.h
  "${CG_ROOT}/include/Cg"
  "${CG_ROOT}/include"
  "${CG_ROOT}"
  /usr/local/include/Cg
  /usr/include/Cg
  DOC   "CgGL includes"
  )


#
# Now find the libraries...
#

FIND_LIBRARY( Cg
  NAMES ${Cg_NAMES}
  PATHS ${SEARCH_DIRS}
  NO_DEFAULT_PATH
  DOC   "Cg library"
)

FIND_LIBRARY( CgGL
  NAMES ${CgGL_NAMES}
  PATHS ${SEARCH_DIRS}
  NO_DEFAULT_PATH
  DOC   "CgGL library"
)

FIND_FILE( CGC_COMPILER
  NAME  cgc cgc.exe
  PATHS ${SEARCH_BIN_DIRS}
  NO_DEFAULT_PATH
  DOC   "cgc compiler" )


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( Cg 
  DEFAULT_MSG
  CGC_COMPILER Cg CG_INCLUDE_DIR
  ) 

FIND_PACKAGE_HANDLE_STANDARD_ARGS( CgGL
  DEFAULT_MSG
  CgGL CGGL_INCLUDE_DIR
  ) 


SET( CG_LIBRARIES ${Cg} )
SET( CGGL_LIBRARIES ${CgGL} )


MACRO( CGC_COMPILE shader )

  SET( profiles )
  FOREACH(it ${ARGN})
    SET( profiles ${profiles} ${it} )
  ENDFOREACH(it)


  GET_DIRECTORY_PROPERTY(cmake_include_directories INCLUDE_DIRECTORIES)

  SET( cgc_include_dirs "-I${CMAKE_CURRENT_SOURCE_DIR}")
  FOREACH(it ${cmake_include_directories})
    SET(cgc_include_dirs ${cgc_include_dirs} "-I${it}")
  ENDFOREACH(it)

  SET( output_files )
  SET( outdir ${CMAKE_CGC_OUTPUT_PATH} )
  IF(NOT outdir )
    SET( outdir ${CMAKE_CURRENT_SOURCE_DIR} )
  ENDIF( NOT outdir )

  FOREACH( it ${profiles} )

    SET( output "${outdir}/${shader}.${it}" )
    SET( output_files ${output_files} ${output} )

    ADD_CUSTOM_COMMAND( 
      OUTPUT  "${output}"
      COMMAND "${CGC_COMPILER}" 
      ARGS -o "${output}" ${cgc_include_dirs} ${CMAKE_CGC_FLAGS}
           -profile ${it} "${shader}.cg"
      DEPENDS "${shader}.cg"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      COMMENT "Compiling ${shader} shader for profile ${it}"
      )

  ENDFOREACH( it )


  ADD_CUSTOM_TARGET( 
    "${shader}_cg" ALL
    DEPENDS ${output_files}
    )


ENDMACRO( CGC_COMPILE shader profiles )


MESSAGE( STATUS "CG_ROOT:      ${CG_ROOT}" )
MESSAGE( STATUS "Cg include:   ${CG_INCLUDE_DIR}")
MESSAGE( STATUS "Cg libraries: ${CG_LIBRARIES}")
MESSAGE( STATUS "CgGL include:   ${CG_INCLUDE_DIR}")
MESSAGE( STATUS "CgGL libraries: ${CG_LIBRARIES}")
