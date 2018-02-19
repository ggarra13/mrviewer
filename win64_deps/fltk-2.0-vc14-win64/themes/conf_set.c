/* CET - FIXME - This function is currently semi-broken :-( */

/*
   "$Id: conf_set.c 8500 2011-03-03 09:20:46Z bgbnbigben $"

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

/* this function returns the proper amount of leading indentation for a line */
static const char* level_indent(int l) {
  static char indent[CONF_MAX_LEVEL * CONF_INDENT + 1];
  memset(indent, 0, sizeof(indent));
  if (l <= CONF_MAX_LEVEL) memset(indent, ' ', l * CONF_INDENT);

  return indent;
}

/*
        int setconf(const char *configfile, const char *key, const char *svalue)

        description:
                sets the string associated with a key in a config file
        arguments:
                configfile: path of config file
                key: section/key to set
                svalue: string associated with key
        return values:
                returns 0 for OK or error code defined in conf.h
        notes:
		BEWARE: big, ugly, nasty function!  NOT efficient code!
		I really suck for writing something that looks like this!
		This is what happens when you keep adding to code for a
		period of years with no real foresight.  Mea culpa!!!
                Please excuse the ugliness of this function.  I will
                rewrite it eventually, but it seems to work for now...
*/

int setconf(const char *configfile, const char *k, const char *svalue) {
        int             done_flag;                                              /* has entry already been written? */
        int             newsec_flag = 0;                                        /* are we in a new section? */
        char            configfile2[CONF_MAXPATHLEN + 6];                       /* file name for old config file */
        FILE            *ifp;                                                   /* file pointer for config file */
        FILE            *ifp2;                                                  /* file pointer for new config file */
        char            line[CONF_MAX_LINE_LEN];                                /* line buffer */
        char            line2[CONF_MAX_LINE_LEN];                               /* temporary line buffer */
        char            lineout[CONF_MAX_LINE_LEN];		                /* temporary output buffer */
        char            base_section[CONF_MAX_LEVEL][CONF_MAX_SECT_LEN];        /* parent section of this section */
        char            base_section2[CONF_MAX_LEVEL][CONF_MAX_SECT_LEN];       /* parent section with [] added */
        int             bsl[CONF_MAX_LEVEL];
        char            *p, *p2, *p3;                                           /* miscelaneous char pointer */
        struct stat     stat_buf;                                               /* buffer for stat info */
        int             new_flag;                                               /* does the config file already exist? */
        int             section_flag = 1;                                       /* section given */
        int             i, level = 0, current_level = 0, last_level;
	char            *comment = 0;                                           /* the comment found on the line */
        char            keysect[CONF_MAX_SECT_LEN], *key;
	const char* section;             /* key, section, and both */

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
                stat(configfile, &stat_buf);                                    /* get original permisson info */

        if (section && *section)
        {
                strcpy(base_section[0], section);                               /* copy current section into base section */
                sprintf(base_section2[0], "[%s]", base_section[0]);
        }
        else
        {
                strcpy(base_section[0], "");
                section_flag = 0;
        }
        bsl[0] = conf_strcnt(base_section[0], conf_level_sep) + 1;

        last_level = 0;
        for (i = 1; i < CONF_MAX_LEVEL; i++)
        {
                strcpy(base_section[i], base_section[i - 1]);                   /* copy current section into base section */
                if ((p = strrchr(base_section[i], conf_level_sep)))
                {
                        last_level = i;
                        *p = (char)0;
                        sprintf(base_section2[i], "[%s]", base_section[i]);
                        bsl[i] = conf_strcnt(base_section[i], conf_level_sep) + 1;
                }
                else
                {
                        *base_section[i] = (char)0;
                        *base_section2[i] = (char)0;
                        bsl[i] = 0;
                }
        }

        if (conf_strcnt(base_section[CONF_MAX_LEVEL - 1], conf_level_sep))
        {
                unlink(configfile2);
                return CONF_ERR_DEPTH;
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
                if (!new_flag)                                                  /* if the file exists but I can't read it */
                {
                        fclose(ifp2);
                        unlink(configfile2);
                        return CONF_ERR_FILE;
                }

                if (section_flag)
                {
                        for (i = last_level; i >= 0; i--)
                        {
                                fprintf(ifp2, "\n%s%s\n",
					level_indent(bsl[i]), base_section2[i]);/* add base sections to new config file */
                        }
                }

                if (svalue)
                        sprintf(line2, "%s%s %c %s\n",
                                level_indent(bsl[0]),
                                key, conf_sep, svalue);                         /* create new entry */
                else
                        sprintf(line2, "%s%s\n",
                                level_indent(bsl[0]),
                                key);                                           /* create new entry */

                fputs(line2, ifp2);
                fclose(ifp2);
                if (rename(configfile2, configfile) == -1)                      /* if can't rename temp file to real file */
                {
                    unlink(configfile2);
                    return CONF_ERR_FILE;
                }
                
                return CONF_SUCCESS;
        } /* if (!ifp) */

        if (section_flag)
        {
                for (i = 0; i <= last_level; i++)
                {
                        rewind(ifp);
                        while ((p = fgets(line, sizeof(line), ifp)))            /* keep reading lines until section found */
			{
				if ((p2 = strchr(line, conf_comment_sep)))      /* if comment found */
					*p2 = (char)0;				/* kill it */

                                if (!strcasecmp(conf_trim(line), base_section2[i]))/* if this is the section we are looking for */
					break;					/* stop looking */
			}
                        if (p)							/* if we found the first parent section */
				break;						/* stop looking */
                }

                level = i;							/* index of the closest parent section */
        }

        rewind(ifp);								/* start from top of config file */

        if ((level != (last_level + 1)) && section_flag)			/* if parent section found & we want a section */
        {
                current_level = 0;

                *line = (char)0;
                do
                {
printf("cl: %d\n", current_level);			
			comment = 0;
			
                        if (!fgets(line, sizeof(line), ifp))			/* if no more lines in config file */
				break;						/* stop looking */

			/* if a comment found */
			if ((p2 = strchr(line, conf_comment_sep)) &&
			    (p2 == line + strspn(line, CONF_WHITESPACE) ||
			     strchr(CONF_WHITESPACE, *(p2 + 1))))
			{
				*p2 = (char)0;					/* kill the comment */
				comment = ++p2;					/* but remember it */
				conf_endtrim(comment);
			}
			
                        conf_trim(line);					/* remove unnecessary whitespace */
			
                        if (!strcmp(line, ""))					/* if there is no key on this line */
			{
printf("line with no key: %s\n", line);				
				if (comment) {					/* line with only comment */
	                                fprintf(ifp2, "%s%c%s\n",
    	                                	level_indent(current_level),
						conf_comment_sep, comment);     /* put line in new config file */
				}
				continue;
			}

                        if (*line == '[')
                        {
printf("line beginning section: %s\n", line);				
                                current_level = conf_strcnt(line, conf_level_sep) + 1;
				fprintf(ifp2, "\n");
                                sprintf(lineout, "%s%s",
                                        level_indent(current_level - 1), line);
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
                        else
                        {
printf("c: %s\n", comment);				
                                sprintf(lineout, "%s%s",
                                    level_indent(current_level),
                                    line);                                      /* write old lines to new config file */
				
				if (comment)
				{
					char temp[CONF_MAX_LINE_LEN];
printf("4c: %s\n", comment);				
					
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
                while (strcasecmp(line, base_section2[level]));                 /* read lines until closest parent section found */

        }
        else if(section_flag)
        {
                current_level = 0;

                while (fgets(line, sizeof(line), ifp))                          /* keep reading lines until section found */
                {
			comment = 0;
			
			/* if a comment found */
			if ((p2 = strchr(line, conf_comment_sep)) &&
			    (p2 == line + strspn(line, CONF_WHITESPACE) ||
			     strchr(CONF_WHITESPACE, *(p2 + 1))))
			{
				*p2 = (char)0;					/* kill the comment */
				comment = ++p2;					/* but remember it */
				conf_endtrim(comment);
			}
			
                        conf_trim(line);					/* remove unnecessary whitespace */
			
                        if (!strcmp(line, ""))					/* if there is no key on this line */
			{
				if (comment)					/* line with only comment */
	                                fprintf(ifp2, "%s%c%s\n",
    	                                	level_indent(current_level),
						conf_comment_sep, comment);     /* put line in new config file */
				continue;
			}

                        if (*line == '[')
                        {
                                current_level = conf_strcnt(line, conf_level_sep) + 1;
				fprintf(ifp2, "\n");
                                sprintf(lineout, "%s%s",
                                        level_indent(current_level - 1), line);
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
			else
                        {
                                sprintf(lineout, "%s%s",
                                    level_indent(current_level),
                                    line);                                      /* write old lines to new config file */
				
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
        }

        if (level != 0)
        {
                while (fgets(line, sizeof(line), ifp))                          /* keep reading lines until section found */
                {
			comment = 0;
			
			/* if a comment found */
			if ((p2 = strchr(line, conf_comment_sep)) &&
			    (p2 == line + strspn(line, CONF_WHITESPACE) ||
			     strchr(CONF_WHITESPACE, *(p2 + 1))))
			{
				*p2 = (char)0;					/* kill the comment */
				comment = ++p2;					/* but remember it */
				conf_endtrim(comment);

			}
			
                        conf_trim(line);					/* remove unnecessary whitespace */
			
                        if (!strcmp(line, ""))					/* if there is no key on this line */
			{
				if (comment)					/* line with only comment */
	                                fprintf(ifp2, "%s%c%s\n",
    	                                	level_indent(current_level),
						conf_comment_sep, comment);     /* put line in new config file */
				continue;
			}

                        if (*line == '[')
                        {
                                newsec_flag = 1;
                                break;
                        }
                        else
                        {
                                sprintf(lineout, "%s%s",
                                    level_indent(current_level),
                                    line);                                      /* write old lines to new config file */
				
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
        }

        for (i = level; i > 0; i--)
        {
                fprintf(ifp2, "\n%s%s\n", level_indent(bsl[i - 1]),
                        base_section2[i - 1]);
        }

        current_level = bsl[0];

        done_flag = 0;

        p = line;

        do
        {
                if (!newsec_flag)
                {
			comment = 0;
			
                        *line = (char)0;
                        p = fgets(line, sizeof(line), ifp);
                }
                else
                {
                        newsec_flag = 0;
                }
		
		/* if a comment found */
                /*
                   hack so that comment seperator can occur in value
                   if not immediately followed by a whitespace
                */
		for (p2 = line; (p3 = strchr(p2, conf_comment_sep)); p2 = p3 + 1)
		{
		        if (strchr(CONF_WHITESPACE, *(p3 + 1)) ||
		            line + strspn(line, CONF_WHITESPACE) == p3)
		        {
		                *p3 = (char)0;					/* kill the comment */
			        comment = ++p2;					/* but remember it */
			        conf_endtrim(comment);
		                break;
		        }
		}
			
                conf_trim(line);						/* remove unnecessary whitespace */
			
                if (!strcmp(line, ""))						/* if there is no key on this line */
		{
			if (comment)						/* line with only comment */
	                        fprintf(ifp2, "%s%c%s\n",
    	                        	level_indent(current_level),
					conf_comment_sep, comment);             /* put line in new config file */
			continue;
		}

                if (*line == '[')
                {
			if (!done_flag)
			{
                                fprintf(ifp2, "%s%s",
                                    level_indent(current_level),
                                    key);

                                if (svalue)
                                    fprintf(ifp2, " %c %s", conf_sep, svalue);

                                fprintf(ifp2, "\n");                            /* create new entry */

                                done_flag = 1;                                  /* entry is written */
			}
			
                        current_level = conf_strcnt(line, conf_level_sep) + 1;
			
			fprintf(ifp2, "\n");
                        sprintf(lineout, "%s%s",
                                level_indent(current_level - 1), line);
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
			
			continue;
		}
		
		if (done_flag)		
                {
                        sprintf(lineout, "%s%s",
            	                level_indent(current_level),
                                line);                                      /* write old lines to new config file */
				
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
			
			continue;
                }
		
                strcpy(line2, line);
                if ((p2 = strchr(line, conf_sep)))
                        *p2 = '\0';                                             /* line is now just key */
                conf_trim(line);                                                /* remove unnecessary whitespace */
                if (strcasecmp(line, key))                                      /* not the entry we want */
                {
                        sprintf(lineout, "%s%s",
            	                level_indent(current_level),
                                line2);                                      /* write old lines to new config file */
				
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
			
			continue;
                }
		
                /* hmmm, this must be the right entry? */
                sprintf(lineout, "%s%s",
                        level_indent(current_level),
                        key);

                if (svalue)
                	sprintf(lineout + strlen(lineout), " %c %s",
				conf_sep, svalue);

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
		
                sprintf(lineout + strlen(lineout), "\n");                       /* build the new line */
		fprintf(ifp2, "%s", lineout);

                done_flag = 1;                                                  /* line has been written */

        }
        while (p);                                                              /* while there are still lines in the file */

        if (!done_flag)                                                         /* line hasn't been written yet */
        {
                fprintf(ifp2, "%s%s",
                        level_indent(current_level),
                        key);

                if (svalue)
                        fprintf(ifp2, " %c %s", conf_sep, svalue);

                fprintf(ifp2, "\n");                                            /* build the new line */
        }

        fclose(ifp);                                                            /* close files */
        fclose(ifp2);
#ifdef _WIN32
	unlink(configfile);							/* can't rename to a file that exists? */ 
#endif
        if (rename(configfile2, configfile) == -1)                              /* if can't rename temp file to real name */
            return CONF_ERR_FILE;
        
        chmod(configfile, stat_buf.st_mode);                                    /* restore original permissions */
        return CONF_SUCCESS;                                                    /* key was not found */
} /* setconf() */


/*
    End of "$Id: conf_set.c 8500 2011-03-03 09:20:46Z bgbnbigben $".
*/
