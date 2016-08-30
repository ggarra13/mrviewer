/*
   "$Id: conf_del.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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

#ifndef F_OK
#define F_OK 0
#endif

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
int
delconf(const char *configfile, const char *k)
{
        int             done_flag = 0;                                          /* are we done yet? */
        char            configfile2[CONF_MAXPATHLEN + 6];                       /* file name for old config file */
        FILE            *ifp;                                                   /* file pointer for config file */
        FILE            *ifp2;                                                  /* file pointer for new config file */
        char            section2[CONF_MAX_SECT_LEN + 2];                        /* section with [] added */
        char            line[CONF_MAX_LINE_LEN];                                /* line buffer */
        char            line2[CONF_MAX_LINE_LEN];                               /* temporary line buffer */
        char		lineout[CONF_MAX_LINE_LEN];				/* temporary output buffer */
        char            *p, *p2;                                                /* miscelaneous char pointers */
        struct stat     stat_buf;                                               /* buffer for stat info */
        int             new_flag;                                               /* does the config file already exist? */
        int             i;                                                      /* generic integer */
        char		*comment = 0;                                           /* comment found on line */
        char            keysect[CONF_MAX_SECT_LEN], *key;
	const char *section;             /* key, section, and both */

        conf_clear_cache();

        if (!configfile || !k)
                return CONF_ERR_ARGUMENT;                                       /* NULL pointer was passed */

        strcpy(keysect, k);                                                     /* copy key and section to buffer */
        if ((p = strrchr(keysect, conf_level_sep)))                             /* if a level separator found */
        {
            *p = (char)0;                                                       /* turn into two strings */
            key = p + 1;                                                        /* set key */
            section = keysect;                                                  /* set section */
        }
        else                                                                    /* no level separator found */
        {
            key = keysect;                                                      /* set key */
            section = "";                                                       /* set toplevel section */
        }
        
        sprintf(configfile2, "%s.lock", configfile);                            /* create new file name */

        i = open(configfile2, O_CREAT | O_EXCL, 0600);                          /* try to create lock file */
        if (i == -1)                                                            /* if an error occurred opening the lock file */
             return (errno == EEXIST) ? CONF_ERR_AGAIN : CONF_ERR_FILE;         /* return appropriate error */
        close(i);
        
        new_flag = access(configfile, F_OK);                                    /* is this a new config file? */
        if (!new_flag)                                                          /* if already exists */
        {
                stat(configfile, &stat_buf);                                    /* get original permisson info */
        }
        else
        {
            unlink(configfile2);
            return CONF_SUCCESS;
        }

        ifp2 = fopen(configfile2, "w");
        if (!ifp2)                                                              /* could not open output config file */
        {
                unlink(configfile2);
                return CONF_ERR_FILE;
        }
        ifp = fopen(configfile, "r");
        if (!ifp)                                                               /* could not open input config file */
        {
                fclose(ifp2);
                unlink(configfile2);
                return CONF_ERR_FILE;
        } /* if (!ifp) */

        if (section && *section)
        {
                sprintf(section2, "[%s]", section);                              /* add [] to section name */

                while ((p = fgets(line, sizeof(line), ifp)))
                {
			strcpy(line2, line);
			
			comment = 0;
			
			
			/* if a comment found */
			if ((p2 = strchr(line, conf_comment_sep)) &&
			    (p2 == line + strspn(line, CONF_WHITESPACE) ||
			     strchr(CONF_WHITESPACE, *(p2 + 1))))
			{
				*p2 = (char)0;
				comment = ++p2;
				conf_endtrim(comment);
			}
			
                        conf_endtrim(line);

			p2 = line + strspn(line, CONF_WHITESPACE);
			
			if (*p2 == '[')
				fprintf(ifp2, "\n");
				
                       if (!strcasecmp(line + strspn(line, CONF_WHITESPACE), section2))
                            break;

            		if (!strcmp(line, ""))						/* if there is no key on this line */
			{
				if (comment)						/* line with only comment */
	                    		fprintf(ifp2, "%s", line2);
				
				continue;
			}

                        sprintf(lineout, "%s", line);                              /* write old lines to new config file */
			if (comment)
			{
				char temp[CONF_MAX_LINE_LEN];
					
				if (strlen(lineout) < conf_comment_column)
				{
					memset(temp, 0, sizeof(temp));
					memset(temp, ' ', conf_comment_column - strlen(lineout) - 1);
				}
				else
				{
				    strcpy(temp, " ");
				}
						
				sprintf(lineout + strlen(lineout), "%s%c%s",
					temp, conf_comment_sep, comment);
			}
			
			sprintf(lineout + strlen(lineout), "\n");
			
			fprintf(ifp2, "%s", lineout);
                }

                if (p)                                                          /* if section was found */
		{
                        sprintf(lineout, "%s", line);                           /* put section header in new config file */
			if (comment)
			{
				char temp[CONF_MAX_LINE_LEN];
					
				if (strlen(lineout) < conf_comment_column)
				{
					memset(temp, 0, sizeof(temp));
					memset(temp, ' ', conf_comment_column - strlen(lineout) - 1);
				}
				else
				{
				    strcpy(temp, " ");
				}
						
				sprintf(lineout + strlen(lineout), "%s%c%s",
					temp, conf_comment_sep, comment);
			}
			
			sprintf(lineout + strlen(lineout), "\n");
			
			fprintf(ifp2, "%s", lineout);
		}
        }

        while (fgets(line, sizeof(line), ifp))                                  /* while there are still lines in the file */
        {                                                                       /* (section must have been found) */
		strcpy(line2, line);
		
		comment = 0;
		
		/* if a comment found */
                /*
                   hack so that comment seperator can occur in value
                   if not immediately followed by a whitespace
                */
		for (p2 = line; (p = strchr(p2, conf_comment_sep)); p2 = p + 1)
		{
		        if (strchr(CONF_WHITESPACE, *(p + 1)) ||
		            line + strspn(line, CONF_WHITESPACE) == p)
		        {
		                *p = (char)0;					/* kill the comment */
			        comment = ++p2;					/* but remember it */
			        conf_endtrim(comment);
		                break;
		        }
		}
			
                conf_endtrim(line);							/* remove unnecessary whitespace */
			
                if (!strcmp(line, ""))						/* if there is no key on this line */
		{
			if (comment)						/* line with only comment */
	                        fprintf(ifp2, "%s", line2);
			continue;
		}

                p = line + strspn(line, CONF_WHITESPACE);

                if ((*p == '['))                                                /* if this is a different section */
		{
			fprintf(ifp2, "\n");
                        done_flag = 1;                                          /* we are done */
		}
                strcpy(line2, line);

                if ((p = strchr(line, conf_sep)))                               /* if there is a seperator character */
                        *p = '\0';                                              /* line is now just key */

                conf_trim(line);                                                     /* remove unnecessary whitespace */

                if (strcasecmp(line, key) || done_flag)                         /* not the entry we want or already done */
                {
                        sprintf(lineout, "%s", line2);
			
			if (comment)
			{
				char temp[CONF_MAX_LINE_LEN];
					
				if (strlen(lineout) < conf_comment_column)
				{
					memset(temp, 0, sizeof(temp));
					memset(temp, ' ', conf_comment_column - strlen(lineout) - 1);
				}
				else
				{
				    strcpy(temp, " ");
				}
						
				sprintf(lineout + strlen(lineout), "%s%c%s",
					temp, conf_comment_sep, comment);
			}
			
			sprintf(lineout + strlen(lineout), "\n");
			
			fprintf(ifp2, "%s", lineout);
			
                        continue;                                               /* go to the next line */
                }

                /* hmmm, this must be the right entry? */
                /* don't write line to file then */

        } /* while (fgets(line, sizeof(line), ifp)) */

        fclose(ifp);                                                            /* close files */
        fclose(ifp2);
#ifdef _WIN32
	unlink(configfile);							/* can't rename to a file that exists? */ 
#endif
        if (rename(configfile2, configfile) == -1)                              /* if can't rename temp file to real file */
            return CONF_ERR_FILE;
        
        chmod(configfile, stat_buf.st_mode);                                    /* restore original permissions */

        return CONF_SUCCESS;                                                    /* key was not found */
} /* delconf() */

/*
    End of "$Id: conf_del.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
