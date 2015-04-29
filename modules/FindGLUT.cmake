# - try to find glut library and include files
#  GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#  GLUT_LIBRARIES, the libraries to link against
#  GLUT_FOUND, If false, do not try to use GLUT.
# Also defined, but not for general use are:
#  GLUT_glut_LIBRARY = the full path to the glut library.
#  GLUT_Xmu_LIBRARY  = the full path to the Xmu library.
#  GLUT_Xi_LIBRARY   = the full path to the Xi Library.

IF (WIN32)

   IF(NOT GLUT_ROOT_PATH)
     SET(GLUT_ROOT_PATH $ENV{GLUT_ROOT_PATH} )
   ENDIF( NOT GLUT_ROOT_PATH)


  IF(CYGWIN)

    FIND_PATH( GLUT_INCLUDE_DIR GL/glut.h
      ${GLUT_ROOT_PATH}/include
      /usr/include
    )

    FIND_LIBRARY( GLUT_glut_LIBRARY glut32
      ${OPENGL_LIBRARY_DIR}
      ${GLUT_ROOT_PATH}/lib
      ${GLUT_ROOT_PATH}/Release
      /usr/local/lib${CMAKE_BUILD_ARCH}
      /usr/lib
      /usr/lib/w32api
      /usr/X11R6/lib
    )

  ELSE(CYGWIN)

    FIND_PATH( GLUT_INCLUDE_DIR GL/glut.h
      ${GLUT_ROOT_PATH}/include
      ${GLUT_ROOT_PATH}
    )

    SET(SEARCH_PATHS 
      ${GLUT_ROOT_PATH}/lib/x${CMAKE_BUILD_ARCH}
      ${GLUT_ROOT_PATH}/lib
      ${OPENGL_LIBRARY_DIR} )

    FIND_LIBRARY( GLUT_glut_LIBRARY
      NAMES glut32 freeglut_static freeglut glut
      PATHS ${SEARCH_PATHS}
    )


  ENDIF(CYGWIN)

ELSE (WIN32)

  IF (APPLE)
# These values for Apple could probably do with improvement.
    FIND_PATH( GLUT_INCLUDE_DIR glut.h
      /System/Library/Frameworks/GLUT.framework/Versions/A/Headers
      ${OPENGL_LIBRARY_DIR}
    )
    SET(GLUT_glut_LIBRARY "-framework GLUT" CACHE STRING "GLUT library for OSX") 
    SET(GLUT_cocoa_LIBRARY "-framework Cocoa" CACHE STRING "Cocoa framework for OSX")
  ELSE (APPLE)

    FIND_PATH( GLUT_INCLUDE_DIR GL/glut.h
      ${GLUT_ROOT_PATH}/include
      /usr/include
      /usr/include/GL
      /usr/local/include
      /usr/openwin/share/include
      /usr/openwin/include
      /usr/X11R6/include
      /usr/include/X11
      /opt/graphics/OpenGL/include
      /opt/graphics/OpenGL/contrib/libglut
    )

    FIND_LIBRARY( GLUT_glut_LIBRARY glut
      /usr/local/lib${CMAKE_BUILD_ARCH}
      /usr/lib
      /usr/local/lib
      /usr/openwin/lib
      /usr/X11R6/lib
      /usr/lib/x86_64-linux-gnu
    )

    FIND_LIBRARY( GLUT_Xi_LIBRARY Xi
      /usr/local/lib${CMAKE_BUILD_ARCH}
      /usr/lib
      /usr/local/lib
      /usr/openwin/lib
      /usr/X11R6/lib
      /usr/lib/x86_64-linux-gnu
    )

    FIND_LIBRARY( GLUT_Xmu_LIBRARY Xmu
      /usr/lib
      /usr/local/lib
      /usr/openwin/lib
      /usr/X11R6/lib
      /usr/lib/x86_64-linux-gnu
    )

  ENDIF (APPLE)

ENDIF (WIN32)

SET( GLUT_FOUND "NO" )
IF(GLUT_INCLUDE_DIR)
  IF(GLUT_glut_LIBRARY)

    SET( GLUT_LIBRARIES
      ${GLUT_glut_LIBRARY}
      ${GLUT_Xmu_LIBRARY}
      ${GLUT_Xi_LIBRARY} 
      ${GLUT_cocoa_LIBRARY}
    )
    SET( GLUT_FOUND "YES" )

#The following deprecated settings are for backwards compatibility with CMake1.4
    SET (GLUT_LIBRARY ${GLUT_LIBRARIES})
    SET (GLUT_INCLUDE_PATH ${GLUT_INCLUDE_DIR})


  ENDIF(GLUT_glut_LIBRARY)
ENDIF(GLUT_INCLUDE_DIR)




IF(NOT GLUT_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT GLUT_FIND_QUIETLY)
    IF(GLUT_FIND_REQUIRED)
      MESSAGE( STATUS "GLUT_INCLUDE_DIR=" ${GLUT_INCLUDE_DIR} )
      MESSAGE( STATUS "GLUT_LIBRARIES=" ${GLUT_LIBRARIES} )
      MESSAGE(FATAL_ERROR
              "GLUT required, please specify its location with GLUT_ROOT_PATH.")
    ELSE(GLUT_FIND_REQUIRED)
      MESSAGE( STATUS "GLUT was not found!!! " ${GLUT_INCLUDE_DIR})
      MESSAGE( STATUS ${GLUT_LIBRARIES} )
    ENDIF(GLUT_FIND_REQUIRED)
  ENDIF(NOT GLUT_FIND_QUIETLY)
ENDIF(NOT GLUT_FOUND)

MARK_AS_ADVANCED(
  GLUT_INCLUDE_DIR
  GLUT_glut_LIBRARY
  GLUT_Xmu_LIBRARY
  GLUT_Xi_LIBRARY
)
