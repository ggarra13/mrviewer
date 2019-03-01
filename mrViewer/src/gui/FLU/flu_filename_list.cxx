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


// Wrapper for scandir with const-correct function prototypes.

#include <FL/filename.H>
#include "flustring.h"
#include <stdlib.h>
#include "flu_filename_list.h"

// OTB Modifications
//#include <config.h>
#include "flu-config.h"


#if defined(__APPLE__)

// Starting with MacOSX 10.8, the scandir prototype is POSIX compliant
// We need to check MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_8
// macros to figure this out
// See http://www.fltk.org/str.php?L2864 for a similar fix in FLTK sources

// MAC_OS_X_VERSION_10_8 is defined in <FL/x.H>
// starting from rev 9649
#  include <FL/x.H>

// Case where FLTK rev is < 9649
#  ifndef MAC_OS_X_VERSION_10_8
#    define MAC_OS_X_VERSION_10_8 1080
#  endif

// This defines MAC_OS_X_VERSION_MAX_ALLOWED
#  include "AvailabilityMacros.h"

#endif

#if !defined(__GLIBC_PREREQ)
#   define  __GLIBC_PREREQ(a,b) 0
#endif

extern "C" {
#ifndef HAVE_SCANDIR
  int flu_scandir (const char *dir, dirent ***namelist,
            int (*select)(dirent *),
            int (*compar)(dirent **, dirent **));
#  define scandir flu_scandir
#endif
}

int flu_alphasort(struct dirent **a, struct dirent **b) {
  return strcmp((*a)->d_name, (*b)->d_name);
}

int flu_casealphasort(struct dirent **a, struct dirent **b) {
  return strcasecmp((*a)->d_name, (*b)->d_name);
}


int flu_filename_list(const char *d, dirent ***list,
                     Flu_File_Sort_F *sort) {
#ifndef HAVE_SCANDIR
  int n = scandir(d, list, 0, sort);
#elif defined(__hpux) || defined(__CYGWIN__) || defined(sun)
  // HP-UX, Cygwin define the comparison function like this:
  int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
#elif defined(__osf__)
  // OSF, DU 4.0x
  int n = scandir(d, list, 0, (int(*)(dirent **, dirent **))sort);
#elif defined(_AIX)
  // AIX is almost standard...
  int n = scandir(d, list, 0, (int(*)(void*, void*))sort);
#elif !defined(__sgi)
  // The vast majority of UNIX systems want the sort function to have this
  // prototype, most likely so that it can be passed to qsort without any
  // changes
#if (defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 10)) \
    || defined(__FreeBSD__) \
    || (defined(__APPLE__) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_8))
  int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
#else
  int n = scandir(d, list, 0, (int(*)(const void*, const void*))sort);
#endif /* ( defined(__GLIBC__) && __GLIBC_PREREQ(2, 10) ) */

#else
  // This version is when we define our own scandir (WIN32 and perhaps
  // some Unix systems) and apparently on IRIX:
  int n = scandir(d, list, 0, sort);
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
  // we did this already during flu_scandir/win32
#else
  // append a '/' to all filenames that are directories
  int i, dirlen = strlen(d);
  char *fullname = (char*)malloc(dirlen+FL_PATH_MAX+3); // Add enough extra for two /'s and a nul
  // Use memcpy for speed since we already know the length of the string...
  memcpy(fullname, d, dirlen+1);
  char *name = fullname + dirlen;
  if (name!=fullname && name[-1]!='/') *name++ = '/';
  for (i=0; i<n; i++) {
    dirent *de = (*list)[i];
    int len = strlen(de->d_name);
    if (de->d_name[len-1]=='/' || len>FL_PATH_MAX) continue;
    // Use memcpy for speed since we already know the length of the string...
    memcpy(name, de->d_name, len+1);
    if (fl_filename_isdir(fullname)) {
      (*list)[i] = de = (dirent*)realloc(de, de->d_name - (char*)de + len + 2);
      char *dst = de->d_name + len;
      *dst++ = '/';
      *dst = 0;
    }
  }
  free(fullname);
#endif
  return n;
}

//
// End of "$Id$".
//
