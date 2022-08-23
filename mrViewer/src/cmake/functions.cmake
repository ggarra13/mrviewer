#
# This file contains utility functions for the build
#
function (FLTK_RUN_FLUID TARGET SOURCES)
  set (CXX_FILES)
  foreach (src ${SOURCES})
    if ("${src}" MATCHES "\\.fl$")
      string(REGEX REPLACE "(.*/)?(.*).fl" \\2 basename ${src})
      add_custom_command(
	OUTPUT "${basename}.cxx" "${basename}.h"
	COMMAND ${FLTK_FLUID_EXECUTABLE} -c ${CMAKE_CURRENT_SOURCE_DIR}/${src}
	DEPENDS ${src}
	MAIN_DEPENDENCY ${src}
      )
      list (APPEND CXX_FILES "${CMAKE_CURRENT_BINARY_DIR}/${basename}.cxx")
    endif ("${src}" MATCHES "\\.fl$")
  endforeach ()
  set (${TARGET} ${CXX_FILES} PARENT_SCOPE)
endfunction (FLTK_RUN_FLUID TARGET SOURCES)

macro(DEBUG_LIBRARIES)
  foreach (src ${ARGV})
    message( STATUS ${src} )
  endforeach ()
endmacro (DEBUG_LIBRARIES SOURCES)
