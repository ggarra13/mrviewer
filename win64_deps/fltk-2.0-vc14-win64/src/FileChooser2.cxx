//
// "$Id: FileChooser2.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// More FileChooser routines.
//
// Copyright 1999-2006 by Michael Sweet.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
// Contents:
//
//   FileChooser::count()                     - Return the number of selected files.
//   FileChooser::directory()                 - Set the directory in the file chooser.
//   FileChooser::filter()                    - Set the filter(s) for the chooser.
//   FileChooser::newdir()                    - Make a new directory.
//   FileChooser::value()                     - Return a selected filename.
//   FileChooser::rescan()                    - Rescan the current directory.
//   FileChooser::favoritesButtonCB()         - Handle favorites selections.
//   FileChooser::fileListCB()                - Handle clicks (and double-clicks)
//                                                  in the FileBrowser.
//   FileChooser::fileNameCB()                - Handle text entry in the FileBrowser.
//   FileChooser::showChoiceCB()              - Handle show selections.
//   FileChooser::activate_okButton_if_file() - Ungrey the OK button if the user
//                                                  chooses a file
//   compare_dirnames()                   - Compare two directory names.
//   quote_pathname()                     - Quote a pathname for a menu.
//   unquote_pathname()                   - Unquote a pathname from a menu.
//

//
// Include necessary headers.
//
#include <fltk/FileChooser.h>
#include <fltk/filename.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/Cursor.h>
#include <fltk/SharedImage.h>
#include <fltk/Item.h>
#include <fltk/string.h>
#include <fltk/utf.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32) && ! defined (__CYGWIN__)
#  include <direct.h>
#  include <io.h>
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#  define access _access
#  define mkdir _mkdir
// Apparently Borland C++ defines DIRECTORY in <direct.h>, which
// interfers with the FileIcon enumeration of the same name.
#  ifdef DIRECTORY
#    undef DIRECTORY
#  endif // DIRECTORY
#else
#  include <unistd.h>
#  include <pwd.h>
#endif /* WIN32 */

using namespace fltk;

/** \class fltk::FileChooser
  The fltk::FileChooser widget displays a standard file selection
  dialogue that supports various selection modes.

  \image html FileChooser.jpg
  \image latex FileChooer.jpg "FileChooser" width=12cm

  The fltk::FileChooser class also exports several static values
  that may be used to localise or customise the appearance of all file chooser
  dialogues:

  <CENTER><TABLE BORDER="1">
    <TR>
      <TH>Member</TH>
      <TH>Default value</TH>
    </TR>
    <TR>
      <TD>add_favorites_label</TD>
      <TD>"Add to Favorites"</TD>
    </TR>
    <TR>
      <TD>all_files_label</TD>
      <TD>"All Files (*)"</TD>
    </TR>
    <TR>
      <TD>custom_filter_label</TD>
      <TD>"Custom Filter"</TD>
    </TR>
    <TR>
      <TD>existing_file_label</TD>
      <TD>"Please choose an existing file!"</TD>
    </TR>
    <TR>
      <TD>favorites_label</TD>
      <TD>"Favorites"</TD>
    </TR>
    <TR>
      <TD>filename_label</TD>
      <TD>"Filename:"</TD>
    </TR>
    <TR>
      <TD>filesystems_label</TD>
      <TD>"My Computer" (WIN32)<BR>
          "File Systems" (all others)</TD>
    </TR>
    <TR>
       <TD>hidden_label</TD>
       <TD>"Show hidden files:"</TD>
    </TR>
    <TR>
       <TD>manage_favorites_label</TD>
       <TD>"Manage Favorites"</TD>
    </TR>
    <TR>
       <TD>new_directory_label</TD>
       <TD>"New Directory?"</TD>
    </TR>
    <TR>
       <TD>new_directory_tooltip</TD>
       <TD>"Create a new directory."</TD>
    </TR>
    <TR>
       <TD>preview_label</TD>
       <TD>"Preview"</TD>
    </TR>
    <TR>
       <TD>save_label</TD>
       <TD>"Save"</TD>
    </TR>
    <TR>
       <TD>show_label</TD>
       <TD>"Show:"</TD>
    </TR>
    <TR>
       <TD>sort</TD>
       <TD>casenumericsort</TD>
    </TR>
	<TR>
	   <TD>sort_menu_label</TD>
	   <TD>"Sort Method"</TD>
	</TR>
    </TABLE></CENTER>

  The fltk::FileChooser::sort member specifies the sort function that is 
  used when loading the contents of a directory and can be customised
  at run-time.

  For more complex customisation, considering copying the fltk::FileChooser
  code and modifying it accordingly.
*/

//
// File chooser label strings and sort function...
//

Preferences	FileChooser::prefs_(Preferences::USER, "fltk.org", "filechooser");

const char	*FileChooser::add_favorites_label = "Add to Favorites";
const char	*FileChooser::all_files_label = "All Files (*)";
const char	*FileChooser::custom_filter_label = "Custom Filter";
const char	*FileChooser::existing_file_label = "Please choose an existing file!";
const char	*FileChooser::favorites_label = "Favorites";
const char	*FileChooser::sort_menu_label = "Sort method";
const char	*FileChooser::filename_label = "Filename:";
#ifdef WIN32
const char	*FileChooser::filesystems_label = "My Computer";
#else
const char	*FileChooser::filesystems_label = "File Systems";
#endif // WIN32
const char	*FileChooser::manage_favorites_label = "Manage Favorites";
const char	*FileChooser::new_directory_label = "New Directory?";
const char	*FileChooser::new_directory_tooltip = "Create a new directory.";
const char	*FileChooser::preview_label = "Preview";
const char	*FileChooser::save_label = "Save";
const char	*FileChooser::show_label = "Show:";
FileSortF	*FileChooser::sort = fltk::casenumericsort;


//
// Local functions...
//

static int	compare_dirnames(const char *a, const char *b);
static void	quote_pathname(char *, const char *, int);
static void	unquote_pathname(char *, const char *, int);


/**
  Calculate the number of selected files.
  
  \return the number of selected files.
*/

int FileChooser::count() {
  int		i;		// Looping var
  int		fcount;		// Number of selected files
  const char	*filename;	// Filename in input field or list


  filename = fileName->text();

  if (!(type_ & FileChooser::MULTI)) {
    // Check to see if the file name input field is blank...
    if (!filename || !filename[0]) return 0;
    else return 1;
  }

  for (i = 1, fcount = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i)) {
      // See if this file is a directory...
      filename = (char *)fileList->child(i-1)->label();

      if (filename[strlen(filename) - 1] != '/')
	fcount ++;
    }

  if (fcount) return fcount;
  else if (!filename || !filename[0]) return 0;
  else return 1;
}

/**
  Set the directory in the file chooser.

  Modify the FileChooser's current directory.

  \param d The directory to change to
  \return void
  \see directory(const char* d, bool f)
*/

void FileChooser::directory(const char *d) {
  return directory(d, true);
}

/**
  Modifies the FileChooser's directory, with the option to modify the displayed "name" field

  Updates the FileChooser's current directory by making the filename absolute, stripping trailing '..' or '.'. This also keeps the 'suggested filename' of a file as directories are traversed.
  
  \param d A const char* containing the FileChooser's new directory
  \param f A boolean representing whether or not to update the displayed filename
  \return void
*/

void FileChooser::directory(const char *d, bool f) {
  char	*dirptr;			// Pointer into directory

  // NULL == current directory
  if (d == NULL)
    d = ".";

#ifdef WIN32
  // See if the filename contains backslashes...
  char	*slash;				// Pointer to slashes
  char	fixpath[1024];			// Path with slashes converted
  if (strchr(d, '\\')) {
    // Convert backslashes to slashes...
    strlcpy(fixpath, d, sizeof(fixpath));

    for (slash = strchr(fixpath, '\\'); slash; slash = strchr(slash + 1, '\\'))
      *slash = '/';

    d = fixpath;
  }
#endif // WIN32
  if (d[0] != '\0')
  {
    // Make the directory absolute...
#if (defined(WIN32) && ! defined(__CYGWIN__))|| defined(__EMX__)
    if (d[0] != '/' && d[0] != '\\' && d[1] != ':')
#else
    if (d[0] != '/' && d[0] != '\\')
#endif /* WIN32 || __EMX__ */
      fltk::filename_absolute(directory_, sizeof(directory_), d);
    else
      strlcpy(directory_, d, sizeof(directory_));


    // Strip any trailing slash...
    dirptr = directory_ + strlen(directory_) - 1;
    if ((*dirptr == '/' || *dirptr == '\\') && dirptr > directory_)
      *dirptr = '\0';

    // See if we have a trailing .. or . in the filename...
    dirptr = directory_ + strlen(directory_) - 3;
    if (dirptr >= directory_ && strcmp(dirptr, "/..") == 0) {
      // Yes, we have "..", so strip the trailing path...
      *dirptr = '\0';
      while (dirptr > directory_) {
        if (*dirptr == '/') break;
	dirptr --;
      }

      if (dirptr >= directory_ && *dirptr == '/')
        *dirptr = '\0';
    } else if ((dirptr + 1) >= directory_ && strcmp(dirptr + 1, "/.") == 0) {
      // Strip trailing "."...
      dirptr[1] = '\0';
    }

  }
  else
    directory_[0] = '\0';

  if (f) {
    // Update the current filename accordingly...
    char pathname[1024];	// New pathname for filename field

    strlcpy(pathname, directory_, sizeof(pathname));
    if ((pathname[0] && pathname[strlen(pathname) - 1] != '/') || !pathname[0])
      strlcat(pathname, "/", sizeof(pathname));
    // Prevent users from cursing us: keep basename, if not a directory
    if (!fltk::filename_isdir(fileName->text())) {
      dirptr = strchr(pathname, 0);
      strlcat(pathname, fltk::filename_name(fileName->text()), sizeof(pathname));
      if (!(type_ & CREATE) && !fltk::filename_isfile(pathname))
        *dirptr = 0;
    }
   
    fileName->text(pathname);
  }

  if (shown()) {
    // Rescan the directory...
    rescan();
  }
}


/**
  'FileChooser::favoritesButtonCB()' - Handle favorites selections.
  
   This function deals with adding and/or removing favorites from the FileChooser's drop down menu. It allows a user to quickly navigate amongst pre-set or personalised favorite directories.

   \return void
*/

void FileChooser::favoritesButtonCB() {
  int		v;			// Current selection
  char		pathname[1024],		// Pathname
		menuname[2048];		// Menu name


  v = favoritesButton->value();

  if (!v) {
    // Add current directory to favorites...
    if (getenv("HOME")) v = favoritesButton->size() - 5;
    else v = favoritesButton->size() - 4;

    sprintf(menuname, "favorite%02d", v);

    prefs_.set(menuname, directory_);
	prefs_.flush();

    quote_pathname(menuname, directory_, sizeof(menuname));
    favoritesButton->add(menuname);

    if (favoritesButton->size() > 104) {
      favoritesButton->child(0)->deactivate();
    }
  } else if (v == 1) {
    // Manage favorites...
    favoritesCB(0);
  } else if (v == 3) {
    // Filesystems/My Computer
    directory("");
  } else {
    unquote_pathname(pathname, favoritesButton->child(v)->label(), sizeof(pathname));
    directory(pathname);
    fileList->deselect();
  }
}


/**
  Handle the favorites window

  This function creates, populates and navigates the favorites window, opened when a user clicks "Manage Favorites". It allows users to shuffle their favorites in the order of most-used or remove favorites.

  \param w The clicked-on (or used) widget
  \see update_favorites()
  \return void
*/

void FileChooser::favoritesCB(Widget *w) {
  int		i;			// Looping var
  char		name[32],		// Preference name
		pathname[1024];		// Directory in list


  if (!w) {
    // Load the favorites list...
    favList->clear();
    favList->deselect();

    for (i = 0; i < 100; i ++) {
      // Get favorite directory 0 to 99...
      sprintf(name, "favorite%02d", i);

      prefs_.get(name, pathname, "", sizeof(pathname));

      // Stop on the first empty favorite...
      if (!pathname[0]) break;

      // Add the favorite to the list...
      favList->add(pathname,FileIcon::find(pathname, FileIcon::DIRECTORY));
    }

    favUpButton->deactivate();
    favDeleteButton->deactivate();
    favDownButton->deactivate();
    favOkButton->deactivate();

    favWindow->hotspot(favList);
    favWindow->show();
  } else if (w == favList) {
    i = favList->value();
    if (i>=0) {
      if (i >= 1) favUpButton->activate();
      else favUpButton->deactivate();

      favDeleteButton->activate();

      if (i < favList->size()-1) favDownButton->activate();
      else favDownButton->deactivate();
    } else {
      favUpButton->deactivate();
      favDeleteButton->deactivate();
      favDownButton->deactivate();
    }
  } else if (w == favUpButton) {
    i = favList->value();

    favList->Menu::insert(*favList->child(i),i-1);
    //favList->remove(i + 1);
    favList->select(i - 1);

    if (i == 1) favUpButton->deactivate();

    favDownButton->activate();

    favOkButton->activate();
  } else if (w == favDeleteButton) {
    i = favList->value();

    favList->remove(i);

    if (i > favList->size()) i --;
    favList->select(i);

    if (i < favList->size()) favDownButton->activate();
    else favDownButton->deactivate();

    if (i > 1) favUpButton->activate();
    else favUpButton->deactivate();

    if (!i) favDeleteButton->deactivate();

    favOkButton->activate();
  } else if (w == favDownButton) {
    i = favList->value();

    favList->Menu::insert(*favList->child(i),i+2);
    //favList->insert(i + 2, favList->child(i)->label(), favList->child(i)->user_data());
    //favList->remove(i);
    favList->select(i + 1);

    if ((i+1) == favList->size()-1) favDownButton->deactivate();

    favUpButton->activate();

    favOkButton->activate();
  } else if (w == favOkButton) {
    // Copy the new list over...
    for (i = 0; i < favList->size(); i ++) {
      // Set favorite directory 0 to 99...
      sprintf(name, "favorite%02d", i);

      prefs_.set(name, favList->child(i)->label());
    }

    // Clear old entries as necessary...
    for (; i < 100; i ++) {
      // Clear favorite directory 0 to 99...
      sprintf(name, "favorite%02d", i);

      prefs_.get(name, pathname, "", sizeof(pathname));

      if (pathname[0]) prefs_.set(name, "");
      else break;
    }

    update_favorites();

    favWindow->hide();
  }
}

/**
  Handle clicks (and double-clicks) in the FileBrowser.

  This function interprets single clicks and double clicks - a single click selects a file whereas a double click either traverses into a directory or assumes the user has selected the file they want

  \return void
*/

void FileChooser::fileListCB() {
  char	*filename,			// New filename
	pathname[1024];			// Full pathname to file

 int cval = fileList->value();
 if (cval<0  || !(filename = (char *)fileList->child(cval)->label())) return;

  if (!directory_[0]) {
    strlcpy(pathname, filename, sizeof(pathname));
  } else if (strcmp(directory_, "/") == 0) {
    snprintf(pathname, sizeof(pathname), "/%s", filename);
  } else {
    snprintf(pathname, sizeof(pathname), "%s/%s", directory_, filename);
  }

  if (fltk::event_clicks()) {
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
    if ((strlen(pathname) == 2 && pathname[1] == ':') ||
        fltk::filename_isdir(pathname))
#else
    if (fltk::filename_isdir(pathname))
#endif /* WIN32 || __EMX__ */
    {
      // Change directories...
      directory(pathname, true);

      // Reset the click count so that a click in the same spot won't
      // be treated as a triple-click.  We use a value of -1 because
      // the next click will increment click count to 0, which is what
      // we really want...
      //fltk::event_clicks(-1); // fabien: doesn't seems to be useful
      fileList->deselect();	// after directory the file is already 
				// selected from previous state so deselect it
    } else {
      // Hide the window - picked the file...
      window->hide();
    }
  } else {
    // Check if the user clicks on a directory when picking files;
    // if so, make sure only that item is selected...
    filename = pathname + strlen(pathname) - 1;

    if ((type_ & FileChooser::MULTI) && !(type_ & DIRECTORY)) {
      if (*filename == '/') {
	// Clicked on a directory, deselect everything else...
	fileList->select_only_this();
      } else {
        // Clicked on a file - deselect any directories
        for (int i = 0; i < fileList->size(); i++) {
          fileList->goto_index(i);
          if (fileList->item()->selected()) {
            const char* temp = fileList->item()->label();
            temp += strlen(temp)-1;
            if (*temp == '/') // this is a directory!
              fileList->set_item_selected(false);
          }
        }
      }
    }
    // Strip any trailing slash from the directory name...
    if (*filename == '/') *filename = '\0';

    if (!fltk::filename_isfile(pathname)) {
      filename = strrchr((char*)fileName->text(), '/');
      if (filename && filename+1) filename++;
      sprintf(pathname, "%s%s%s", pathname, filename ? "/" : "", filename ? filename : "");
    }
    fileName->value(pathname);

    // Update the preview box...
    fltk::remove_timeout((TimeoutHandler)previewCB, this);
    fltk::add_timeout(0.5, (TimeoutHandler)previewCB, this);

    // Do any callback that is registered...
    if (callback_) (*callback_)(this, data_);

    // Activate the OK button as needed...
    if (!fltk::filename_isdir(fileName->text()) || (type_ & DIRECTORY))
      okButton->activate();
    else
      okButton->deactivate();
  }
}


/**
  Handle text entry in the FileChooser.

  If a user tries to manually enter a path into the FileChooser's input box, instead of using the FileChooser to navigate, this function will make sure what is entered is in line with what is required by the FileChooser - for instance, it expands ~ to the user's home directory. It also attempts to offer tabcompletion for paths and files.

  \return void
*/
void FileChooser::fileNameCB() {
  char		*filename,	// New filename
		*slash,		// Pointer to trailing slash
		pathname[1024],	// Full pathname to file
		matchname[256];	// Matching filename
  int		i,		// Looping var
		min_match,	// Minimum number of matching chars
		max_match,	// Maximum number of matching chars
		num_files,	// Number of files in directory
		first_line;	// First matching line
  const char	*file;		// File from directory

  // Get the filename from the text field...
  filename = (char *)fileName->text();

  if (!filename || !filename[0]) {
    okButton->deactivate();
    return;
  }

  // Expand ~ and $ variables as needed...
  if (strchr(filename, '~') || strchr(filename, '$')) {
    fltk::filename_absolute(pathname, sizeof(pathname), filename);
    filename = pathname;
    value(pathname);
  }

  // Make sure we have an absolute path...
#if (defined(WIN32) && !defined(__CYGWIN__)) || defined(__EMX__)
  if (directory_[0] != '\0' && filename[0] != '/' &&
      filename[0] != '\\' &&
      !(isalpha(filename[0] & 255) && (!filename[1] || filename[1] == ':'))) {
#else
  if (directory_[0] != '\0' && filename[0] != '/') {
#endif /* WIN32 || __EMX__ */
    fltk::filename_absolute(pathname, sizeof(pathname), filename);
    value(pathname);
    fileName->position(fileName->mark()); // no selection after expansion
  } else if (filename != pathname) {
    // Finally, make sure that we have a writable copy...
    strlcpy(pathname, filename, sizeof(pathname));
  }

  filename = pathname;

  // Now process things according to the key pressed...
  if (fltk::event_key() == ReturnKey || fltk::event_key() == KeypadEnter) {
    // Enter pressed - select or change directory...
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
    if ((isalpha(pathname[0] & 255) && pathname[1] == ':' && !pathname[2]) ||
        fltk::filename_isdir(pathname) &&
	compare_dirnames(pathname, directory_)) {
#else
    if (fltk::filename_isdir(pathname) &&
	compare_dirnames(pathname, directory_)) {
#endif /* WIN32 || __EMX__ */
      directory(pathname, false);
    } else if ((type_ & CREATE) || access(pathname, 0) == 0) {
      if (!fltk::filename_isdir(pathname) || (type_ & DIRECTORY)) {
	// Update the preview box...
	update_preview();

	// Do any callback that is registered...
	if (callback_) (*callback_)(this, data_);

	// Hide the window to signal things are done...
	window->hide();
      }
    } else {
      // File doesn't exist, so beep at and alert the user...
      fltk::alert(existing_file_label);
    }
  }
    else if (fltk::event_key() != fltk::DeleteKey &&
           fltk::event_key() != fltk::BackSpaceKey) {
    // Check to see if the user has entered a directory...
    if ((slash = strrchr(pathname, '/')) == NULL)
      slash = strrchr(pathname, '\\');

    if (!slash) return;

    // Yes, change directories if necessary...
    *slash++ = '\0';
    filename = slash;

#if defined(WIN32) || defined(__EMX__)
    if (strcasecmp(pathname, directory_) &&
        (pathname[0] || strcasecmp("/", directory_))) {
#else
    if (strcmp(pathname, directory_) &&
        (pathname[0] || strcasecmp("/", directory_))) {
#endif // WIN32 || __EMX__
      int p = fileName->position();
      int m = fileName->mark();

      directory(pathname, false);

      if (filename[0]) {
	char tempname[1024];

	snprintf(tempname, sizeof(tempname), "%s/%s", directory_, filename);
	fileName->text(tempname);
	strlcpy(pathname, tempname, sizeof(pathname));
      }

      fileName->position(p, m);
    }

    // Other key pressed - do filename completion as possible...
    num_files  = fileList->size();
    min_match  = strlen(filename);
    max_match  = min_match + 1;
    first_line = 0;

    for (i = 1; i <= num_files && max_match > min_match; i ++) {
      file = fileList->child(i-1)->label();

#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
      if (strncasecmp(filename, file, min_match) == 0) {
#else
      if (strncmp(filename, file, min_match) == 0) {
#endif // WIN32 || __EMX__
        // OK, this one matches; check against the previous match
	if (!first_line) {
	  // First match; copy stuff over...
	  strlcpy(matchname, file, sizeof(matchname));
	  max_match = strlen(matchname);

          // Strip trailing /, if any...
	  if (matchname[max_match - 1] == '/') {
	    max_match --;
	    matchname[max_match] = '\0';
	  }

	  // And then make sure that the item is visible
          fileList->topline(i);
	  first_line = i;
	} else {
	  // Succeeding match; compare to find maximum string match...
	  while (max_match > min_match)
#if (defined(WIN32) && ! defined(__CYGWIN__)) || defined(__EMX__)
	    if (strncasecmp(file, matchname, max_match) == 0)
#else
	    if (strncmp(file, matchname, max_match) == 0)
#endif // WIN32 || __EMX__
	      break;
	    else
	      max_match --;

          // Truncate the string as needed...
          matchname[max_match] = '\0';
	}
      }
    }

    // If we have any matches, add them to the input field...
    if (first_line > 0 && min_match == max_match &&
        max_match == (int)strlen(fileList->child(first_line-1)->label())) {
      // This is the only possible match...
      fileList->deselect(0);
      fileList->select(first_line - 1);
      fileList->redraw();
    } else if (max_match > min_match && first_line) {
      // Add the matching portion...
      fileName->replace(filename - pathname, filename - pathname + min_match,
                        matchname,strlen(matchname));

      // Highlight it with the cursor at the end of the selection so
      // s/he can press the right arrow to accept the selection
      // (Tab and End also do this for both cases.)
      fileName->position(filename - pathname + max_match,
	                 filename - pathname + min_match);
    } else if (max_match == 0) {
      fileList->deselect(0);
      fileList->redraw();
    }

    // See if we need to enable the OK button...
    activate_okButton_if_file();

  } else {
    // fltk::DeleteKey or fltk::BackSpace
    fileList->deselect(0);
    fileList->redraw();
    activate_okButton_if_file();
  }
}


/**
  Set the filter(s) for the FileChooser.

  This function allows the FileChooser to use globs and (eventually) regular expressions to select multiple files of the same filetype or same filename.

  \param p This const char* represents the pattern to match files against.
  \todo Full Regex matchings
  \return void
*/

void FileChooser::filter(const char *p) {
  char		*copyp,				// Copy of pattern
		*start,				// Start of pattern
		*end;				// End of pattern
  int		allfiles;			// Do we have a "*" pattern?
  char		temp[1024];			// Temporary pattern string


  // Make sure we have a pattern...
  if (!p || !*p) p = "*";

  // Copy the pattern string...
  copyp = strdup(p);

  // Separate the pattern string as necessary...
  showChoice->clear();

  for (start = copyp, allfiles = 0; start && *start; start = end) {
    end = strchr(start, '\t');
    if (end) *end++ = '\0';

    if (strcmp(start, "*") == 0) {
      showChoice->add(all_files_label);
      allfiles = 1;
    } else {
      quote_pathname(temp, start, sizeof(temp));
      showChoice->add(temp);
      if (strstr(start, "(*)") != NULL) allfiles = 1;
    }
  }

  free(copyp);

  if (!allfiles) showChoice->add(all_files_label);

  showChoice->add(custom_filter_label);

  showChoice->value(0);
  showChoiceCB();
}


/**
  Make a new directory.
  
  This function creates a new directory on the user's computer from inside the FileChooser. It also allows the user to name this directory.
  After this directory is created, the user is moved into this directory.

  \return void
*/

void FileChooser::newdir() {
  const char	*dir;		// New directory name
  char		pathname[1024];	// Full path of directory


  // Get a directory name from the user
  if ((dir = fltk::input(new_directory_label, NULL)) == NULL)
    return;

  // Make it relative to the current directory as needed...
#if (defined(WIN32) && ! defined (__CYGWIN__)) || defined(__EMX__)
  if (dir[0] != '/' && dir[0] != '\\' && dir[1] != ':')
#else
  if (dir[0] != '/' && dir[0] != '\\')
#endif /* WIN32 || __EMX__ */
    snprintf(pathname, sizeof(pathname), "%s/%s", directory_, dir);
  else
    strlcpy(pathname, dir, sizeof(pathname));

  // Create the directory; ignore EEXIST errors...
#if defined(WIN32) && ! defined (__CYGWIN__)
  if (mkdir(pathname))
#else
  if (mkdir(pathname, 0777))
#endif /* WIN32 */
    if (errno != EEXIST)
    {
      fltk::alert("%s", strerror(errno));
      return;
    }

  // Show the new directory...
  directory(pathname);
}


/**
  Enable or disable the preview tile.
  
  The preview tile allows a user to view a 'snapshot' of the file they have currently selected. This loads all forms of files, from images to source code to pdf documents, but requires a SharedImage handler to correctly decypher the file data and then display it as an image.

  \param e A boolean flag representing whether or not to turn previews on. False turns them off, true turns them on.
  \see SharedImage::add_handler()
  \return void
*/

void FileChooser::preview(bool e) {
  previewButton->value(e);
  prefs_.set("preview", e ? 1 : 0);

  Group *p = previewBox->parent();
  if (e) {
    int w = p->w() * 2 / 3;
    fileList->resize(fileList->x(), fileList->y(),
                     w, fileList->h());
    previewBox->resize(fileList->x()+w, previewBox->y(),
                       p->w()-w, previewBox->h());
    previewBox->show();
    update_preview();
  } else {
    fileList->resize(fileList->x(), fileList->y(),
                     p->w(), fileList->h());
    previewBox->resize(p->x()+p->w(), previewBox->y(),
                       0, previewBox->h());
    previewBox->hide();
  }
  p->init_sizes();

  fileList->parent()->redraw();
}


/**
  Timeout handler for the preview box.

  This function calls update_preview on the FileChooser after a time lapse of half a second from when a file is first selected

  \param fc Which FileChooser to use
  \see update_preview()
  \return void
*/

void FileChooser::previewCB(FileChooser *fc) {
  fc->update_preview();
}


/**
  Rescan the current directory.
  
  This function re-loads the current directory, scanning for new files or folders

  \return void
*/

void FileChooser::rescan() {
  activate_okButton_if_file();

  // Build the file list...
  fileList->load(directory_, sort);

  // Update the preview box...
  update_preview();
}


/**
  Handle "Show" menu selections.

  This function handles all the options in the drop-down "Show" menu. This also allows users to enter their own manual filename patterns or select from ones previously used.

  \see filter(const char* p)
  \return void
*/

void FileChooser::showChoiceCB() {
  const char	*item,			// Selected item
		*patstart;		// Start of pattern
  char		*patend;		// End of pattern
  char		temp[1024];		// Temporary string for pattern


  item = showChoice->child(showChoice->value())->label();

  if (strcmp(item, custom_filter_label) == 0) {
    if ((item = fltk::input(custom_filter_label, pattern_)) != NULL) {
      strlcpy(pattern_, item, sizeof(pattern_));

      quote_pathname(temp, item, sizeof(temp));
      showChoice->add(temp);
      showChoice->value(showChoice->size() - 2);
    }
  } else if ((patstart = strchr(item, '(')) == NULL) {
    strlcpy(pattern_, item, sizeof(pattern_));
  } else {
    strlcpy(pattern_, patstart + 1, sizeof(pattern_));
    if ((patend = strrchr(pattern_, ')')) != NULL) *patend = '\0';
  }

  fileList->filter(pattern_);

  if (shown()) {
    // Rescan the directory...
    rescan();
    fileList->deselect();
  }
}


/**
  Update the favorites menu.
  
  This function allows users to add or remove from the favorites menu

  \see favoritesCB()
  \return void
*/

void FileChooser::update_favorites() {
  if(favorites_showing) {
	favoritesButton->show();
  } else {
	favoritesButton->hide();
  }

  int		i;			// Looping var
  char		pathname[1024],		// Pathname
		menuname[2048];		// Menu name
  const char	*home;			// Home directory


  favoritesButton->clear();
  favoritesButton->add("bla");
  favoritesButton->clear();
  favoritesButton->add(add_favorites_label, fltk::ALT + 'a', 0);
  favoritesButton->add(manage_favorites_label, fltk::ALT + 'm', 0, 0, fltk::MENU_DIVIDER);
  favoritesButton->add(filesystems_label, fltk::ALT + 'f', 0);
    
  if ((home = getenv("HOME")) != NULL) {
    quote_pathname(menuname, home, sizeof(menuname));
    favoritesButton->add(menuname, fltk::ALT + 'h', 0);
  }

  for (i = 0; i < 100; i ++) {
    sprintf(menuname, "favorite%02d", i);
    prefs_.get(menuname, pathname, "", sizeof(pathname));
    if (!pathname[0]) break;

    quote_pathname(menuname, pathname, sizeof(menuname));

    if (i < 10) favoritesButton->add(menuname, fltk::ALT + '0' + i, 0);
    else favoritesButton->add(menuname);
  }

  if (i == 100) favoritesButton->child(0)->deactivate(); // disable Add to Favorites
}


/**
  Update the preview box

  This function updates the contents of the preview box with an image, or failing that attempts to load the first 1 kilobyte of a file (if it contains printable characters). Failing this, the function just prints a large "?" in the place of the file to be previewed.

  \return void
*/

void FileChooser::update_preview() {
  const char		*cfilename;	// Current filename
  char			*filename; 	// UTF-8 converted filename
  SharedImage		*image,		// New image
			*oldimage;	// Old image
  int			pbw, pbh;	// Width and height of preview box
  int			w, h;		// Width and height of preview image


  if (!previewButton->value()) return;
  cfilename = value();
  if (cfilename != NULL && !utf8test(cfilename, strlen(cfilename))) {
    int length = utf8frommb(NULL, 0, cfilename, strlen(cfilename));
    filename = (char*)malloc(sizeof(char)*length+11);
    utf8frommb(filename, length+1, cfilename, length+1);
  } else if(cfilename != NULL) {
    filename = (char*)malloc(sizeof(char)*strlen(cfilename)+11);
    strcpy(filename, cfilename);
  } else {
    filename = NULL;
  }
  if (filename == NULL || fltk::filename_isdir(filename)) image = NULL;
  else {
    window->cursor(fltk::CURSOR_WAIT);
    fltk::check();

    image = SharedImage::get(filename);

    if (image) {
      window->cursor(fltk::CURSOR_DEFAULT);
      fltk::check();
    }
  }

  oldimage = (SharedImage *)previewBox->image();

  if (oldimage) oldimage->remove();

  previewBox->image((Symbol*)0);

  if (!image) {
    FILE	*fp;
    int		bytes;
    char	*ptr;

    if (filename) fp = fopen(filename, "rb");
    else fp = NULL;

    if (fp != NULL) {
      // Try reading the first 1k of data for a label...
      bytes = fread(preview_text_, 1, sizeof(preview_text_) - 1, fp);
      preview_text_[bytes] = '\0';
      fclose(fp);
    } else {
      // Assume we can't read any data...
      preview_text_[0] = '\0';
    }

    window->cursor(fltk::CURSOR_DEFAULT);
    fltk::check();

    // Scan the buffer for printable chars...
    for (ptr = preview_text_;
         *ptr && (isprint(*ptr & 255) || isspace(*ptr & 255));
	 ptr ++);

    if (*ptr || ptr == preview_text_) {
      // Non-printable file, just show a big ?...
      previewBox->label(filename ? "?" : 0);
      previewBox->align(fltk::ALIGN_CLIP);
      previewBox->labelsize(100);
      previewBox->labelfont(fltk::HELVETICA);
    } else {
      // Show the first 1k of text...
      int size = previewBox->h() / 20;
      if (size < 6) size = 6;
      else if (size > 14) size = 14;

      previewBox->label(preview_text_);
      previewBox->align((fltk::ALIGN_CLIP | fltk::ALIGN_INSIDE |
                                   fltk::ALIGN_LEFT | fltk::ALIGN_TOP));
      previewBox->labelsize((uchar)size);
      previewBox->labelfont(fltk::COURIER);
    }
  } else {
    pbw = previewBox->w() - 20;
    pbh = previewBox->h() - 20;

    if (image->w() > pbw || image->h() > pbh) {
      w   = pbw;
      h   = w * image->h() / image->w();

      if (h > pbh) {
	h = pbh;
	w = h * image->w() / image->h();
      }
      image->setsize(w,h);
      previewBox->image((Image *)image);
    } else {
      previewBox->image((Image *)image);
    }

    previewBox->align(fltk::ALIGN_CLIP);
    previewBox->label(0);
    previewBox->set_flag(fltk::RESIZE_FIT);
  }

  previewBox->redraw();
}

/** Update the sorting method

  This function completely re-loads the displayed FileList.
  This may be slow for large directories as it rescans completely
  \todo Only resort the contents of the directory, not rescan
*/
  
void FileChooser::update_sort() {
  fileList->load(directory_, sort);
};

/** Toggle the visibility of the sort menu

  \param e determines whether or not to show the menu
*/

void FileChooser::sort_visible(int e) { 
  if (e)
    sortButton->show();
  else sortButton->hide();
  sortButton->relayout();
}

/** Tests the visility of the sort menu

  \return whether or not the sort menu is visible.
*/
int FileChooser::sort_visible() const {
  return (int)sortButton->visible();
}

/**
  Return a selected filename.

  Finds and then returns the fth file in the directory, where f is the number of the file. 
  If the value of f is higher than the amount of files in the directory, this function returns either the value in the filename field, or failing that, NULL

  \param f The 'f'th file in the directory
  \return The 'f'th filename in the directory. If f is too large, it returns the file listed in the filename field, or NULL if there is no file in the filename field and the 'f'th file doesn't exist.
*/

const char * FileChooser::value(int f) {
  int		i;		// Looping var
  int		fcount;		// Number of selected files
  const char	*name;		// Current filename
  static char	pathname[1024];	// Filename + directory


  name = fileName->text();

  if (!(type_ & FileChooser::MULTI)) {
    // Return the filename in the filename field...
    if (!name || !name[0]) return NULL;
    else return name;
  }

  // Return a filename from the list...
  for (i = 1, fcount = 0; i <= fileList->size(); i ++)
    if (fileList->selected(i)) {
      // See if this file is a selected file/directory...
      name = fileList->child(i)->label();

      fcount ++;

      if (fcount == f) {
	if (directory_[0]) {
	  snprintf(pathname, sizeof(pathname), "%s/%s", directory_, name);
	} else {
	  strlcpy(pathname, name, sizeof(pathname));
	}

	return pathname;
      }
    }

  // If nothing is selected, use the filename field...
  if (!name || !name[0]) return NULL;
  else return name;
}


/** 
  Set the current filename.

  Takes the passed filename and sets the current directory (and filename) to this name. 
  If no name is passed, this function essentially just deactivates the OK button.

  \param filename The filename and/or directory
  \see directory(const char* d, bool f)
  \return void
*/

void FileChooser::value(const char *filename) {
  int	i,				// Looping var
  	fcount;				// Number of items in list
  char	*slash;				// Directory separator
  char	pathname[1024];			// Local copy of filename


//  printf("FileChooser::value(\"%s\")\n", filename == NULL ? "(null)" : filename);

  // See if the filename is the "My System" directory...
  if (filename == NULL || !filename[0]) {
    // Yes, just change the current directory...
    directory(filename, false);
    fileName->value("");
    okButton->deactivate();
    return;
  }

#ifdef WIN32
  // See if the filename contains backslashes...
  char	fixpath[1024];			// Path with slashes converted
  if (strchr(filename, '\\')) {
    // Convert backslashes to slashes...
    strlcpy(fixpath, filename, sizeof(fixpath));

    for (slash = strchr(fixpath, '\\'); slash; slash = strchr(slash + 1, '\\'))
      *slash = '/';

    filename = fixpath;
  }
#endif // WIN32

  // See if there is a directory in there...
  fltk::filename_absolute(pathname, sizeof(pathname), filename);
  if ((slash = strrchr(pathname, '/')) != NULL) {
    // Yes, change the display to the directory... 
    if (fltk::filename_isdir(pathname))
      slash = pathname;
    else
      *slash++ = 0;

    directory(pathname, false);
    if (!shown()) fileList->load(pathname);
    if (slash > pathname) {
      --slash; *slash = '/'; slash++;
    }
  } else {
    directory(".", false);
    slash = pathname;
  }

  // Set the input field to the absolute path...
  fileName->value(pathname);
  fileName->position(0, strlen(pathname));
  okButton->activate();

  // Then find the file in the file list and select it...
  fcount = fileList->size();

  fileList->deselect(0);
  fileList->redraw();

  for (i = 0; i < fcount; i ++)
#if defined(WIN32) || defined(__EMX__)
    if (strcasecmp(fileList->child(i)->label(), slash) == 0) {
#else
    if (strcmp(fileList->child(i)->label(), slash) == 0) {
#endif // WIN32 || __EMX__
//      printf("Selecting line %d...\n", i);
      fileList->topline(i);
      fileList->select(i);
      update_preview();
      okButton->activate();
      break;
    }
}


/**
  Activates the OK button if a file is selected.

  \return void
*/
void FileChooser::activate_okButton_if_file() {
    if (((type_ & CREATE) || !access(fileName->text(), 0)) &&
        (!fltk::filename_isdir(fileName->text()) || (type_ & DIRECTORY)))
      okButton->activate();
    else
      okButton->deactivate();
}


//
// 'compare_dirnames()' - Compare two directory names.
//

static int
compare_dirnames(const char *a, const char *b) {
  int alen, blen;

  // Get length of each string...
  alen = strlen(a) - 1;
  blen = strlen(b) - 1;

  if (alen < 0 || blen < 0) return alen - blen;

  // Check for trailing slashes...
  if (a[alen] != '/') alen ++;
  if (b[blen] != '/') blen ++;

  // If the lengths aren't the same, then return the difference...
  if (alen != blen) return alen - blen;

  // Do a comparison of the first N chars (alen == blen at this point)...
#ifdef WIN32
  return strncasecmp(a, b, alen);
#else
  return strncmp(a, b, alen);
#endif // WIN32
}


//
// 'quote_pathname()' - Quote a pathname for a menu.
//

static void
quote_pathname(char       *dst,		// O - Destination string
               const char *src,		// I - Source string
	       int        dstsize)	// I - Size of destination string
{
  dstsize --;

  while (*src && dstsize > 1) {
    if (*src == '\\') {
      // Convert backslash to forward slash...
      *dst++ = '\\';
      *dst++ = '/';
      src ++;
    } else {
      if (*src == '/') *dst++ = '\\';

      *dst++ = *src++;
    }
  }

  *dst = '\0';
}


//
// 'unquote_pathname()' - Unquote a pathname from a menu.
//

static void
unquote_pathname(char       *dst,	// O - Destination string
                 const char *src,	// I - Source string
	         int        dstsize)	// I - Size of destination string
{
  dstsize --;

  while (*src && dstsize > 1) {
    if (*src == '\\') src ++;
    *dst++ = *src++;
  }

  *dst = '\0';
}


//
// End of "$Id: FileChooser2.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
