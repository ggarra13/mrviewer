/*
   "$Id: conf_endtrim.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
        char *conf_endtrim(char *s)

        description:
                removes just trailing whitespace from a string
                whitespace is any character in literal string CONF_WHITESPACE
        arguments:
                s: string to be modified
        return value:
                returns s
*/
char *
conf_endtrim(char *s)
{
        char    *p;                                                             /* temporary pointers */

        if (!s)                                                                 /* if null pointer passed */
                return s;

        p = s + strlen(s) - 1;
        while ((p >= s) && strchr(CONF_WHITESPACE, *p))                         /* kill trailing whitespace */
                *p-- = '\0';
        return s;
}

/*
    End of "$Id: conf_endtrim.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
