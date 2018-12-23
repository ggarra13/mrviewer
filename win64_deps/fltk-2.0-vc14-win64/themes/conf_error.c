/*
   "$Id: conf_error.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
        const char *getconf_error(int error)

        description:
                returns string error message associated with error number
        arguments:
                error: error number returned by [gs]etconf* functions
        return value:
                returns pointer to a static error message string
*/
const char *
getconf_error(int error)                                                        /* get error string associated with error number */
{
    switch(error)
    {
        case CONF_SUCCESS: return "successful completion";
        case CONF_ERR_ARGUMENT: return "invalid argument";
        case CONF_ERR_FILE: return "could not access config file";
        case CONF_ERR_SECTION: return "config file section not found";
        case CONF_ERR_KEY: return "key not found in section";
        case CONF_ERR_DEPTH: return "section or include nesting too deep";
        case CONF_ERR_MEMORY: return "could not allocate memory";
        case CONF_ERR_NOVALUE: return "invalid value associated with key";
        case CONF_ERR_AGAIN: return "try operation again (lockfile exists?)";
        default: return "unknown error";
    }
}

/*
    End of "$Id: conf_error.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
