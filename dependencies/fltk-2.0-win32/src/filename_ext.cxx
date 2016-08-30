// "$Id: filename_ext.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
 */

#include <fltk/filename.h>

/*!
  Returns a pointer to the last period in filename_name(f), or a
  pointer to the trailing nul if none. Notice that this points \e at
  the period, not after it!
*/
const char *fltk::filename_ext(const char *buf) {
  const char *q = 0;
  const char *p = buf;
  for (p=buf; *p; p++) {
    if (*p == '/') q = 0;
#if defined(_WIN32) || defined(__EMX__)
    else if (*p == '\\') q = 0;
#endif
    else if (*p == '.') q = p;
  }
  return q ? q : p;
}

//
// End of "$Id: filename_ext.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
