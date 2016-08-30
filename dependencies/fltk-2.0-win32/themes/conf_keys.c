/*
   "$Id: conf_keys.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

    Configuration file routines for the Fast Light Tool Kit (FLTK).

    Carl Thompson's conig file routines version 0.5
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
//#include <fltk/vsnprintf.h>
#include <config.h>

extern int conf_is_path_rooted(const char *);
extern const char* conf_dirname(const char *);

/*
        int getconf_keys(const char *configfile, const char *section,
                         conf_list *list)
        description:
                gets the list of key / value pairs that are in the specified
                section
        arguments:
                configfile: path of config file
                section: section in config file (must be unique in file)
                list: pointer to a conf_list (same as pointer to pointer
                      to conf_entry) of keys
        return values:
                returns 0 for OK or error code defined in conf.h
                returns a pointer to first in list of entries in "list"
*/
int
getconf_keys(const char *configfile, const char *sec, conf_list *list)
{
  FILE       *ifp;                                                              /* file pointer for config file */
  char       current_section[CONF_MAX_SECT_LEN + 2] = "";                       /* current section */
  char       section[CONF_MAX_SECT_LEN + 1];                                    /* desired section */
  char       line[CONF_MAX_LINE_LEN];                                           /* line buffer */
  char       *p, *p2;                                                           /* miscelaneous char pointers */
  int        in_correct_section;                                                /* found the section we want? */
  int        section_found = 0;                                                 /* did we ever find the right section? */
  conf_entry **current;                                                         /* pointer to pointer to current entry */
  static int depth = 0;                                                         /* don't go too deep */

  if (!configfile || !list) return CONF_ERR_ARGUMENT;                           /* NULL pointer was passed */
  if (!sec) sec = "";                                                           /* toplevel section */

  conf_list_free(list);                                                         /* free list passed to us */
  current = list;                                                               /* pointer to current (first) entry */

  ifp = fopen(configfile, "r");
  if (!ifp) return CONF_ERR_FILE;                                               /* could not open config file */

  if (strlen(sec) > (CONF_MAX_SECT_LEN - 1))                                    /* if section name too long */
    return CONF_ERR_ARGUMENT;                                                   /* bail out */
  strncpy(section, sec, sizeof(section));
  conf_trim(section);
  if (*section && *(section + strlen(section) - 1) != conf_level_sep) {                                                               /* add trailing '/' to section name */
    *(section + strlen(section) + 1) = (char)0;
    *(section + strlen(section)) = conf_level_sep;
  }

  in_correct_section = (*section) ? 0 : 1;                                      /* found the right section if we want toplevel */
  while (fgets(line, sizeof(line), ifp)) {                                      /* while there are still lines in the section */
    /* hack so that comment separator can occur in value
       if not immediately followed by a whitespace       */
    for (p2 = line; (p = strchr(p2, conf_comment_sep)); p2 = p + 1) {
      if (strchr(CONF_WHITESPACE, *(p + 1)) ||
          line + strspn(line, CONF_WHITESPACE) == p)
      {
        *p = (char)0;
        break;
      }
    }

    conf_trim(line);                                                            /* remove unnecessary whitespace */
    if (!(*line)) continue;                                                     /* this is a blank line, go to next line */

    if (*line == '[') {                                                         /* in new section */
      strncpy(current_section, line + 1, sizeof(current_section));
      *(current_section + strlen(current_section) - 1) = conf_level_sep;
      if (!strcasecmp(current_section, section)) in_correct_section = 1;        /* this is the section we want */
      else in_correct_section = 0;                                              /* not the section we want */
      continue;                                                                 /* go to next line in file */
    }

    if (strncasecmp(current_section, section, strlen(current_section)))
      continue;                                                                 /* not wanted section or parent */

    if ( !(p = strchr(line, conf_sep)) ) {                                      /* if there is no separator character */
      char* sv;                                                                 /* to save strtok_r() state */
      p = strtok_r(line, CONF_WHITESPACE, &sv);                                 /* get the command */
      if (!strcasecmp(p, "include")) {                                          /* it is include command */
        char fn[CONF_MAXPATHLEN];                                               /* filename of include file */
        char s[CONF_MAX_SECT_LEN];                                              /* what to look for in included file */
        int r;

        p = strtok_r(0, "", &sv);                                               /* get the name of file to be included */
        conf_trim(p);                                                           /* kill unecessary whitespace */
        if (conf_is_path_rooted(p)) strncpy(fn, p, sizeof(fn));                 /* fully qualified path */
        else snprintf(fn, sizeof(fn), "%s%s", conf_dirname(configfile), p);     /* figure out pathname */
        strncpy(s, section + strlen(current_section), sizeof(s));               /* section/key to look up in included file */
        if (depth == CONF_MAX_INCLUDE) return CONF_ERR_DEPTH;                   /* went too deep */
        depth++;
        r = getconf_keys(fn, s, current);                                       /* look for what we want in included file */
        depth--;
        if (r == CONF_ERR_DEPTH) return CONF_ERR_DEPTH;                         /* went too deep */
        if (r == CONF_ERR_MEMORY) return CONF_ERR_MEMORY;                       /* buy some RAM */
        if (!r) {
          section_found = 1;                                                    /* we found what we wanted in included file */
          while (*current) current = &((*current)->next);
        }
      }
      continue;                                                                 /* go to next line */
    }
    *p++ = '\0';                                                                /* separate value from key */

    /* line is now just key, p is value */
    if (!in_correct_section) continue;                                          /* section not found yet, keep looking for it */
    section_found = 1;                                                          /* must be in right section */

    conf_trim(line);                                                            /* kill unnecessary whitespace */
    conf_trim(p);                                                               /* kill unnecessary whitespace */

    (*current) = (conf_entry *)malloc(sizeof(conf_entry));                      /* allocate memory for this entry */
    if (*current) {
      (*current)->key = strdup(line);                                           /* duplicate the key for this entry */
      (*current)->value = strdup(p);                                            /* duplicate the value for this entry */
    }
    if ((*current == 0) || ((*current)->key == 0) || ((*current)->value == 0)) {/* if we had a memory allocation problem */
      fclose(ifp);                                                              /* close data file */
      return CONF_ERR_MEMORY;                                                   /* and bail out */
    }
    (*current)->next = 0;                                                       /* set this entry's next pointer to null */
    current = &((*current)->next);                                              /* point to next entry */
  } /* while (fgets(line, sizeof(line), ifp)) */

  fclose(ifp);                                                                  /* close the config file */

  if (!section_found) return CONF_ERR_SECTION;                                  /* section was never found */
  return CONF_SUCCESS;                                                          /* it worked? */
} /* getconf_keys() */


/*
        int getconf_list(const conf_list list, const char *key, char *svalue,
                         int slen)

        description:
                gets the string associated with a key in a list returned by
                getconf_keys()
        arguments:
                list: list returned by getconf_keys()
                key: key to look for
                slen: length of passed string buffer
        return values:
                returns 0 for OK or error code defined in conf.h
                svalue: string associated with key
*/
int
getconf_list(const conf_list list, const char *k, char *svalue, int slen)
{
  char key[CONF_MAX_LINE_LEN];
  conf_entry* cent;

  strncpy(key, k, sizeof(key));
  conf_trim(key);
  for (cent = list; cent; cent = cent->next) {
    if (!strcasecmp(key, cent->key)) {
      if (!cent->value) return CONF_ERR_NOVALUE;
      strncpy(svalue, cent->value, slen);
      return CONF_SUCCESS;
    }
  }

  return CONF_ERR_KEY;
} /* getconf_list() */

/*
    End of "$Id: conf_keys.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
