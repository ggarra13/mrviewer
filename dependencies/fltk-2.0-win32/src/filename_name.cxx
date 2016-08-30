// "$Id: filename_name.cxx 4115 2005-03-14 04:48:51Z spitzak $"

/* Copyright 2004-2006 by Bill Spitzak and others.
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

/**
  Returns a pointer to after the last slash in \a name. If the name
  ends with a slash then this returns a pointer to the NUL. If there
  is no slash this returns a pointer to the start of \a name.
*/
FL_API const char *fltk::filename_name(const char *name) {
  if (!name) return 0;

  const char *p,*q;
  q = name;
#ifdef _WIN32
  if (q[0] && q[1]==':') q += 2; // skip leading drive letter
  for (p = q; *p; p++) if (*p == '/' || *p == '\\') q = p+1;
#else
  for (p = q; *p;) if (*p++ == '/') q = p;
#endif
  return q;
}

//
// End of "$Id: filename_name.cxx 4115 2005-03-14 04:48:51Z spitzak $".
//
