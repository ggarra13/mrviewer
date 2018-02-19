/*
   "$Id: conf_setlong.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
        int setconf_long(const char *configfile, const char *key, long lvalue)

        description:
                sets the long value associated with a key in a config file
        arguments:
                configfile: path of config file
                key: section/key field to set
                lvalue: the long integer associated with key in section
        return values:
                returns 0 for OK or error code defined in conf.h
*/
int
setconf_long(const char *configfile, const char *key, long lvalue)
{
        char    svalue[CONF_MAX_LINE_LEN];                                      /* tempory storage for string value */
        int     result;                                                         /* result of called functions */

        sprintf(svalue, "%ld", lvalue);                                         /* put long in string */
        if ((result = setconf(configfile, key, svalue)))                        /* set string value in config file */
                return result;
        else
                return CONF_SUCCESS;
} /* setconf_long() */

/*
    End of "$Id: conf_setlong.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
