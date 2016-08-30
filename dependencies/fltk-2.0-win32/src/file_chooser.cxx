//
// "$Id: file_chooser.cxx 8751 2011-05-28 13:16:09Z bgbnbigben $"
//
// File chooser widget for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2006 by Bill Spitzak and others.
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
//    http://www.fltk.org/str.php
//

#include <config.h>
#include <fltk/FileChooser.h>
#include <fltk/file_chooser.h>
#include <fltk/run.h>
#include <fltk/string.h>
#include <fltk/utf.h>

using namespace fltk;

static FileChooser	*fc = (FileChooser *)0;
static void default_callback(const char*) {}
static void		(*current_callback)(const char*) = default_callback;
static const char	*current_label = "Ok";

#ifdef _WIN32
// Prototype for local functions regarding the WIN32 filechooser
static wchar_t* patternToWin(const char* const pat, int len);
static unsigned windowsWLen(const wchar_t* const string);
#endif

// Do a file chooser callback...
static void callback(FileChooser *, void*) {
  if (current_callback && fc->value())
    (*current_callback)(fc->value());
}


/*! This function is called every time the user navigates to a new file
  or directory in the file chooser. It can be used to preview the result
  in the main window. */
void fltk::file_chooser_callback(void (*cb)(const char*)) {
  current_callback = cb ? cb : default_callback;
}


// Set the "OK" button label
void fltk::file_chooser_ok_label(const char *l) {
  if (l) current_label = l;
  else current_label = "Ok";
}


//
// 'fltk::dir_chooser()' - Show a file chooser dialog and get a directory.
//

const char *					// O - Directory or NULL
fltk::dir_chooser(const char *message,	// I - Message for titlebar
               const char *fname,	// I - Initial directory name
	       int        relative)	// I - 0 for absolute
{
  static char	retname[1024];		// Returned directory name

  if (!fc) {
    if (!fname || !*fname) fname = ".";

    fc = new FileChooser(fname, "*", FileChooser::CREATE |
                                         FileChooser::DIRECTORY, message);
    fc->callback(callback, 0);
  } else {
    fc->type(FileChooser::CREATE | FileChooser::DIRECTORY);
    fc->filter("*");
    if (fname && *fname) fc->value(fname);
    fc->label(message);
  }

  //fc->show();
  // while (fc->visible()) fltk::wait();
  fc->exec(0, true);

  if (fc->value() && relative) {
      fltk::filename_relative(retname, sizeof(retname), fc->value());

    return retname;
  } else if (fc->value()) return (char *)fc->value();
  else return 0;
}

static bool use_system_fc = false;
/*! On Windows this makes file_chooser() call the Win32 file chooser
  API instead of using the one constructed in fltk. Ignored on other
  systems.
*/
void fltk::use_system_file_chooser(bool useit) {use_system_fc = useit;}

#ifdef _WIN32
# include <windows.h>
# include <commdlg.h>
# ifndef __CYGWIN__
#  include <direct.h>
#  ifndef OPENFILENAME_SIZE_VERSION_400
#   define OPENFILENAME_SIZE_VERSION_400 sizeof(OPENFILENAME)
#  endif
# endif
# if defined(__BORLANDC__)
#  include <dos.h> // _getdrive()
# endif
#endif

/*!
  \image html filechooser.gif

  pops up the file chooser, waits for the user to pick a file or
  Cancel, and then returns a pointer to that filename or NULL if
  Cancel is chosen. If use_system_file_chooser() is set to true, a
  system FileChooser is opened. If the user picks multiple files,
  these will be separated by a new line.

  \param message is a string used to title the window.

  \param pattern is used to limit the files listed in a directory to those
  matching the pattern. This matching is done by filename_match(). Use
  NULL to show all files. If use_system_file_chooser() is set to true, this
  will automatically include the "All Files" option.

  \param fname is a default folder/filename to fill in the chooser
  with. If this ends with a '/' then this is a default folder and
  no file is preselected.

  If \a fname is NULL then the last filename that was choosen is used,
  unless the \a pattern changes, in which case only the last directory
  is used. The first time the file chooser is called this defaults to
  a blank string.

  \param save determines whether or not the chooser is created in "save"
  or "open" mode. This only effects WIN32 and will probably be replaced by the
  option to OR FileChooser flags together.

  \param relative is used to determine whether this function returns a relative
  or an absolute pathname.

  \return The returned value points at a static buffer that is only good until
  the next time the file_chooser() is called.

  \todo Add a "flags" variable that allows the user to override flags.
*/

const char * fltk::file_chooser(const char *message, const char *pattern,
				const char *fname, bool save,
				int        relative) {
  static char	retname[1024];		// Returned filename

#if defined( _WIN32 ) && !defined( __CYGWIN__ )
  if (use_system_fc) {
    static wchar_t filenamebuffer[MAX_PATH];
    static wchar_t currentDirBuffer[MAX_PATH];
    int length;
    wchar_t *messageW, *patternW = NULL, *fnameW;
    OPENFILENAMEW wreq;
    static char ret[PATH_MAX];

    // Convert all input to wide-char format.
    length = utf8towc(message, strlen(message), NULL, 0);
    messageW = new wchar_t [length+1];
    *messageW = 0;
    utf8towc(message, strlen(message), messageW, length + 1);
    length = utf8towc(fname, strlen(fname), NULL, 0);
    fnameW = new wchar_t [length+1];
    *fnameW = 0;
    utf8towc(fname, strlen(fname), fnameW, length + 1);

    // convert the fltk filename pattern to a windows pattern
    patternW = patternToWin(pattern, strlen(pattern)+1);

    // set up the wreq struct. This sets the appropriate pattern(s),
    // title of the window, and the flags (including MULTI)
    memset(&wreq, 0, sizeof(wreq));
    wreq.lStructSize = OPENFILENAME_SIZE_VERSION_400; // needed for Win < Win2k
    wreq.lpstrFilter = patternW;
    wreq.nFilterIndex = (patternW) ? 1 : 0;
    wreq.nMaxFile = MAX_PATH;
    wreq.lpstrTitle = messageW ? messageW : L"Select the filename";
    
    if (fnameW) {
      memset(filenamebuffer, 0, MAX_PATH);
      // BS: Commented this out, as the memset() seems to render it redundant
      /*if((filenamebuffer[1] == ':') && (_getdrive() + 'A' - 1 == filenamebuffer[0]))
	wcsncpy(filenamebuffer, fnameW + 2, MAX_PATH);
      else*/
      wcsncat(filenamebuffer, fnameW, MAX_PATH);
    }
    wreq.lpstrFile = filenamebuffer;
    GetCurrentDirectoryW(MAX_PATH, currentDirBuffer);
    wreq.lpstrInitialDir = currentDirBuffer;
    wreq.Flags = OFN_NOCHANGEDIR | OFN_NOVALIDATE | OFN_HIDEREADONLY |
		 OFN_EXPLORER | OFN_ENABLESIZING | OFN_ALLOWMULTISELECT;
    if (save ? GetSaveFileNameW(&wreq) : GetOpenFileNameW(&wreq)) {
      char temp[PATH_MAX], dir[PATH_MAX];
      memset(temp, 0, PATH_MAX); memset(dir, 0, PATH_MAX);
      ret[0] = 0;
      utf8fromwc(temp, PATH_MAX, wreq.lpstrFile, windowsWLen(wreq.lpstrFile)+1);
      sprintf(dir, "%s\\", temp);
      for (char *s = temp + strlen(temp) + 1; s && *s; s += strlen(s) + 1) {
	sprintf(ret + strlen(ret), "%s%s\n", dir, s);
      }
      if (!ret[0]) sprintf(ret, "%s", temp);
      delete [] messageW; delete [] fnameW; delete [] patternW;
      return ret;
    }
    delete [] messageW; delete [] fnameW; delete [] patternW;
    return 0;
  }
#endif
  if (!fc) {
    if (!fname || !*fname) fname = ".";
    
    fc = new FileChooser(fname, pattern, FileChooser::CREATE, message);
    fc->callback(callback, 0);
  }
  fc->type(FileChooser::CREATE);
  fc->filter(pattern);
  fc->label(message);
  
  if (!fname || !*fname) {
    if (fc->filter() != pattern && (!pattern || !fc->filter() ||
				strcmp(pattern, fc->filter())) && fc->value()) {
      // if pattern is different, remove name but leave old directory:
      strlcpy(retname, fc->value(), sizeof(retname));
      
	char *p = strrchr(retname, '/');
	
        if (p) {
	  // If the filename is "/foo", then the directory will be "/", not
	  // ""...
	  if (p == retname)
	    retname[1] = '\0';
	  else
	    *p = '\0';
	}
	
	// Set the directory...
	fc->directory(retname);
    }
  }
  else {
    fc->ok_label(current_label);
  }  

  fc->value(fname);
  fc->exec(0, true);
  
  if (fc->value() && relative) {
    fltk::filename_relative(retname, sizeof(retname), fc->value());
    
    return retname;
  } else if (fc->value()) {
    fltk::filename_absolute(retname, sizeof(retname), fc->value());
    return retname;
  }
  else return 0;
}

#ifdef _WIN32
// This function converts a FLTK pattern to Windows' format.
// It is only static and not available via the API.
wchar_t* patternToWin(const char* const pat, int len) {
  wchar_t *out;
  char temp[4096];
  const char *ptr = pat;
  char *tmp = temp;
  bool wildCard = false;
  int length;
  for ( ; ptr && *ptr && ptr - pat < len; ptr += length) {
    unsigned code;
    if (*ptr & 0x80) {
      code = utf8decode(ptr, pat+len, &length);
    } else {
      code = *ptr; length = 1;
    }
    switch (code) {
      case '(':
        if ((ptr-1) > pat) *(tmp-1) = '\0';
        break;
      case ')':
        if ((tmp - temp + 1) < 4096 ) {
          *tmp = '\0';
          tmp++; // legal; '\0' is one-byte
        }
        break;
      case '{':
        wildCard = true;
        break;
      case '}':
        wildCard = false;
        break;
      case ',':
        if (wildCard && (tmp - temp + 2 < 4096)) {
          *tmp = ';'; 
          *(tmp+1) = '*';
          *(tmp+2) = '.';
	  tmp += 3; // legal; ";*." makes up 3 bytes
        }
        break;
      case '\t':
      case '\n':
        break;
      default:
        utf8encode(code, tmp);
        tmp += length;
        break;
	}
  }
  // Make sure there's space for "All Files\0*.*\0\0
  if (tmp + 15 < temp + 4096) {
    tmp[0] = 'A'; tmp[1] = 'l'; tmp[2] = 'l'; tmp[3] = ' ';
    tmp[4] = 'F'; tmp[5] = 'i'; tmp[6] = 'l'; tmp[7] = 'e';
    tmp[8] = 's'; tmp[9] = '\0'; tmp[10] = '*'; tmp[11] = '.';
    tmp[12] = '*'; tmp[13] = '\0'; tmp[14] = '\0';
    tmp += 15;
  }

  length = utf8towc(temp, tmp - temp, NULL, 0);
  out = new wchar_t [length];
  memset(out, 0, sizeof(wchar_t)*(length));
  utf8towc(temp, tmp - temp, out, length);
  return out;
}
  
// This function determines the length of a Windows
// double-null-terminated string. It is static and 
// not available via the public API.
unsigned windowsWLen(const wchar_t* const string) {
  unsigned out = 0;
  for ( ; ; out++)
    if (!string[out] && !string[out+1])
      return out;
}
#endif

//
// End of "$Id: file_chooser.cxx 8751 2011-05-28 13:16:09Z bgbnbigben $".
//
