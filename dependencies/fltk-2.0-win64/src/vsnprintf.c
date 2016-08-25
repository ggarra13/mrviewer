/* "$Id: vsnprintf.c 8500 2011-03-03 09:20:46Z bgbnbigben $"
 *
 * Copyright 1998-2006 by Bill Spitzak and others.
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

#include <config.h>
#include <fltk/string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !HAVE_SNPRINTF || defined(DOXYGEN)

/*!
  printf a string and set of arguments into an output buffer. At
  most size-1 bytes will be written, and a NUL terminator is \e always
  added. The return value is the number of bytes that \e would be
  written if the buffer was big enough, if this is greater than \a
  size then you need to reallocate a buffer of retval+1 size to get
  the full result.

  FLTK provides an implementation of this function on the (few)
  systems that don't provide it. Include the <fltk/string.h> header to
  call this portably. FLTK's emulation is rather poor and has the
  following bugs:
  - Field width & Precision is ignored for %%%, %%c, and %%s.
  - A malicious user who manages to create a %%-fmt string that prints
    more than 99 characters can still overflow the temporary buffer.
    For instance %%110f will overflow.
  - Only handles formats that are both documented in the glibc man page
    for printf and also handled by your system's sprintf().
  - Return value is not correct. If the buffer overflows a value
    greater or equal to \a size is returned, but it is only a guess
    about the actual length.

  Windows, Linux, and BSD all have this function so FLTK's emulation
  is not used. Warning: some systems do not return the correct value
  when the buffer overflows. A common alternative is to return -1 or
  \a size.
*/
int snprintf(char* str, size_t size, const char* fmt, ...) {
  int ret;
  va_list ap;
  va_start(ap, fmt);
  ret = vsnprintf(str, size, fmt, ap);
  va_end(ap);
  return ret;
}

#endif

#if !HAVE_VSNPRINTF || defined(DOXYGEN)

/*! See snprintf(). This version takes a va_list so it can be called
  from another function that has a variable argument list.

  FLTK provides an implementation of this function on the (few)
  systems that don't provide it. Include the <fltk/string.h> header to
  call this portably. See snprintf() for bugs with this emulation.
*/
int vsnprintf(char* str, size_t size, const char* fmt, va_list ap) {
  const char* e = str+size-1;
  char* p = str;
  char copy[20];
  char* copy_p;
  char sprintf_out[100];

  while (*fmt && p < e) {
    if (*fmt != '%') {
      *p++ = *fmt++;
    } else {
      fmt++;
      copy[0] = '%';
      for (copy_p = copy+1; copy_p < copy+19;) {
	switch ((*copy_p++ = *fmt++)) {
	case 0:
	  fmt--; goto CONTINUE;
	case '%':
	  *p++ = '%'; goto CONTINUE;
	case 'c':
	  *p++ = va_arg(ap, int);
	  goto CONTINUE;
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
	  *copy_p = 0;
	  sprintf(sprintf_out, copy, va_arg(ap, int));
	  copy_p = sprintf_out;
	  goto DUP;
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	  *copy_p = 0;
	  sprintf(sprintf_out, copy, va_arg(ap, double));
	  copy_p = sprintf_out;
	  goto DUP;
	case 'p':
	  *copy_p = 0;
	  sprintf(sprintf_out, copy, va_arg(ap, void*));
	  copy_p = sprintf_out;
	  goto DUP;
	case 'n':
	  *(va_arg(ap, int*)) = p-str;
	  goto CONTINUE;
	case 's':
	  copy_p = va_arg(ap, char*);
	  if (!copy_p) copy_p = "NULL";
	DUP:
	  while (*copy_p && p < e) *p++ = *copy_p++;
	  goto CONTINUE;
	}
      }
    }
  CONTINUE:;
  }
  *p = 0;
  return p-str+strlen(fmt);
}

#endif

#ifdef __cplusplus
}
#endif

/*
 * End of "$Id: vsnprintf.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
 */

