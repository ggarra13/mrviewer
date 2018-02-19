//
// "$Id: HelpView.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// Help Viewer widget definitions.
//
// Copyright 1997-2006 by Easy Software Products.
// Image support donated by Matthias Melcher, Copyright 2000.
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

#ifndef HelpView_H
#  define HelpView_H

//
// Include necessary header files...
//

#  include <stdio.h>
#  include <fltk/Group.h>
#  include <fltk/Scrollbar.h>
#  include <fltk/draw.h>
#  include <fltk/SharedImage.h>

namespace fltk {

/** HelpFunc type - link callback function for files... */
typedef const char *(HelpFunc) (Widget *, const char *);


/** HelpBlock structure, used internally in the HelpView.
    Draws and remembers a block of text's start and end */
struct HelpBlock {
  const char *start,            /**< Start of text */
   *end;                        /**< End of text */
  uchar border;                 /**< Draw border? */
  Color bgcolor;             	/**< Background color */
  int x,                        /**< Indentation/starting X coordinate */
    y,                          /**< Starting Y coordinate */
    w,                          /**< Width */
    h;                          /**< Height */
  int line[32];                 /**< Left starting position for each line */
};

/** HelpLink structure.
    This is what the HTML viewer considers a "link"
	*/
struct HelpLink {
  char filename[192],           /**< Reference filename */
    name[32];                   /**< Link target (blank if none) */
  int x,                        /**< X offset of link text */
    y,                          /**< Y offset of link text */
    w,                         /**< Width of link text */
    h;                          /**< Height of link text */
};

/** HelpTarget structure.
    This is the target of an image/link
*/
struct HelpTarget {
  char name[32];                /**< Target name */
  int y;                        /**< Y offset of target */
};

/** HelpView class.

The fltk::HelpView widget displays HTML text. Most HTML 2.0 elements
are supported, as well as a primitive implementation of tables.
GIF, JPEG and PNG images are displayed inline. This may change though!

Supported HTML tags:
  - A: HREF/NAME
  - B
  - BODY: BGCOLOR/TEXT/LINK
  - BR
  - CENTER 
  - CODE
  - DD
  - DL
  - DT
  - EM
  - FONT: COLOR/SIZE/FACE=(helvetica/arial/sans/times/serif/symbol/courier)
  - H1/H2/H3/H4/H5/H6
  - HEAD
  - HR
  - I
  - IMG: SRC/WIDTH/HEIGHT/ALT
  - KBD
  - LI
  - OL
  - P
  - PRE
  - STRONG
  - TABLE: TH/TD/TR/BORDER/BGCOLOR/COLSPAN/ALIGN=(CENTER/RIGHT/LEFT)
  - TITLE
  - TT
  - U
  - UL
  - VAR
  
Supported colour names:
  - black,red,green,yellow,blue,magenta,fuchsia,cyan,aqua,white,gray,grey,lime,maroon,navy,olive,purple,silver,teal.

Supported URLs:
  - Internal: file:
  - External: http: ftp: https: ipp: mailto: news:
  
Quoted char names:
  - Aacute aacute Acirc acirc acute AElig aelig Agrave agrave amp Aring aring Atilde atilde Auml auml
  - brvbar bull
  - Ccedil ccedil cedil cent copy curren
  - deg divide
  - Eacute eacute Ecirc ecirc Egrave egrave ETH eth Euml euml euro
  - frac12 frac14 frac34
  - gt
  - Iacute iacute Icirc icirc iexcl Igrave igrave iquest Iuml iuml
  - laquo lt
  - macr micro middot
  - nbsp not Ntilde ntilde
  - Oacute oacute Ocirc ocirc Ograve ograve ordf ordm Oslash oslash Otilde otilde Ouml ouml
  - para premil plusmn pound
  - quot
  - raquo reg
  - sect shy sup1 sup2 sup3 szlig
  - THORN thorn times trade
  - Uacute uacute Ucirc ucirc Ugrave ugrave uml Uuml uuml
  - Yacute yacute
  - yen Yuml yuml
*/
class FL_API HelpView : public Group    //// Help viewer widget
{
  /** Alignments */
  enum { 
    RIGHT = -1, /**<Align right */
    CENTER, /**<Align in the center */
    LEFT  /**< Align left */
  };

  char title_[1024];            /**< Title string */
  Color defcolor_,           	/**< Default text color */
    bgcolor_,                   /**< Background color */
    textcolor_,                 /**< Text color */
    linkcolor_;                 /**< Link color */
  Font *textfont_;              /**< Default font for text */
  int textsize_;                /**< Default font size */
  const char *value_;           /**< HTML text value */

  int nblocks_,                 /**< Number of blocks/paragraphs */
    ablocks_;                   /**< Allocated blocks */
  HelpBlock *blocks_;       	/**< Blocks */

  int nfonts_;                  /**< Number of fonts in stack */
  Font *fonts_[100];         	/**< Font stack */
  int fontsizes_[100];

  HelpFunc *link_;          	/**< Link transform function */

  int nlinks_,                  /**< Number of links */
    alinks_;                    /**< Allocated links */
  HelpLink *links_;         	/**< Links */

  int ntargets_,                /**< Number of targets */
    atargets_;                  /**< Allocated targets */
  HelpTarget *targets_;     	/**< Targets */

  char directory_[1024];        /**< Directory for current file */
  char filename_[1024];         /**< Current filename */
  int topline_,                 /**< Top line in document */
    leftline_,                  /**< Lefthand position */
    size_,                      /**< Total document length */
    hsize_;                     /**< Maximum document width */
  Scrollbar *scrollbar_,      	/**< Vertical scrollbar for document */
    *hscrollbar_;               /**< Horizontal scrollbar */

  HelpBlock *add_block (const char *s, int xx, int yy, int ww, int hh,
                            uchar border = 0);
  void add_link (const char *n, int xx, int yy, int ww, int hh);
  void add_target (const char *n, int yy);
  static int compare_targets (const HelpTarget * t0,
                              const HelpTarget * t1);
  int do_align (HelpBlock * block, int line, int xx, int a, int &l);
  void write_text (const char * buf, const char * ptr, int X, int Y, int X1, int underline);
  void draw ();
  void format ();
  void format_table (int *table_width, int *columns, const char *table);
  int get_align (const char *p, int a);
  const char *get_attr (const char *p, const char *n, char *buf, int bufsize);
  Color get_color (const char *n, Color c);
  SharedImage *get_image (const char *name, int W, int H);
  int get_length (const char *l);
  int handle (int);

  void initfont (Font *&f, int &s);
  void pushfont (Font *f, int s);
  void popfont (Font *&f, int &s);

public:

  HelpView (int xx, int yy, int ww, int hh, const char *l = 0);

  ~HelpView ();
  /** Get the current directory for text in the buffer
  \returns the current directory, or NULL if it doesn't exist */
  const char *directory () const {
    if (directory_[0])
      return (directory_);
    else
      return ((const char *) 0);
  }
  /** Get the current filename for the text in the buffer
  \returns the current filename, or NULL if it doesn't exist */
  const char *filename () const {
    if (filename_[0])
      return (filename_);
    else
      return ((const char *) 0);
  } 
  /**
    This method assigns a callback function to use when a link is
    followed or a file is loaded (via HelpView::load()) that
    requires a different file or path.
			      
    The callback function receives a pointer to the HelpView
    widget and the URI or full pathname for the file in question.
    It must return a pathname that can be opened as a local file or NULL:
							      
    \code
    const char *fn(Widget *w, const char *uri);
    \endcode
											      
    The link function can be used to retrieve remote or virtual
    documents, returning a temporary file that contains the actual
    data. If the link function returns NULL, the value of
    the HelpView widget will remain unchanged.
																      
    If the link callback cannot handle the URI scheme, it should
    return the uri value unchanged or set the value() of the widget
    before returning NULL.
*/
  void link (HelpFunc * fn) {
    link_ = fn;
  }
  int load (const char *f);
  void layout();
  /** Gets the size of the HelpView */
  int size () const { return (size_); } 

  void textcolor (Color c);
  void textfont (Font *f);
  void textsize (int s);
  
  /** Get the current colour
  \returns the current text colour */
  Color textcolor () const { return (defcolor_); } 
  /** Get the current font
  \returns the current text font */
  Font *textfont () const { return (textfont_); } 
  /** Get the current size
  \returns the current text size */
  int textsize () const { return (textsize_); } 
  /** \returns the current document title or NULL if none exists */
  const char *title () { return (title_); }
  void topline (const char *n);
  void topline (int);
  /** \returns the current top line in pixels */
  int topline () const { return (topline_); } 
  void leftline (int);
  /** \returns the current left position in pixels */
  int leftline () const { return (leftline_); } 
  void value (const char *v);
  /** \returns the current buffer contents */
  const char *value () const { return (value_); }
  int find (const char *s,int p);
};

} // namespace fltk

#endif // !HelpView_H

//
// End of "$Id: HelpView.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
