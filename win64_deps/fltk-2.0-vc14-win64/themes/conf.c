/*
   "$Id: conf.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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
        file:           conf.c
        author:         Carl Thompson (clip@home.net)
        start date:     16 Dec 1995

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


/* global variables */
char        conf_sep = '=';                                                     /* this seperates keys from values */
char        conf_level_sep = '/';                                               /* this denotes nested sections */
char	    conf_comment_sep = '#';						/* this denotes comments */
int         conf_comment_column = 51;						/* what column comments start in */

/*
    End of "$Id: conf.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
