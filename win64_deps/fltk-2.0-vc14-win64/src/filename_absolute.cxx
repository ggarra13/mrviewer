/* "$Id: filename_absolute.cxx 4660 2005-11-27 14:45:48Z mike $"
 *
 * Copyright 2004 by Bill Spitzak and others.
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
 * Please report all bugs and problems to http://www.fltk.org/str.php
 */

#include <config.h>
#include <fltk/filename.h>
#include <fltk/string.h>
#include <stdlib.h>
#include <assert.h>
#if defined(WIN32) && !defined(__CYGWIN__)
# include <direct.h>
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#  if !defined(__DMC__)
#    define getcwd _getcwd
#  endif
#else
#  include <unistd.h>
#  ifdef __EMX__
#    define getcwd _getcwd2
#  endif
#include <ctype.h>
#endif

#if defined(WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
static inline bool isdirsep(char c) {return c=='/' || c=='\\';}
#else
#define isdirsep(c) ((c)=='/')
#endif
#include <ctype.h>

/**
  Return the filename \a from expanded to a full "absolute" path name
  by prepending the current directory:
  - \a output is a pointer to a buffer that the result is written to.
  - \a length is the size of \a output. No more than \a length-1 bytes
    are written, plus a trailing nul.
  - \a input is the initial filename. Must be different than output!
  - \a cwd is the directory that filename is relative to. If
    this is NULL then the result of getcwd() is used.

  The return value is the number of bytes this \e wants to write
  to \a output. If this value is greater or equal to \a length, then
  you know the output has been truncated, and you can call this again
  with a buffer of n+1 bytes.

  Leading "./" sequences in \a input are removed, and "../" sequences
  are removed as well as the matching trailing part of the prefixed
  directory.  Technically this is incorrect if symbolic links are used
  but this matches the behavior of most programs.

  If the \a pwd argument is null, this also expands names starting
  with '~' to the user or another user's HOME directory.  To expand a
  filename starting with ~ in the current directory you must start it
  with "./~".

  If input is a zero-length string then the pwd is returned with a
  slash added to the end.
*/
int
fltk::filename_absolute(char *output, int length, const char *input, const char* pwd)
{
  assert(output > input || output+length <= input);
  const char* prefix = 0;
  int prefixlen = 0;
  if (!pwd && input[0] == '~') {
    prefix = getenv("HOME");
    if (prefix && *prefix) {
      prefixlen = strlen(prefix);
      input++;
      if (isdirsep(*input)) {
	input++;
      } else if (*input) {
	// another user. Fake it for now by assumming it is at the same
	// level as the current user and has that user's name. The real
	// real way is to call getpwnam(name):
	while (prefixlen > 0 && !isdirsep(prefix[--prefixlen]));
      }
    }
  } else if (isdirsep(input[0]) /*|| input[0] == '|' // for tcl pipes? */
#if defined(_WIN32) || defined(__EMX__) && !defined(__CYGWIN__)
	     || input[0] && input[1]==':'
#endif
	     ) {
    ;
  } else {
    // current directory
    if (pwd) prefix = pwd;
    else {
      prefix = getcwd(output, length);
      if (!prefix) prefix = getenv("PWD");
    }
    prefixlen = strlen(prefix);
  }
  while (*input == '.') {
    if (isdirsep(input[1])) {
      input += 2;
    } else if (input[1] == '.' && isdirsep(input[2])) {
      if (!prefixlen) break;
      while (prefixlen > 0 && !isdirsep(prefix[--prefixlen]));
      input += 3;
    } else break;
  }
  if (!prefix) return strlcpy(output, input, length);
  if (prefixlen > length-2) prefixlen = length-2;
  if (prefix != output) memcpy(output, prefix, prefixlen);
  if (!prefixlen || !isdirsep(prefix[prefixlen-1])) output[prefixlen++] = '/';
  return prefixlen+strlcpy(output+prefixlen, input, length-prefixlen);
}

/**
Does the opposite of filename_absolute(). Produces the shortest possible
name in \a output that is relative to the current directory. If the
filename does not start with any text that matches the current directory
then it is returned unchanged.

Return value is the number of characters it wants to write to \a output.
*/
int
fltk::filename_relative(char *to, int tolen, const char* from, const char* cwd) {
  assert(to > from || to+tolen <= from);

  if (!isdirsep(*from)
#if defined(_WIN32) || defined(__EMX__)
      && !(from[0] && from[1]==':')
#endif
      ) {
    return strlcpy(to, from, tolen);
  }

  if (!cwd) {
    cwd = getcwd(to, tolen);
    if (!cwd) cwd = getenv("PWD");
  }

  const char* slash = from;
  const char* newslash = cwd;
#if defined(WIN32) || defined(__EMX__)
  if (!isdirsep(*from)) { // from starts with drive letter
    if (tolower(*from & 255) != tolower(*cwd & 255)) {
      // Not the same drive...
      return strlcpy(to, from, tolen);
    }
    slash = from+2;
    newslash = cwd+2;
  } else {
    if (!isdirsep(*cwd)) // to starts with drive but from does not
      return strlcpy(to, from, tolen);
  }
#endif
  for (; *slash && *newslash; slash ++, newslash ++)
    if (isdirsep(*slash) && isdirsep(*newslash)) continue;
#if defined(WIN32) || defined(__EMX__) || defined(__APPLE__)
    else if (tolower(*slash & 255) != tolower(*newslash & 255)) break;
#else
    else if (*slash != *newslash) break;
#endif // WIN32 || __EMX__ || __APPLE__

  if (*newslash == '\0' && *slash != '\0' && !isdirsep(*slash))
    newslash--;

  while (!isdirsep(*slash) && slash > from) slash --;

  if (isdirsep(*slash)) slash ++;

  if (*newslash != '\0')
    while (!isdirsep(*newslash) && newslash > cwd) newslash --;

  to[0]         = '\0';

  while (*newslash != '\0') {
    if (isdirsep(*newslash)) strlcat(to, "../", tolen);
    newslash ++;
  }

  return strlcat(to, slash, tolen);
}


//
// End of "$Id: filename_absolute.cxx 4660 2005-11-27 14:45:48Z mike $".
//
