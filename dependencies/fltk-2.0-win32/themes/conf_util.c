/*
   "$Id: conf_util.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

    Configuration file routines for the Fast Light Tool Kit (FLTK).

    Carl Thompson's config file routines version 0.5
    Copyright 1995-2000 Carl Everard Thompson (clip@home.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    USA.
*/

#include "conf.h"

/*
        char *conf_trim(char *s)

        description:
                removes leading and trailing whitespace from a string
                whitespace is any character in literal string CONF_WHITESPACE
        arguments:
                s: string to be modified
        return value:
                returns s
*/
char *
conf_trim(char *s)
{
        int     i;                                                              /* temporary int */
        char    *p;                                                             /* temporary pointer */

        if (!s)                                                                 /* if null pointer passed */
                return s;

        i = strspn(s, CONF_WHITESPACE);
        memmove(s, s + i, strlen(s + i) + 1);                                   /* kill leading whitespace */

        p = s + strlen(s) - 1;
        while ((p >= s) && strchr(CONF_WHITESPACE, *p))                              /* kill trailing whitespace */
                *p-- = '\0';
        return s;
}

int conf_is_path_rooted(const char *fn)
{
  /* see if an absolute name was given: */
  if (fn[0] == '/' || fn[0] == '.'
#ifdef _WIN32
      || fn[0] == '\\' || fn[1]==':'
#endif
      )
    return 1;
  return 0;
}

const char *conf_basename(const char *fn)
{
  const char *p;

  if ( (p = strrchr(fn, '/'))
#ifdef _WIN32
      || (p = strrchr(fn, '\\'))
#endif
      )
    return p + 1;
  return fn;
}

/* this will modify the input string! */
const char *conf_dirname(char *fn)
{
  static char dirname[CONF_MAXPATHLEN];

  strncpy(dirname, fn, sizeof(dirname));
  *((char *)conf_basename(dirname)) = (char)0;

  return dirname;
}

/*
    End of "$Id: conf_util.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
