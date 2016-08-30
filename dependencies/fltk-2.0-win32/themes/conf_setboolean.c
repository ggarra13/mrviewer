/*
   "$Id: conf_setboolean.c 2866 2002-12-09 04:52:32Z spitzak $"

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
        int setconf_boolean(const char *configfile, const char *key, int bvalue)

        description:
               sets the boolean value associated with a key in a config file
        arguments:
               configfile: path of config file
               key: section/key to set
               bvalue: boolean value associated with key (on = 1, off = 0)
        return values:
                returns 0 for OK or error code defined in conf.h
*/
int
setconf_boolean(const char *configfile,  const char *key, int bvalue)
{
        int     result;                                                         /* result of called functions */

        if (bvalue)
                result = setconf(configfile, key, "TRUE");                      /* set string value in config file to ON */
        else
                result = setconf(configfile, key, "FALSE");                     /* set string value in config file to OFF */

        return result;

} /* setconf_boolean() */

/*
    End of "$Id: conf_setboolean.c 2866 2002-12-09 04:52:32Z spitzak $".
*/
