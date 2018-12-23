# - Find Ruby
# This module finds if Ruby is installed and determines where the include files
# and libraries are. It also determines what the name of the library is. This
# code sets the following variables:
#
#  RUBY_INCLUDE_PATH = path to where ruby.h can be found
#  RUBY_EXECUTABLE   = full path to the ruby binary

# Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
# See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

SET( RUBY_FOUND "NO" )

if(RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
   # Already in cache, be silent
   set(RUBY_FIND_QUIETLY TRUE)
endif (RUBY_LIBRARY AND RUBY_INCLUDE_PATH)

#   RUBY_ARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"archdir"@:>@)'`
#   RUBY_SITEARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitearchdir"@:>@)'`
#   RUBY_SITEDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitelibdir"@:>@)'`
#   RUBY_LIBDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"libdir"@:>@)'`
#   RUBY_LIBRUBYARG=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"LIBRUBYARG_SHARED"@:>@)'`

FIND_PROGRAM(RUBY_EXECUTABLE NAMES ruby ruby1.8 ruby18
  "${RUBY_ROOT}/bin"
  /usr/local/bin
  "$ENV{PROGRAMFILES}/ruby/bin"
  C:/ruby/bin
  )

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "puts Config::CONFIG['archdir']"
   OUTPUT_VARIABLE RUBY_ARCH_DIR)


EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "puts Config::CONFIG['libdir']"
   OUTPUT_VARIABLE RUBY_POSSIBLE_LIB_PATH)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "puts Config::CONFIG['rubylibdir']"
   OUTPUT_VARIABLE RUBY_RUBY_LIB_PATH)

# remove the new lines from the output by replacing them with empty strings
STRING(REPLACE "\n" "" RUBY_ARCH_DIR "${RUBY_ARCH_DIR}")
STRING(REPLACE "\n" "" RUBY_POSSIBLE_LIB_PATH "${RUBY_POSSIBLE_LIB_PATH}")
STRING(REPLACE "\n" "" RUBY_RUBY_LIB_PATH "${RUBY_RUBY_LIB_PATH}")

FIND_PATH(RUBY_INCLUDE_PATH
  NAMES ruby.h
  PATHS ${RUBY_ARCH_DIR} )

FIND_LIBRARY(RUBY_LIBRARY
  NAMES ruby ruby1.8 ruby1.9 
        msvcrt-ruby18 msvcrt-ruby19 msvcrt-ruby18-static msvcrt-ruby19-static
  PATHS ${RUBY_POSSIBLE_LIB_PATH}
  )

IF(NOT RUBY_FOUND)
  IF (RUBY_INCLUDE_PATH)
    IF(RUBY_LIBRARY)
      SET(RUBY_FOUND "YES")
      IF( NOT RUBY_LIBRARY_DIR )
	GET_FILENAME_COMPONENT(RUBY_LIBRARY_DIR "${RUBY_LIBRARY}" PATH)
      ENDIF( NOT RUBY_LIBRARY_DIR )
      IF(WIN32)
	ADD_DEFINITIONS( "-DRUBY_DLL" )
      ENDIF(WIN32)
    ENDIF(RUBY_LIBRARY)
  ENDIF(RUBY_INCLUDE_PATH)
ENDIF(NOT RUBY_FOUND)

IF(NOT RUBY_FOUND)
  # make FIND_PACKAGE friendly
  IF(NOT RUBY_FIND_QUIETLY)
    IF(RUBY_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR
              "Ruby required, please specify its location with RUBY_ROOT.")
    ELSE(RUBY_FIND_REQUIRED)
      MESSAGE(STATUS "Ruby was not found.")
    ENDIF(RUBY_FIND_REQUIRED)
  ENDIF(NOT RUBY_FIND_QUIETLY)
ENDIF(NOT RUBY_FOUND)


MARK_AS_ADVANCED(
  RUBY_EXECUTABLE
  RUBY_LIBRARY
  RUBY_INCLUDE_PATH
  )
