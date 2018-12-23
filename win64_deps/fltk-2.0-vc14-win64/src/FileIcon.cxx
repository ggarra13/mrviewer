//
// "$Id: FileIcon.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// FileIcon routines for the Fast Light Tool Kit (FLTK).
//
// Copyright 1997-1999 by Easy Software Products.
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
// Contents:
//
//   FileIcon::FileIcon()       - Create a new file icon.
//   FileIcon::~FileIcon()      - Remove a file icon.
//   FileIcon::add()	       - Add data to an icon.
//   FileIcon::find()	      - Find an icon based upon a given file.
//   FileIcon::draw()	      - Draw an icon.
//

//
// Include necessary header files...
//

#include <stdio.h>
#include <stdlib.h>
#include <fltk/string.h>
#include <fltk/Item.h>
#include <fltk/Browser.h>
#include <fltk/utf.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <io.h>
# define F_OK 0
#else
# include <unistd.h>
#endif
#include <fltk/FileIcon.h>
#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <fltk/Box.h>
#include <fltk/filename.h>

//
// Define missing POSIX/XPG4 macros as needed...
//

#ifndef S_ISDIR
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif /* !S_ISDIR */

using namespace fltk;

//
// Icon cache...
//

FileIcon	*FileIcon::first_ = (FileIcon *)0;


//
// 'FileIcon::FileIcon()' - Create a new file icon.
//

FileIcon::FileIcon(const char *p,	/* I - Filename pattern */
			 int	t,	/* I - File type */
			 int	nd,	/* I - Number of data values */
			 short      *d)	/* I - Data values */
  : Symbol(0) {
  // Initialize the pattern and type...
  pattern_ = p;
  type_    = t;

  // Copy icon data as needed...
  if (nd)  {
    num_data_   = nd;
    alloc_data_ = nd + 1;
    data_       = (short *)calloc(sizeof(short), nd + 1);
    memcpy(data_, d, nd * sizeof(short));
  }  else  {
    num_data_   = 0;
    alloc_data_ = 0;
  }

  // And add the icon to the list of icons...
  next_  = first_;
  first_ = this;
  w_= h_=16;
  on_select_ = false;
  image_=0;
}

//! deep copy implementation
FileIcon::FileIcon(const FileIcon& f) {
  char n[1024]="";
  if (&f==0) {
      memset(this, 0, sizeof(FileIcon));
      return;
  }
      memcpy(this, &f, sizeof(FileIcon));
  // change the name so that know it's a copy
  sprintf(n, "%s%s", f.name() ? f.name() : "", "(copy)");
  name(strdup(n));

  if (f.alloc_data_ && f.data_) {
    this->data_=(short*) calloc(sizeof(short),f.alloc_data_);
    memcpy(data_, f.data_, f.alloc_data_*sizeof(short));
  }
}
//
// 'FileIcon::~FileIcon()' - Remove a file icon.
//

FileIcon::~FileIcon()
{
  FileIcon	*current,	// Current icon in list
		*prev;		// Previous icon in list


  // Find the icon in the list...
  for (current = first_, prev = (FileIcon *)0;
       current != this && current != (FileIcon *)0;
       prev = current, current = current->next_);

  // Remove the icon from the list as needed...
  if (current)
  {
    if (prev)
      prev->next_ = current->next_;
    else
      first_ = current->next_;
  }

  // Free any memory used...
  if (alloc_data_)
    free(data_);
}

// WAS: measure() is passed the size it is expected to draw into, including
// the inset. Thus it does not need to look at the widget or it's box.
// This one matches the previous implementation, making the icon always
// be square and fitting the height of the inset:

void FileIcon::_measure(int& w, int& h) const {
  w = h;
}

// Provided for back-compatability, sets the widgets image() to this and
// also sets the on_select() flag.
void FileIcon::value(Widget* i, bool on_sel)  {
  on_select_ = on_sel;
  i->image(this);
}

//
// 'FileIcon::add()' - Add data to an icon.
//

short *			// O - Pointer to new data value
FileIcon::add(short d)	// I - Data to add
{
  short	*dptr;		// Pointer to new data value


  // Allocate/reallocate memory as needed
  if ((num_data_ + 1) >= alloc_data_)
  {
    alloc_data_ += 128;

    if (alloc_data_ == 128)
      dptr = (short *)malloc(sizeof(short) * alloc_data_);
    else
      dptr = (short *)realloc(data_, sizeof(short) * alloc_data_);

    if (dptr == NULL)
      return (NULL);

    data_ = dptr;
  }

  // Store the new data value and return
  data_[num_data_++] = d;
  data_[num_data_]   = END;

  return (data_ + num_data_ - 1);
}


//
// 'FileIcon::find()' - Find an icon based upon a given file.
//

FileIcon *				// O - Matching file icon or NULL
FileIcon::find(const char *filename,	// I - Name of file */
		  int	filetype)	// I - Enumerated file type
{
  FileIcon	*current;		// Current file in list
  struct stat	fileinfo;		// Information on file

  // Get file information if needed...
  if (filetype == ANY)
    if (!fltk_stat(filename, &fileinfo))
    {
      if (S_ISDIR(fileinfo.st_mode))
	filetype = DIRECTORY;
#ifdef S_IFIFO
      else if (S_ISFIFO(fileinfo.st_mode))
	filetype = FIFO;
#endif // S_IFIFO
#if defined(S_ICHR) && defined(S_IBLK)
      else if (S_ISCHR(fileinfo.st_mode) || S_ISBLK(fileinfo.st_mode))
	filetype = DEVICE;
#endif // S_ICHR && S_IBLK
#ifdef S_ILNK
      else if (S_ISLNK(fileinfo.st_mode))
	filetype = LINK;
#endif // S_ILNK
      else
	filetype = PLAIN;
    }

  // Loop through the available file types and return any match that
  // is found...
  for (current = first_; current != (FileIcon *)0; current = current->next_)
    if ((current->type_ == filetype || current->type_ == ANY) &&
	filename_match(filename, current->pattern_))
      break;

  // Return the match (if any)...
  return (current);
}


//
// 'FileIcon::draw()' - Draw an icon.
//
//Color ic,		// I - Icon color...
//int      active)	// I - Active or inactive?

void FileIcon::_draw(const Rectangle& r) const {
  
  if (image()) {image()->draw(r);return;}

  short		*d;		// Pointer to data
  short		*prim;		// Pointer to start of primitive...
  float		scale;		// Scale of icon


  // Don't try to draw a NULL array!
  if (num_data_ == 0)
    return;

  // fabien: must save currentcolor because otherwise the browser would write the text
  // with icon color, thanks bill for having removed the back pointer !
  Color saved_color = getcolor(); 

  // Setup the transform matrix as needed...
  scale = float(r.w() < r.h() ? r.w() : r.h());

  push_matrix();
  translate(0.5f * (r.w() - scale),
	       0.5f * (r.h() + scale));
  fltk::scale(scale, -scale);

  // Loop through the array until we see an unmatched END...
  d    = data_;
  prim = NULL;
  
  Color	c, ic;
  bool active = !drawflags(INACTIVE_R);
  if (active && (!on_select_ || drawflags(SELECTED))  ) 
    ic = fltk::YELLOW;
  else
    ic = fltk::GRAY90 /* light2 */; 
  c = ic;
  setcolor(ic);
  while (*d != END || prim)
    switch (*d)
    {
      case END :
	  switch (*prim)
	  {
	    case LINE :
		strokepath();
		break;

	    case CLOSEDLINE :
		closepath();
		strokepath();
		break;

	    case POLYGON :
		fillpath();
		break;

	    case OUTLINEPOLYGON : 
		c= prim[1]==-1 ? ic : 
		    (Color)((((unsigned short *)prim)[1] << 16) | 
	                	  ((unsigned short *)prim)[2]);
		if (!active) c = inactive(c);
		fillstrokepath(c);
		break;
	  }

	  prim = NULL;
	  d ++;
	  break;

      case COLOR :
	  if (d[1] == -1)
	    c = ic;
	  else
          c = (Color)((((unsigned short *)d)[1] << 16) | ((unsigned short *)d)[2]);
	  if (!active)
	    c = inactive(c);

	  setcolor(c);
	  d += 3;
	  break;

      case LINE :
      case CLOSEDLINE :
      case POLYGON :
	  prim = d;
	  d ++;
	  break;

      case OUTLINEPOLYGON :
	  prim = d;
	  d += 3;
	  break;

      case VERTEX :
	  if (prim)
	    addvertex(d[1] * 0.0001f, d[2] * 0.0001f);
	  d += 3;
	  break;
    }

  // If we still have an open primitive, close it...
  if (prim)
    switch (*prim)
    {
	case LINE :
	    strokepath();
	    break;

	case CLOSEDLINE :
	    closepath();
	    strokepath();
	    break;

	case POLYGON :
	    fillpath();
	    break;

	case OUTLINEPOLYGON : {
	    Color color = prim[1]==-1 ? ic : 
		(Color)((((unsigned short *)prim)[1] << 16) | 
	                      ((unsigned short *)prim)[2]);
	    if (!active) color = inactive(color);
	    fillstrokepath(color);
	    break;}
    }
  // Restore the transform matrix
  pop_matrix();

  setcolor(saved_color); // restoring currrent_color necessary for browser text drawing
}


//
// End of "$Id: FileIcon.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
