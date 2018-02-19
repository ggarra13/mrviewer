/* "$Id: string.c 8500 2011-03-03 09:20:46Z bgbnbigben $"
 *
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 * strcasecmp() Copyright (c) 2005 Bill Spitzak
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

#if !HAVE_STRCASECMP || defined(DOXYGEN)
/*! Do a case-insensitive string comparison. Return less than zero if
  \a s is before \a t in alphabetical order, zero if they are equal,
  and greater than zero if \a s is after \a t.
  Only ASCII strings are correctly compared. What this does with UTF-8
  or ISO-8859-1 is undefined!

  FLTK provides this function on systems that don't have it. Use
  the <fltk/string.h> header to call this portably.
*/
int strcasecmp(const char *s,	/* I - First string */
	       const char *t)	/* I - Second string */
{
  for (;;)
  {
    int x = tolower(*s) - tolower(*t);
    if (x) return x;
    if (!*s) return 0;
    s ++;
    t ++;
  }
}
#endif


#if !HAVE_STRNCASECMP || defined(DOXYGEN)
/*! Do a case-insensitive comparison of up to the first \a n characters
  of the strings.
  \see strcasecmp()

  FLTK provides this function on systems that don't have it. Use
  the <fltk/string.h> header file to call this.
*/
int				/* O - Result of comparison (<0, 0, or >0) */
strncasecmp(const char *s,	/* I - First string */
	    const char *t,	/* I - Second string */
	    size_t     n)	/* I - Maximum number of characters to compare */
{
  while (n--)
  {
    int x = tolower(*s) - tolower(*t);
    if (x) return x;
    if (!*s) return 0;
    s ++;
    t ++;
  }
  return 0;
}
#endif


#if !HAVE_STRLCAT || defined(DOXYGEN)
/*!
  Appends \a src to string \a dst of size \a siz (unlike strncat(),
  \a siz is the full size of \a dst, not space left).  At most siz-1
  characters will be copied.  Always NUL terminates (unless siz == 0).
  Returns strlen(initial dst) + strlen(src); if retval >= siz,
  truncation occurred and
  you may want to allocate a new longer buffer for \a dst of size
  retval+1.

  FLTK provides this function on systems that don't have it. Use
  the <fltk/string.h> header to call this portably.
*/
size_t				/* O - strlen(dst)+strlen(src) */
strlcat(char *dst,		/* I - Destination buffer */
	const char *src,	/* I - String to append to dst */
	size_t siz)		/* I - sizeof(dst) */
{
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  size_t dlen;

  /* Find the end of dst and adjust bytes left but don't go past end */
  while (*d != '\0' && n-- != 0)
    d++;
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0)
    return(dlen + strlen(s));
  while (*s != '\0') {
    if (n != 1) {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';

  return(dlen + (s - src));	/* count does not include NUL */
}
#endif


#if !HAVE_STRLCPY || defined(DOXYGEN)
/*!
  Copy \a src to string \a dst of size \a siz.  At most siz-1 characters
  will be copied.  Always NUL terminates (unless siz == 0).
  Returns strlen(src); if retval >= siz, truncation occurred and
  you may want to allocate a new longer buffer for \a dst of
  size retval+1.

  FLTK provides this function on systems that don't have it. Use
  the <fltk/string.h> header to call this portably.
*/
size_t				/* O - strlen(src) */
strlcpy(char *dst,		/* I - Destination buffer */
	const char *src,	/* I - String to put in dst */
	size_t siz)		/* I - sizeof(dst) */
{
  char *d = dst;
  const char *s = src;
  size_t n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0) {
    do {
      if ((*d++ = *s++) == 0)
	break;
    } while (--n != 0);
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0) {
    if (siz != 0)
      *d = '\0';		/* NUL-terminate dst */
    while (*s++)
      ;
  }

  return(s - src - 1);	/* count does not include NUL */
}
#endif /* !HAVE_STRLCPY */

#ifdef __cplusplus
}
#endif

/*
 * End of "$Id: string.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
 */
