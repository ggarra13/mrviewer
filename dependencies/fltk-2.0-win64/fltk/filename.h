// "$Id: filename.h 8507 2011-03-06 10:18:05Z bgbnbigben $"

/* Copyright 1998-2006 by Bill Spitzak and others.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Please report all bugs and problems to the following page:
 *
 *    http://www.fltk.org/str.php
 *
 * These functions are not in the fltk namespace because they really
 * should not be part of fltk. They are used by the file chooser.
 * THESE FUNCTIONS MAY BE CHANGED OR DELETED IN FUTURE VERSIONS. DO
 * NOT USE THEM, AS THEY ARE NOT AN OFFICIAL PART OF fltk!
 */

#ifndef fltk_filename_h
#define fltk_filename_h

#include "FL_API.h"
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////
#ifndef DOXYGEN

// dirent (what a pain)...

// FC: UNDER WIN32/VC6 long long is undefined, so use __int64 instead
//  for cross platform type compatibility though in fact VC6 uses 
//  a 32 bit long to calculate size in the stat struct so don't expect
//  to handle >4GB files here...
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__) && (_MSC_VER==1200)
typedef unsigned __int64 FL_FILESIZE_T;
#else
typedef unsigned long long FL_FILESIZE_T;
#endif

#if defined(__WATCOMC__)

# include <sys/types.h>
# include "direct.h"

#elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
// Dummy version used on win32 that just holds a name:

struct dirent {char d_name[1];};

#elif defined(__linux)
// Newest Linux libc is broken when it emulates the 32-bit dirent, it
// generates errors when the data like the inode number does not fit, even
// though we are not going to look at anything other than the name. This
// code seems to force the 64-bit version to be used:

# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# include <features.h>
# include <sys/types.h>
# include <dirent.h>
# if defined(__GLIBC_PREREQ)
#  if __GLIBC_PREREQ(2,3)
#   define dirent dirent64
#   define scandir scandir64
#  endif
# endif

#else
// warning: on some systems (very few nowadays?) <dirent.h> may not exist.
// The correct information is in one of these three files:
//  #include <sys/ndir.h>
//  #include <sys/dir.h>
//  #include <ndir.h>
// plus you must do the following #define:
//  #define dirent direct
// I recommend you create a /usr/include/dirent.h containing the correct info

# include <sys/types.h>
# include <dirent.h>

#endif

#ifndef PATH_MAX
# ifdef _MAX_PATH
#  define PATH_MAX _MAX_PATH
# else
#  define PATH_MAX 1024
# endif
#endif

#endif
////////////////////////////////////////////////////////////////

namespace fltk {

/// \name fltk/filename.h
/// Some functions to manipulate filenames, to make portable programs.
//@{

// To deal with Windows' Unicode chars, the "stat" struct has also been gratuitously changed
// the stat struct to _stat. This needs to be set and unset accordingly.
// BS: It seems at the moment the MSVS compilers have a cry if stat is undefined after this definition
// and redefined later, thus I'm going to leave this in. This may need to be modified at a future date....
// \todo clean up the #define stat _stat
#if defined (_WIN32) && !defined (__CYGWIN__)
# define stat _stat
#endif
FL_API int fltk_stat(const char* name, struct stat *buffer);

FL_API int filename_absolute(char *to, int tolen, const char *from, const char* cwd=0);
FL_API int filename_relative(char *to, int tolen, const char *from, const char* cwd=0);
FL_API const char *filename_name(const char *);
inline char* filename_name(char* a) {return (char*)(filename_name((const char*)a));}
FL_API const char *filename_ext(const char *);
inline char* filename_ext(char* a) {return (char*)(filename_ext((const char*)a));}
FL_API bool filename_match(const char *, const char *pattern); // glob match
FL_API bool filename_exist(const char*);
FL_API bool filename_isdir(const char*);
FL_API bool filename_isfile(const char*);
FL_API FL_FILESIZE_T filename_size(const char *); // return size of file
FL_API long int filename_mtime(const char *); // return modification time

typedef int (FileSortF)(const dirent*const*, const dirent*const*);
FL_API int alphasort(const dirent*const*, const dirent*const*);
FL_API int casealphasort(const dirent*const*, const dirent*const*);
FL_API int casenumericsort(const dirent*const*, const dirent*const*);
FL_API int numericsort(const dirent*const*, const dirent*const*);
FL_API int modificationsort(const dirent*const*, const dirent*const*);
FL_API int filename_list(const char *d, dirent ***list, FileSortF *sort);
FL_API int filename_list(const char *d, dirent ***list); // uses numericsort

//@}

}

#endif

// End of "$Id: filename.h 8507 2011-03-06 10:18:05Z bgbnbigben $".
