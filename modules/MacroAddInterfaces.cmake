
include(MacroAddFileDependencies)

################################################################################
# MACRO_ADD_INTERFACES(idl_files...)
#
# Syntax: MACRO_ADD_INTERFACES(<output list> <idl1> [<idl2> [...]])
# Notes: <idl1> should be absolute paths so the MIDL compiler can find them.
# For every idl file xyz.idl, two files xyz.h and xyz.c are generated, which
# are added to the <output list>

# Copyright (c) 2007, Guilherme Balena Versiani, <[EMAIL PROTECTED]>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
MACRO (MACRO_ADD_INTERFACES _output_list)
  FOREACH(_in_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_out_FILE ${_in_FILE} NAME_WE)
    GET_FILENAME_COMPONENT(_in_PATH ${_in_FILE} PATH)

    SET(_out_header_name ${_out_FILE}.h)
    SET(_out_header ${CMAKE_CURRENT_BINARY_DIR}/${_out_header_name})
    SET(_out_iid_name ${_out_FILE}.c)
    SET(_out_iid ${CMAKE_CURRENT_BINARY_DIR}/${_out_iid_name})
    #message("_out_header_name=${_out_header_name}, _out_header=${_out_header}, _out_iid=${_out_iid}")
    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out_header} ${_out_iid}
      DEPENDS ${_in_FILE}
      COMMAND midl /header ${_out_header_name} /iid ${_out_iid_name} ${_in_FILE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    MACRO_ADD_FILE_DEPENDENCIES(
      ${_out_header}
      ${_in_FILE}
    )

    SET_SOURCE_FILES_PROPERTIES(
      ${_out_header}
      ${_out_iid}
      PROPERTIES
      GENERATED TRUE
    )
    SET_SOURCE_FILES_PROPERTIES(${_in_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)

    SET(${_output_list} ${${_output_list}}
      ${_out_header}
      ${_out_iid}
    )

  ENDFOREACH(_in_FILE ${ARGN})

ENDMACRO (MACRO_ADD_INTERFACES)


