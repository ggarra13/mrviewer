SET(FLU_VERSION_MAJOR "2")
SET(FLU_VERSION_MINOR "14")
SET(FLU_VERSION_PATCH "1")

SET(FLU_VERSION_STRING "${FLU_VERSION_MAJOR}.${FLU_VERSION_MINOR}.${FLU_VERSION_PATCH}")
SET(FLU_LIBRARY_PROPERTIES
    VERSION   "${FLU_VERSION_MAJOR}.${FLU_VERSION_MINOR}.${FLU_VERSION_PATCH}"
    SOVERSION "${FLU_VERSION_MAJOR}.${FLU_VERSION_MINOR}")

IF(CYGWIN)
  ADD_DEFINITIONS(-DCYGWIN)
ENDIF(CYGWIN)

#
# Handle Large File Support
#
# Needed to avoid Flu_FileChooser calling fl_filename_list function from FLTK,
# leading to filename mess if FLTK is not built with large file support
#
# We redefine fl_filename_list in function flu_filename_list
# This involves adding the files :
# flu_filename_list.cxx, flu_numeric_sort.c, flu_scandir.c, flu_scandir_win32.c
# We need also to add a configuration file as FLTK does
#
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)

# Check if header file exists and add it to the list.
MACRO(CHECK_INCLUDE_FILE_CONCAT FILE VARIABLE)
  CHECK_INCLUDE_FILES("${PROJECT_INCLUDES};${FILE}" ${VARIABLE})
  IF(${VARIABLE})
    SET(PROJECT_INCLUDES ${PROJECT_INCLUDES} ${FILE})
  ENDIF(${VARIABLE})
ENDMACRO(CHECK_INCLUDE_FILE_CONCAT)

CHECK_INCLUDE_FILE_CONCAT("dirent.h"       HAVE_DIRENT_H)
CHECK_INCLUDE_FILE_CONCAT("stdio.h"        HAVE_STDIO_H)
CHECK_INCLUDE_FILE_CONCAT("strings.h"      HAVE_STRINGS_H)
CHECK_INCLUDE_FILE_CONCAT("sys/dir.h"      HAVE_SYS_DIR_H)
CHECK_INCLUDE_FILE_CONCAT("sys/ndir.h"     HAVE_SYS_NDIR_H)
CHECK_INCLUDE_FILE_CONCAT("sys/select.h"   HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILE_CONCAT("sys/stdtypes.h" HAVE_SYS_STDTYPES_H)

CHECK_SYMBOL_EXISTS(strlcat       "${PROJECT_INCLUDES}" HAVE_STRLCAT)
CHECK_SYMBOL_EXISTS(strlcpy       "${PROJECT_INCLUDES}" HAVE_STRLCPY)
CHECK_SYMBOL_EXISTS(vsnprintf     "${PROJECT_INCLUDES}" HAVE_VSNPRINTF)
CHECK_SYMBOL_EXISTS(snprintf      "${PROJECT_INCLUDES}" HAVE_SNPRINTF)
CHECK_SYMBOL_EXISTS(scandir       "${PROJECT_INCLUDES}" HAVE_SCANDIR)
#
# /End Handle Large File Support
#


CONFIGURE_FILE(fluconfigh.cmake.in
  ${Monteverdi_BINARY_DIR}/flu-config.h)

SET(CPPFILES
Flu_Button.cpp             Flu_Combo_Tree.cpp            Flu_Float_Input.cpp  Flu_Link_Button.cpp     Flu_Separator.cpp
Flu_Chat_Buffer.cpp        Flu_DND.cpp                   Flu_GL_Canvas.cpp    Flu_Output.cpp          Flu_Simple_Group.cpp
Flu_Choice_Group.cpp       Flu_Dual_Progress_Meter.cpp   Flu_GL_Window.cpp    flu_pixmaps.cpp         FluSimpleString.cpp
Flu_Collapsable_Group.cpp  Flu_Dual_Slider.cpp           Flu_Helpers.cpp      Flu_Progress.cpp        Flu_Spinner.cpp
Flu_Combo_Box.cpp          Flu_File_Chooser.cpp          Flu_Int_Input.cpp    Flu_Progress_Meter.cpp  Flu_Toggle_Group.cpp
Flu_Combo_List.cpp         flu_file_chooser_pixmaps.cpp  Flu_Label.cpp        Flu_Return_Button.cpp   Flu_Tree_Browser.cpp
flu_filename_list.cxx      Flu_Wrap_Group.cpp)

SET(CFILES
 flu_numericsort.c flu_scandir.c
)


# Suppress deprecated warning
IF(CMAKE_COMPILER_IS_GNUCXX)
      SET_SOURCE_FILES_PROPERTIES(${CPPFILES} PROPERTIES COMPILE_FLAGS -w)
ELSE(CMAKE_COMPILER_IS_GNUCXX)
  IF(WIN32)
    IF (CMAKE_CXX_COMPILER MATCHES "^cl$")
        SET_SOURCE_FILES_PROPERTIES( ${CPPFILES} PROPERTIES COMPILE_FLAGS "/W0" )
    ENDIF (CMAKE_CXX_COMPILER MATCHES "^cl$")
  ENDIF(WIN32)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


ADD_LIBRARY(flu ${CPPFILES} ${CFILES})
TARGET_LINK_LIBRARIES(flu OTBGuiFLTK)
IF(FLU_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(flu PROPERTIES ${FLU_LIBRARY_PROPERTIES})
ENDIF()

INSTALL(TARGETS flu
        RUNTIME DESTINATION ${Monteverdi_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
        LIBRARY DESTINATION ${Monteverdi_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
        ARCHIVE DESTINATION ${Monteverdi_INSTALL_LIB_DIR} COMPONENT Development)
