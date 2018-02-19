/*
   "$Id: conf_list_free.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
        int conf_list_free(conf_list* list)

        description:
                frees all memory used by a conf_list
        arguments:
                list: the conf_list to free
        return values:
                returns 0 for OK or error code defined in conf.h
*/
int
conf_list_free(conf_list *list)
{
        conf_entry  *current, *next;                                            /* current and next entries */

        if (!list)                                                              /* if NULL pointer was passed */
                return CONF_ERR_ARGUMENT;

        for (current = *list; current; current = next)                          /* while there are more entries */
        {
                free(current->key);                                             /* free space used by key element */
                if (current->value) free(current->value);                       /* free space used by value element */
                next = current->next;                                           /* pointer to next in list */
                free(current);                                                  /* free this entry */
        }

        *list = 0;
        return CONF_SUCCESS;                                                    /* successful completion */
} /* conf_list_free() */

/*
    End of "$Id: conf_list_free.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
