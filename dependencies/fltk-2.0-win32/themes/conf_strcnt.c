/*
   "$Id: conf_strcnt.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
        int *conf_strcnt(const char *s, char c)

        description:
                counts the number of instances of char c in string s
        arguments:
                s: string in which to look for c
                c: character to look for in s
        return value:
                returns the number of instances of c in s
*/
int
conf_strcnt(const char *s, char c)
{
        int count;
        const char *p;

        for (count = 0, p = s; *p != (char)0; p++)
                if (*p == c) count++;

        return count;
}

/*
    End of "$Id: conf_strcnt.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
