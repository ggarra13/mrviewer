//
// "$Id: FileIcon.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Fl_FileIcon definitions for the Fast Light Tool Kit (FLTK).
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

#ifndef fltk_FileIcon_h
#define fltk_FileIcon_h

#include "FL_API.h"
#include "Color.h"
#include "Symbol.h"

namespace fltk {

class Widget;

class FL_API FileIcon : public Symbol {
  static FileIcon *first_;	// Pointer to first icon/filetype
  FileIcon	*next_;		// Pointer to next icon/filetype
  const char	*pattern_;	// Pattern string
  int		type_;		// Match only if directory or file?
  int		num_data_;	// Number of data elements
  int		alloc_data_;	// Number of allocated elements
  short		*data_;		// Icon data
  int		w_,h_;
  bool          on_select_;     // true if in browser or menu
public:

  enum				// File types
  {
    ANY,			// Any kind of file
    PLAIN,			// Only plain files
    FIFO,			// Only named pipes
    DEVICE,			// Only character and block devices
    LINK,			// Only symbolic links
    DIRECTORY 			// Only directories
  };

  enum				// Data opcodes
  {
    END,			// End of primitive/icon
    COLOR,			// Followed by color index
    LINE,			// Start of line
    CLOSEDLINE,			// Start of closed line
    POLYGON,			// Start of polygon
    OUTLINEPOLYGON,		// Followed by outline color
    VERTEX			// Followed by scaled X,Y
  };

  FileIcon(const char *p, int t, int nd = 0, short *d = 0);
  FileIcon(const FileIcon& f) ; 
  ~FileIcon();

  short		*add(short d);
  short		*add_color(Color c)
		{ short *d = add((short)COLOR); add((short)(c >> 16)); add((short)c); return (d); }
  short		*add_vertex(int x, int y)
		{ short *d = add(VERTEX); add(x); add(y); return (d); }
  short		*add_vertex(float x, float y)
		{ short *d = add(VERTEX); add((int)(x * 10000.0));
		  add((int)(y * 10000.0)); return (d); }
  void		clear() { num_data_ = 0; }

  const Symbol* image() const {return image_;}

  void		load(const char *f);
  const char	*pattern() { return (pattern_); }
  int		size() { return (num_data_); }
  int		type() { return (type_); }
  short		*data() { return (data_); }

  static FileIcon *find(const char *filename, int filetype = ANY);
  static FileIcon *first() { return (first_); }
  static void	load_system_icons(void);

  void		value(Widget* i, bool on_select=false);

  // virtual image overloads
  void _measure(int& w, int& h) const;
  void _draw(const Rectangle& r) const;
private:
  void		image(const Symbol* direct_raster, bool owned=true) {image_=direct_raster;owned_image_=owned;}
  void		load_fti(const char *fti);
  const Symbol* image_;
  bool owned_image_;
};

}

#endif

//
// End of "$Id: FileIcon.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
