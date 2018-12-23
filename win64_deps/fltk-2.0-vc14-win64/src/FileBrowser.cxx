//
// "$Id: FileBrowser.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// FileBrowser routines.
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
//   FileBrowser::full_height()     - Return the height of the list.
//   FileBrowser::item_height()     - Return the height of a list item.
//   FileBrowser::item_width()      - Return the width of a list item.
//   FileBrowser::item_draw()       - Draw a list item.
//   FileBrowser::FileBrowser() - Create a FileBrowser widget.
//   FileBrowser::load()            - Load a directory into the browser.
//   FileBrowser::filter()          - Set the filename filter.
//

//
// Include necessary header files...
//

#include <fltk/FileBrowser.h>
#include <fltk/Browser.h>
#include <fltk/Item.h>
#include <fltk/draw.h>
#include <fltk/Color.h>
#include <fltk/Flags.h>
#include <fltk/Font.h>
#include <fltk/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __CYGWIN__
#  include <mntent.h>
#elif defined(WIN32)
#  include <windows.h>
#  include <direct.h>
// Apparently Borland C++ defines DIRECTORY in <direct.h>, which
// interfers with the FileIcon enumeration of the same name.
#  ifdef DIRECTORY
#    undef DIRECTORY
#  endif // DIRECTORY
#endif // __CYGWIN__

#ifdef __EMX__
#  define  INCL_DOS
#  define  INCL_DOSMISC
#  include <os2.h>
#endif // __EMX__

// CodeWarrior (__MWERKS__) gets its include paths confused, so we
// temporarily disable this...
#if defined(__APPLE__) && !defined(__MWERKS__)
#  include <sys/param.h>
#  include <sys/ucred.h>
#  include <sys/mount.h>
#endif // __APPLE__ && !__MWERKS__

using namespace fltk;

/** \class fltk::FileBrowser
  The fltk::FileBrowser widget displays a list of filenames, optionally
  with file-specific icons.
*/

/** Create a FileBrowser widget
  \param X Upper left-hand X coordinate
  \param Y Upper right-hand Y coordinate
  \param W Width in pixels
  \param H Height in pixels
  \param l Label text  
 */
FileBrowser::FileBrowser(int X, int Y, int W, int H, const char *l) : Browser(X, Y, W, H, l) {
  // Initialize the filter pattern, current directory, and icon size...
  pattern_   = "*";
  directory_ = "";
  icon_size_  = -1.0f;
  filetype_  = FILES;
  show_hidden_ = false;
}

/** Load a directory into the browser.
  \param directory Directory to load
  \param sort Sorting function to use
  \return Number of files loaded
*/
int FileBrowser::load(const char *directory, FileSortF *sort) {
  int		i;				// Looping var
  int		num_files;			// Number of files in directory
  int		num_dirs;			// Number of directories in list
  char		filename[4096];			// Current file
  FileIcon	*icon;				// Icon to use


//  printf("FileBrowser::load(\"%s\")\n", directory);

  if (!directory)
    return (0);

  clear();
  yposition(0);
  directory_ = directory;

  if (directory_[0] == '\0')
  {
    //
    // No directory specified; for UNIX list all mount points.  For DOS
    // list all valid drive letters...
    //

    num_files = 0;
    if ((icon = FileIcon::find("any", FileIcon::DEVICE)) == NULL)
      icon = FileIcon::find("any", FileIcon::DIRECTORY);

#ifdef WIN32
#  ifdef __CYGWIN__
    //
    // Cygwin provides an implementation of setmntent() to get the list
    // of available drives...
    //
    FILE          *m = setmntent("/-not-used-", "r");
    struct mntent *p;

    while ((p = getmntent (m)) != NULL) {
      add(p->mnt_dir, icon);
      num_files ++;
    }

    endmntent(m);
#  else
    //
    // Normal WIN32 code uses drive bits...
    //
    DWORD	drives;		// Drive available bits

    drives = GetLogicalDrives();
    for (i = 'A'; i <= 'Z'; i ++, drives >>= 1)
      if (drives & 1)
      {
        sprintf(filename, "%c:/", i);

	add(filename, icon);

	num_files ++;
      }
#  endif // __CYGWIN__
#elif defined(__EMX__)
    //
    // OS/2 code uses drive bits...
    //
    ULONG	curdrive;	// Current drive
    ULONG	drives;		// Drive available bits
    int		start = 3;      // 'C' (MRS - dunno if this is correct!)


    DosQueryCurrentDisk(&curdrive, &drives);
    drives >>= start - 1;
    for (i = 'A'; i <= 'Z'; i ++, drives >>= 1)
      if (drives & 1)
      {
        sprintf(filename, "%c:/", i);
        add(filename, icon);

	num_files ++;
      }
#elif defined(__APPLE__) && !defined(__MWERKS__)
    // MacOS X and Darwin use getfsstat() system call...
    int			numfs;	// Number of file systems
    struct statfs	*fs;	// Buffer for file system info


    // We always have the root filesystem.
    add("/", icon);

    // Get the mounted filesystems...
    numfs = getfsstat(NULL, 0, MNT_NOWAIT);
    if (numfs > 0) {
      // We have file systems, get them...
      fs = new struct statfs[numfs];
      getfsstat(fs, sizeof(struct statfs) * numfs, MNT_NOWAIT);

      // Add filesystems to the list...
      for (i = 0; i < numfs; i ++) {
	// Ignore "/", "/dev", and "/.vol"...
        if (fs[i].f_mntonname[1] && strcmp(fs[i].f_mntonname, "/dev") &&
	    strcmp(fs[i].f_mntonname, "/.vol")) {
          snprintf(filename, sizeof(filename), "%s/", fs[i].f_mntonname);
          add(filename, icon);
        }
        num_files ++;
      }

      // Free the memory used for the file system info array...
      delete[] fs;
    }
#else
    //
    // UNIX code uses /etc/fstab or similar...
    //
    FILE	*mtab;		// /etc/mtab or /etc/mnttab file
    char	line[1024];	// Input line

    //
    // Open the file that contains a list of mounted filesystems...
    //

    mtab = fopen("/etc/mnttab", "r");	// Fairly standard
    if (mtab == NULL)
      mtab = fopen("/etc/mtab", "r");	// More standard
    if (mtab == NULL)
      mtab = fopen("/etc/fstab", "r");	// Otherwise fallback to full list
    if (mtab == NULL)
      mtab = fopen("/etc/vfstab", "r");	// Alternate full list file

    if (mtab != NULL)
    {
      while (fgets(line, sizeof(line), mtab) != NULL)
      {
        if (line[0] == '#' || line[0] == '\n')
	  continue;
        if (sscanf(line, "%*s%4095s", filename) != 1)
	  continue;

        strlcat(filename, "/", sizeof(filename));

//        printf("FileBrowser::load() - adding \"%s\" to list...\n", filename);
        add(filename, icon);
	num_files ++;
      }

      fclose(mtab);
    }
#endif // WIN32 || __EMX__
  }
  else
  {
    dirent	**files;	// Files in in directory


    //
    // Build the file list...
    //

#if (defined(WIN32) && !defined(__CYGWIN__)) || defined(__EMX__)
    strlcpy(filename, directory_, sizeof(filename));
    i = strlen(filename) - 1;

    if (i == 2 && filename[1] == ':' &&
        (filename[2] == '/' || filename[2] == '\\'))
      filename[2] = '/';
    else if (filename[i] != '/' && filename[i] != '\\')
      strlcat(filename, "/", sizeof(filename));

    num_files = fltk::filename_list(filename, &files, sort);
#else
    num_files = fltk::filename_list(directory_, &files, sort);
#endif /* WIN32 || __EMX__ */

    if (num_files <= 0)
      return (0);

    for (i = 0, num_dirs = 0; i < num_files; i ++) {
      if (strcmp(files[i]->d_name, "./") ) {
	snprintf(filename, sizeof(filename), "%s/%s", directory_,
	         files[i]->d_name);

        //bool ft = true;	if (ft) {FileIcon::load_system_icons(); ft=false;}

        icon = FileIcon::find(filename);
	//printf("%s\n",files[i]->d_name);
	if (!strcmp(files[i]->d_name, ".") || !strcmp(files[i]->d_name, "./") ||
	    (!show_hidden_ &&  files[i]->d_name[0]=='.' &&  strncmp(files[i]->d_name,"../",2)))
	  continue;
	if ((icon && icon->type() == FileIcon::DIRECTORY) ||
	     fltk::filename_isdir(filename)) {
          num_dirs ++;
          this->insert(num_dirs-1, files[i]->d_name, icon);
	} else if (filetype_ == FILES &&
	           fltk::filename_match(files[i]->d_name, pattern_)) {
          add(files[i]->d_name, icon);
	}
      }

      free(files[i]);
    }

    free(files);
  }

  return (num_files);
}


/** Set the filename filter.
  \param pattern Pattern to filter with. Eventually should deal with proper regex!
*/

void FileBrowser::filter(const char *pattern)	{
  // If pattern is NULL set the pattern to "*"...
  if (pattern) pattern_ = pattern;
  else pattern_ = "*";
}

////////////////////////////////////////////////////////////////
//#include <pixmaps/file_small.xpm>
//#include <fltk/xpmImage.h>
class FileItem : public Item {
public:
    FileItem(const char * label, FileIcon * icon);
    void draw();
private:
    FileIcon* fileIcon_;
};

FileItem::FileItem(const char * label, FileIcon * icon) : Item(label) {
    fileIcon_=icon;
    textsize(14);
    if(icon) icon->value(this,true);
}
void FileItem::draw()  {
  if (fileIcon_) fileIcon_->value(this,true);
    Item::draw();
}
////////////////////////////////////////////////////////////////

/** Add a line to the filebrowser
  \param line Name of the line to add
  \param icon Optional icon to add to this item
*/

void FileBrowser::add(const char *line, FileIcon *icon) {
    this->begin();
    FileItem * i = new FileItem(strdup(line),icon);
    i->w((int) icon_size());  i->h(i->w());
    this->end();
}

/** Insert an item into the file browser at the 'n'th position
    
    This shifts everything up to and including this position \a back and then places the current item in this spot.
  \param n The position to insert at. This will append if necessary
  \param label The name of the line to add
  \param icon Optional icon to add to this item
*/
void FileBrowser::insert(int n, const char *label, FileIcon*icon) {
    current(0);
    FileItem * i = new FileItem(strdup(label),icon);
    i->w((int) icon_size());  i->h(i->w());
    Menu::insert(*i,n);
}

//
// End of "$Id: FileBrowser.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
