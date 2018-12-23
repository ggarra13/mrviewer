//
// "$Id: FileBrowser.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// FileBrowser definitions.
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

//
// Include necessary header files...
//

#ifndef fltk_FileBrowser_h
#define fltk_FileBrowser_h

#include <fltk/Browser.h>
#include <fltk/FileIcon.h>
#include <fltk/filename.h>

namespace fltk {


//
// FileBrowser class...
//

class FL_API FileBrowser : public Browser
{
  int		filetype_; /**< Paramted based on FileBrowser::FILES or FileBrowser::DIRECTORIES */
  const char	*directory_; /**< The current directory */
  float		icon_size_; /**< The FileBrowser's icon sizes */
  const char	*pattern_; /**< The filename glob pattern \todo regex! */

public:
  /** The types of items this browser can show */
  enum { 
    FILES, /**< Show all files */
    DIRECTORIES /**< Show only directories */
  };

  FileBrowser(int, int, int, int, const char * = 0);

  /** \returns The icon size as a float */
  float		icon_size() const { 
    return (icon_size_ <0?  (2.0f* textsize()) : icon_size_); 
  }
  /** Sets the icon size and redraws the browser
    \param f The new icon size
  */
  void 		icon_size(float f) { icon_size_ = f; redraw(); };

  void	filter(const char *pattern);
  /** \returns The filter pattern */
  const char	*filter() const { return (pattern_); };

  int		load(const char *directory, FileSortF *sort = (FileSortF*) fltk::numericsort);
  
  /** \returns the current Browser's textsize */
  float		textsize() const { return (Browser::textsize()); };
  /** Sets the current Browser's textsize and scales the icon size accordingly
    \param s The new text size
  */
  void		textsize(float s) { Browser::textsize(s); icon_size_ = (uchar)(3 * s / 2); };

  /** \returns The browser's current filetype */
  int		filetype() const { return (filetype_); };
  /** Sets the filetype
    \param t The new filetype. Can be either FileBrowser::DIRECTORIES or FileBrowser::FILES
  */
  void		filetype(int t) { filetype_ = t; };
  /** \return The current directory */
  const char *  directory() const {return directory_;}

  // adding or inserting a line into the fileBrowser
  void insert(int n, const char* label, FileIcon* icon);
  /** Inserts a line at the 'n'th position with Callback data set. This will not attempt to split the label at '/' characters
    \param n The position to insert \a at. This will append if necessary
    \param label The label of the widget
    \param data The callback data of the widget
  */
  void insert(int n, const char* label, void* data){Menu::insert(n, label,data);}
  void add(const char * line, FileIcon* icon);

  // Showing or not showing the hidden files, that's the question:
public:
  /** Set this flag if you want to see the hidden files in the browser
    \param show A flag to determine whether or not to show hidden files
  */
  void	    show_hidden(bool show) { show_hidden_= show; }
  /** \return Whether or not we're currently showing hidden files */
  bool	    show_hidden() const {return show_hidden_;}
private:
    bool		show_hidden_; /**< Used to determine whether to show hidden files or not */
};

}

#endif // !_Fl_File_Browser_H_

//
// End of "$Id: FileBrowser.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
