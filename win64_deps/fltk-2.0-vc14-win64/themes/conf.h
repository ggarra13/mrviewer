/*
   "$Id: conf.h 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
/*
        file:           conf.h
        author:         Carl Thompson (clip@home.net)
        creation date:  16 Dec 1995

        Typical config file format:

        ------- test.conf --------------------------------------------------------------------------------------------------------
		
		key1 = value1							# this key is not in a section
		key2								# this key has no value associated
		
                [section1]
		# Three standard key/value pairs
            	    key1 = value1
            	    key2 = value2
            	    key3 = value3

                    [section1/subsection]					# this is how a subsection is defined
                	key1 = value1
                	key2 = value2

                [section2]
        	    key1 = value1
            	    key2 = value2

        --------------------------------------------------------------------------------------------------------------------------


        Notes:
        - comments may be on a line by themselves or at the end of a line
	  - for comments at the end of the line, the comment separator
	    character must followed by whitespace
        - indentation/formatting is handled automatically (sorry!)
*/



#ifndef CONF_H
#  define CONF_H

/* includes */
#  include <sys/types.h>
#  include <stdlib.h>
#  include <stdio.h>
#  include <string.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <ctype.h>
#  include <errno.h>
#  include <limits.h>
#  if defined(_WIN32)
#    include <io.h>
#  else
#    include <unistd.h>
#  endif /* _WIN32 */

#define FL_API

/* misc defines */
#  define CONF_WHITESPACE	" \t\n\r"	/* all whitespace characters */
#  define CONF_MAX_LEVEL	10		/* maximum depth of sections in config file */
#  define CONF_MAX_INCLUDE	10		/* maximum depth of includes in config file */
#  define CONF_INDENT		2		/* number of spaces to use for indenting */
#  define CONF_MAX_LINE_LEN	512		/* maximum length of line in config file */
#  define CONF_MAX_SECT_LEN	128		/* maximum length of section name */
#  ifdef PATH_MAX
#    define CONF_MAXPATHLEN	PATH_MAX
#  else
#    define CONF_MAXPATHLEN	128		/* maximum length of pathname */
#  endif


/* error defines */
#  define CONF_SUCCESS		0		/* successful return */
#  define CONF_ERR_ARGUMENT	1		/* argument invalid (NULL?) */
#  define CONF_ERR_FILE		2		/* trouble accessing config file or directory */
#  define CONF_ERR_SECTION	3		/* requested section was not found */
#  define CONF_ERR_KEY		4		/* requested key was not found */
#  define CONF_ERR_DEPTH	5		/* nested sections or includes in config file too deep */
#  define CONF_ERR_MEMORY	6		/* memory allocation error */
#  define CONF_ERR_NOVALUE	7		/* key found, but invalid value associated with it */
#  define CONF_ERR_AGAIN	8		/* try operation again (lockfile existed?) */

#  ifdef __cplusplus
extern "C" {
#  endif

  /* data types */
  typedef struct _conf_entry
  {
        char *key;
        char *value;
        struct _conf_entry *next;
  } conf_entry;

  typedef conf_entry* conf_list;

  /* global variables */
  extern FL_API char   conf_sep;                  /* seperator charactor used in config files */
  extern FL_API char   conf_level_sep;            /* seperator used to denote nested sections */
  extern FL_API char   conf_comment_sep;          /* denotes comments */
  extern FL_API int    conf_comment_column;       /* the column comments start in */



  /* function declarations */

  /*
        int getconf(const char *configfile, const char *key, char *svalue,
                    int slen)

        description:
                gets the string associated with a key in a config file
        arguments:
                configfile: path of config file
                key: section/key to look for
                slen: length of passed string buffer
        return values:
                returns 0 for OK or error code defined in conf.h
                svalue: string associated with key
  */
  FL_API int getconf(const char *configfile, const char *key, char *svalue,
                     int slen);




  /*
        void conf_clear_cache(void)

        description:
                clears all cached config file data so it will be reread
                from disk
  */

  FL_API void conf_clear_cache(void);




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
                      to conf_entry) of key / value pairs
        return values:
                returns 0 for OK or error code defined in conf.h
                returns a pointer to first in list of entries in "list"
  */
  FL_API int getconf_keys(const char *configfile, const char *section,
                          conf_list *list);



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
  FL_API int getconf_list(const conf_list list, const char *key, char *svalue,
                         int slen);



  /*
        int getconf_sections(const char *configfile, const char *section,
                             conf_list *list)

        description:
                gets the list of subsections that are in the specified section
        arguments:
                configfile: path of config file
                section: section in config file (must be unique in file)
                list: pointer to a conf_list (same as pointer to pointer
                      to conf_entry) of subsections
        return values:
                returns 0 for OK or error code defined in conf.h
                returns a pointer to first in list of entries in "list"
  */
  FL_API int getconf_sections(const char *configfile, const char *section,
                              conf_list *list);



  /*
        int conf_list_free(conf_list* list)

        description:
                frees all memory used by a conf_list
        arguments:
                list: the conf_list to free
        return values:
                returns 0 for OK or error code defined in conf.h
  */
  FL_API int conf_list_free(conf_list *list);



  /*
        int setconf(const char *configfile, const char *key,
                    const char *svalue)

        description:
                sets the string associated with a key in a config file
        arguments:
                configfile: path of config file
                key: section/key to set
                svalue: string associated with key
        return values:
                returns 0 for OK or error code defined in conf.h
  */
  FL_API int setconf(const char *configfile, const char *key,
                     const char *svalue);



  /*
        int setconf_keys(const char *configfile, const char *section,
                         conf_list list)

        description:
                sets the list of key / value pairs that are in the specified
                section from list
        arguments:
                configfile: path of config file
                section: section in config file (must be unique in file)
                list: a conf_list (same as a pointer to conf_entry) of
                key / value pairs
        return values:
                returns 0 for OK or error code defined in conf.h
  */
  FL_API int setconf_keys(const char *configfile, const char *section,
                          conf_list list);



  /*
        int setconf_list(conf_list list, const char *key, const char *svalue)

        description:
                sets the string associated with a key in a list returned by
                getconf_keys()
        arguments:
                list: list returned by getconf_keys()
                key: key to look for
                slen: length of passed string buffer
                svalue: string associated with key
        return values:
                returns 0 for OK or error code defined in conf.h
  */
  FL_API int setconf_list(conf_list list, const char *key, const char *svalue);



  /*
        int delconf(const char *configfile, const char *key)

        description:
                deletes an entry from a section in a config file
        arguments:
                configfile: path of config file
                key: section/key to delete
        return values:
                returns 0 for OK or error code defined in conf.h
  */
  FL_API int delconf(const char *configfile, const char *key);



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
  FL_API int getconf_long(const char *configfile, const char *key,
                          long *lvalue);



  /*
        int setconf_long(const char *configfile, const char *key, long lvalue)

        description:
                sets the long value associated with a key in a config file
        arguments:
                configfile: path of config file
                key: section/key to set
                lvalue: the long integer associated with key in section
        return values:
                returns 0 for OK or error code defined in conf.h
  */
  FL_API int setconf_long(const char *configfile, const char *key,
                          long lvalue);



  /*
        int getconf_boolean(const char *configfile, const char *key,
                            int *bvalue)

        description:
                gets the boolean value associated with a key in a config file
        arguments:
                configfile: path of config file
                key: section/key to look for
        return values:
                returns 0 for OK or error code defined in conf.h
                bvalue: boolean value associated with key (ON = 1, OFF = 0)
  */
  FL_API int getconf_boolean(const char *configfile, const char *key,
                             int *bvalue);



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
  FL_API int setconf_boolean(const char *configfile, const char *key,
                             int bvalue);



  /*
        const char *getconf_error(int error)

        description:
                returns string error message associated with error number
        arguments:
                error: error number returned by [gs]etconf* functions
        return value:
                returns pointer to a static error message string
  */
  FL_API const char *getconf_error(int error);




  /*
        char *conf_trim(char *s)

        description:
                removes leading and trailing whitespace from a string
                whitespace is any character in literal string WHITESPACE
        arguments:
                s: string to be modified
        return value:
                returns s
  */
  FL_API char *conf_trim(char *s);



  /*
        char *conf_endtrim(char *s)

        description:
                removes just trailing whitespace from a string
                whitespace is any character in literal string WHITESPACE
        arguments:
                s: string to be modified
        return value:
                returns s
  */
  FL_API char *conf_endtrim(char *s);



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
  FL_API int conf_strcnt(const char *s, char c);



  /*
        long conf_atol(const char *decstr)

        description:
                this function coverts the given decimal string to a long int
                on illegally formatted input, returns zero
        arguments:
                decstr: decimal string to be converted
        return value:
                returns the long integer represented by the decimal string
  */
  FL_API long conf_atol(const char *decstr);



  FL_API int conf_is_path_rooted(const char *path);

#  ifdef __cplusplus
}
#  endif

#endif /* !CONF_H */

/*
 * End of "$Id: conf.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
 */
