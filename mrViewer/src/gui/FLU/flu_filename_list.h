/*
 * Taken explicitly from FLTK to avoid transferring dirent between FLU and FLTK,
 * causing filename mess when FLTK is not compiled with Large File Support
 *
 * The bugfix involves :
 * - copy/paste of fl_filename_list.c, fl_numericsort.c, fl_scandir.c, fl_scandir_win32.c,
 *   flstring.h and flstring.c
 * - creation of a flu-config.h configuration file to define some compile-time macros
 * - modification of CMakeLists.txt to generate the configuration file the same way
 *   fltk-config.h is generated
 * - renaming the function with 'flu' prefix instead of 'fl'
 * - using the renamed flu_filename_list function in Flu_File_Chooser.cpp
 *
 */
#ifndef flu_filename_list_h
#  define flu_filename_list_h

#include <FL/filename.H>
#include "flustring.h"
#include <stdlib.h>


#  if defined(WIN32) && !defined(__CYGWIN__) && !defined(__WATCOMC__)

//
// [OTB modification]
// On windows, dirent is defined in <FL/filename.H>
// Don't define it twice, otherwise we get a compile error
//
// struct dirent {char d_name[1];};

#  elif defined(__APPLE__) && defined(__PROJECTBUILDER__)

/* Apple's ProjectBuilder has the nasty habit of including recursively
 * down the file tree. To avoid re-including <FL/dirent.h> we must
 * directly include the systems math file. (Plus, I could not find a
 * predefined macro for ProjectBuilder builds, so we have to define it
 * in the project)
 */
#    include <sys/types.h>
#    include "/usr/include/dirent.h"

#  elif defined(__WATCOMC__)
#    include <sys/types.h>
#    include <direct.h>

#  else
/*
 * WARNING: on some systems (very few nowadays?) <dirent.h> may not exist.
 * The correct information is in one of these files:
 *
 *     #include <sys/ndir.h>
 *     #include <sys/dir.h>
 *     #include <ndir.h>
 *
 * plus you must do the following #define:
 *
 *     #define dirent direct
 *
 * It would be best to create a <dirent.h> file that does this...
 */
#    include <sys/types.h>
#    include <dirent.h>
#  endif

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

FL_EXPORT int flu_alphasort(struct dirent **, struct dirent **);
FL_EXPORT int flu_casealphasort(struct dirent **, struct dirent **);
FL_EXPORT int flu_casenumericsort(struct dirent **, struct dirent **);
FL_EXPORT int flu_numericsort(struct dirent **, struct dirent **);

typedef int (Flu_File_Sort_F)(struct dirent **, struct dirent **);

#  ifdef __cplusplus
}

/*
 * Portable "scandir" function.  Ugly but necessary...
 */

FL_EXPORT int flu_filename_list(const char *d, struct dirent ***l,
                                Flu_File_Sort_F *s = flu_numericsort);
#  endif /* __cplusplus */

#endif /* flu_filename_list_h */

