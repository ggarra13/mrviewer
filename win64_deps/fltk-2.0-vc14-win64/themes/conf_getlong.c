/*
   "$Id: conf_getlong.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
#include <config.h>

/*
        int getconf_long(const char *configfile, const char *key,
                         long *lvalue)

        description:
                gets the long value associated with a key in a config file
        arguments:
                configfile: path of config file
                key: section/key to look for
        return values:
                returns 0 for OK or error code defined in conf.h
                lvalue: the long integer associated with key in section
*/
int
getconf_long(const char *configfile, const char *key, long *lvalue)
{
        char    svalue[CONF_MAX_LINE_LEN];                                      /* tempory storage for string value */
        int     result;                                                         /* result of called functions */

        if (!lvalue)                                                            /* NULL pointer was passed */
                return CONF_ERR_ARGUMENT;

        if ((result = getconf(configfile, key, svalue, sizeof(svalue))))        /* get string value from config file */
                return result;

        *lvalue = strtol(svalue,0,0);                                                 /* convert to long */
        return CONF_SUCCESS;
} /* getconf_long() */

/*
    End of "$Id: conf_getlong.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
