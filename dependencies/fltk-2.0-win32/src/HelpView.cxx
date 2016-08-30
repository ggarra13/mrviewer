//
// "$Id: HelpView.cxx 8799 2011-06-10 06:37:28Z bgbnbigben $"
//
// HelpView widget routines.
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
//     http://www.fltk.org/str.php
//
// Contents:
//
//   HelpView::add_block()       - Add a text block to the list.
//   HelpView::add_link()        - Add a new link to the list.
//   HelpView::add_target()      - Add a new target to the list.
//   HelpView::compare_targets() - Compare two targets.
//   HelpView::do_align()        - Compute the alignment for a line in
//                                     a block.
//   HelpView::draw()            - Draw the HelpView widget.
//   HelpView::format()          - Format the help text.
//   HelpView::format_table()    - Format a table...
//   HelpView::get_align()       - Get an alignment attribute.
//   HelpView::get_attr()        - Get an attribute value from the string.
//   HelpView::get_color()       - Get an alignment attribute.
//   HelpView::handle()          - Handle events in the widget.
//   HelpView::HelpView()    - Build a HelpView widget.
//   HelpView::~HelpView()   - Destroy a HelpView widget.
//   HelpView::load()            - Load the specified file.
//   HelpView::resize()          - Resize the help widget.
//   HelpView::topline()         - Set the top line to the named target.
//   HelpView::topline()         - Set the top line by number.
//   HelpView::value()           - Set the help text directly.
//   scrollbar_callback()            - A callback for the scrollbar.
//
/**\todo isspace probably breaks on UTF8, yet it's used all throughout */

//
// Include necessary header files...
//

#include <fltk/HelpView.h>
#include <fltk/Box.h>
#include <fltk/Font.h>
#include <fltk/xpmImage.h>
#include <fltk/draw.h>
#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>
#include <stdio.h>
#include <stdlib.h>
#include <fltk/string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#if defined(WIN32) && ! defined(__CYGWIN__)
#  include <io.h>
#  include <direct.h>
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#  define getcwd _getcwd
#else
#  include <unistd.h>
#endif // WIN32

#define MAX_COLUMNS	200

#define FOREGROUND_COLOR (Color(0))
#define BACKGROUND_COLOR (Color(49))
#define BACKGROUND2_COLOR (Color(7))
#define SELECTION_COLOR (Color(15))

using namespace fltk;

//
// Typedef the C API sort function type the only way I know how...
//

extern "C"
{
  typedef int (*compare_func_t)(const void *, const void *);
}


//
// Local functions...
//

static int	quote_char(const char *);
static void	scrollbar_callback(Widget *s, void *);
static void	hscrollbar_callback(Widget *s, void *);


//
// Broken image...
//

static const char *broken_xpm[] =
		{
		  "16 24 4 1",
		  "@ c #000000",
		  "  c #ffffff",
		  "+ c none",
		  "x c #ff0000",
		  // pixels
		  "@@@@@@@+++++++++",
		  "@    @++++++++++",
		  "@   @+++++++++++",
		  "@   @++@++++++++",
		  "@    @@+++++++++",
		  "@     @+++@+++++",
		  "@     @++@@++++@",
		  "@ xxx  @@  @++@@",
		  "@  xxx    xx@@ @",
		  "@   xxx  xxx   @",
		  "@    xxxxxx    @",
		  "@     xxxx     @",
		  "@    xxxxxx    @",
		  "@   xxx  xxx   @",
		  "@  xxx    xxx  @",
		  "@ xxx      xxx @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@              @",
		  "@@@@@@@@@@@@@@@@",
		  NULL
		};

static xpmImage broken_image(broken_xpm);


/** Add a text block to the HelpView.

  \param s Pointer to the start of a block of text
  \param xx X position of the block
  \param yy Y position of the block
  \param ww Width of the block
  \param hh Height of the block
  \param border Flag which, if set to a non-zero value, draws the border

  \returns A pointer to the new block is returned
*/
HelpBlock* HelpView::add_block(const char *s, int xx, int yy, int ww, int hh,uchar border) {
  HelpBlock	*temp;				// New block


//  printf("add_block(s = %p, xx = %d, yy = %d, ww = %d, hh = %d, border = %d)\n",
//         s, xx, yy, ww, hh, border);

  if (nblocks_ >= ablocks_)
  {
    ablocks_ += 16;

    if (ablocks_ == 16)
      blocks_ = (HelpBlock *)malloc(sizeof(HelpBlock) * ablocks_);
    else
      blocks_ = (HelpBlock *)realloc(blocks_, sizeof(HelpBlock) * ablocks_);
  }

  temp = blocks_ + nblocks_;
  memset(temp, 0, sizeof(HelpBlock));
  temp->start   = s;
  temp->end     = s;
  temp->x       = xx;
  temp->y       = yy;
  temp->w       = ww;
  temp->h       = hh;
  temp->border  = border;
  temp->bgcolor = bgcolor_;
  nblocks_ ++;

  return (temp);
}


/** Add a new link to the list.
  \param n Name of the link
  \param xx X position of the link
  \param yy Y position of the link
  \param ww Width of the link text
  \param hh Height of the link text
*/
void HelpView::add_link(const char *n, int xx, int yy, int ww, int hh) {
  HelpLink	*temp;			// New link
  char		*target;		// Pointer to target name


  if (nlinks_ >= alinks_)
  {
    alinks_ += 16;

    if (alinks_ == 16)
      links_ = (HelpLink *)malloc(sizeof(HelpLink) * alinks_);
    else
      links_ = (HelpLink *)realloc(links_, sizeof(HelpLink) * alinks_);
  }

  temp = links_ + nlinks_;

  temp->x       = xx;
  temp->y       = yy;
  temp->w       = xx + ww;
  temp->h       = yy + hh;

  strlcpy(temp->filename, n, sizeof(temp->filename));

  if ((target = strrchr(temp->filename, '#')) != NULL)
  {
    *target++ = '\0';
    strlcpy(temp->name, target, sizeof(temp->name));
  }
  else
    temp->name[0] = '\0';

  nlinks_ ++;
}


/** Add a new target to the list.
  \param n Name of the target
  \param yy Y position of the target
*/
void HelpView::add_target(const char *n, int yy) {
  HelpTarget	*temp;			// New target


  if (ntargets_ >= atargets_)
  {
    atargets_ += 16;

    if (atargets_ == 16)
      targets_ = (HelpTarget *)malloc(sizeof(HelpTarget) * atargets_);
    else
      targets_ = (HelpTarget *)realloc(targets_, sizeof(HelpTarget) * atargets_);
  }

  temp = targets_ + ntargets_;

  temp->y = yy;
  strlcpy(temp->name, n, sizeof(temp->name));

  ntargets_ ++;
}


/** Compare two targets with a case-insensitive match.
  \param t0 The first target
  \param t1 The second target
  \return An integer less than 0 if t0 is lexicographically less than t1, zero if t0 and t1 are the same and an integer greater than 0 if t0 is lexicographically greater than t1 (just like strncasecmp!)
*/
int HelpView::compare_targets(const HelpTarget *t0, const HelpTarget *t1) {
  return (strcasecmp(t0->name, t1->name));
}


/** Compute the alignment for a line in a block.
  \param block Block that will be aligned
  \param line The current line
  \param xx Current X position
  \param a The current alignment
  \param l The integer value of the starting link. This is returned after the HelpView has done its computations
  \returns The new position of the line
*/
int HelpView::do_align(HelpBlock *block, int line, int xx, int a, int& l) {
  int	offset;					// Alignment offset


  switch (a)
  {
    case RIGHT :	// Right align
	offset = block->w - xx;
	break;
    case CENTER :	// Center
	offset = (block->w - xx) / 2;
	break;
    default :		// Left align
	offset = 0;
	break;
  }

  block->line[line] = block->x + offset;

  if (line < 31)
    line ++;

  while (l < nlinks_)
  {
    links_[l].x += offset;
    links_[l].w += offset;
    l ++;
  }

  return (line);
}



static void fltk_line(int x0,int y0, int x1,int y1) {fltk::drawline(x0,y0,x1,y1);}
static void fltk_xyline(int x, int y, int x1) {
  fltk::drawline(x,y,x1,y);
}

/** Draws a block of text in the HelpView
  \param buf The text buffer to draw
  \param ptr Currently UNUSED.
  \param X The X position to draw into
  \param Y The Y position to draw into
  \param X1 The width of the underline
  \param underline Whether or not to underline this current piece of text
*/
void HelpView::write_text (const char * buf, const char * ptr, int X, int Y, int X1, int underline) {
    drawtext (buf, (float) X, (float) Y);
    if (underline) {
      fltk_xyline(X, Y+1, X+X1);
    }
}

/** Draw the HelpView widget.
    Like any other FLTK drawing method, you should never call this directly!
*/
void HelpView::draw() {
  int			i;		// Looping var
  const HelpBlock	*block;		// Pointer to current block
  const char		*ptr,		// Pointer to text in block
			*attrs;		// Pointer to start of element attributes
  char			*s,		// Pointer into buffer
			buf[1024],	// Text buffer
			attr[1024];	// Attribute buffer
  int			xx, yy, ww, hh;	// Current positions and sizes
  int			line;		// Current line
  Font*			font; int  fsize;	// Current font and size
  int			head, pre,	// Flags for text
			needspace;	// Do we need whitespace?
  Box *b = box ()? box () : DOWN_BOX;
					// Box to draw...
  int			underline;	// Underline text?
  int                   xtra_ww;        // Extra width for underlined space between words


  // Draw the scrollbar(s) and box first...
  ww = w() ;
  hh = h();
  i  = 0;

  setcolor(bgcolor_);
  box(b);
  draw_box(Rectangle(0, 0, ww, hh));

  if (damage() & (DAMAGE_ALL | DAMAGE_CHILD)) {
    hscrollbar_->set_damage(DAMAGE_ALL);
	scrollbar_->set_damage(DAMAGE_ALL);
  }

  if (hscrollbar_->visible()) {
    update_child (*hscrollbar_);
    hh -= 17;
    i ++;
  }
  if (scrollbar_->visible()) {
    update_child (*scrollbar_);
    ww -= 17;
    i ++;
  }
  if (i == 2) {
    setcolor (GRAY50);
    fillrect(scrollbar_->x(),hscrollbar_->y(),17,17);
  }

  if (!value_)
    return;

  // Clip the drawing to the inside of the box...
  Rectangle tmp(0, 0, w(), h());

  if(scrollbar_->visible())
    tmp.w(tmp.w() - scrollbar_->w());

  if(hscrollbar_->visible())
    tmp.h(tmp.h() - hscrollbar_->h());

  b->inset(tmp);
  fltk::push_clip(tmp);

  setcolor(textcolor_);

  // Draw all visible blocks...
  for (i = 0, block = blocks_; i < nblocks_; i ++, block ++)
    if ((block->y + block->h) >= topline_ && block->y < (topline_ + h()))
    {
      line      = 0;
      xx        = block->line[line];
      yy        = block->y - topline_;
      hh        = 0;
      pre       = 0;
      head      = 0;
      needspace = 0;
      underline = 0;

      initfont(font, fsize);

      for (ptr = block->start, s = buf; ptr < block->end;)
      {
	if ((*ptr == '<' || isspace(*ptr)) && s > buf)
	{
	  if (!head && !pre)
	  {
            // Check width...
            *s = '\0';
            s  = buf;
            ww = (int) getwidth (buf);

            if (needspace && xx > block->x)
              xx += (int) getwidth(" ", 1);

            if ((xx + ww) > block->w)
	    {
	      if (line < 31)
	        line ++;
	      xx = block->line[line];
	      yy += hh;
	      hh = 0;
	    }

            xtra_ww = isspace(*ptr)?(int)getwidth(" ",1):0;
            write_text(buf, ptr, xx - leftline_, yy,ww+xtra_ww, underline);
/*	    if (underline) {
              fltk_xyline(xx - leftline_, yy + 1,
	                xx - leftline_ + ww + xtra_ww);
            }*/

            xx += ww;
	    if ((fsize + 2) > hh)
	      hh = fsize + 2;

	    needspace = 0;
	  }
	  else if (pre)
	  {
	    while (isspace(*ptr))
	    {
	      if (*ptr == '\n')
	      {
	        *s = '\0';
                s = buf;

                write_text (buf, ptr, xx - leftline_, yy, (int)getwidth(buf), underline);
/*		if (underline) fltk_xyline(xx - leftline_, yy + 1,
	                        	 xx - leftline_ + (int)getwidth(buf));*/

		if (line < 31)
	          line ++;
		xx = block->line[line];
		yy += hh;
		hh = fsize + 2;
	      }
	      else if (*ptr == '\t')
	      {
		// Do tabs every 8 columns...
		while (((s - buf) & 7))
	          *s++ = ' ';
	      }
	      else
	        *s++ = ' ';

              if ((fsize + 2) > hh)
	        hh = fsize + 2;

              ptr ++;
	    }

            if (s > buf)
	    {
	      *s = '\0';
	      s = buf;

	      drawtext (buf, float(xx - leftline_), float(yy));
	      ww = (int)getwidth(buf);
	      if (underline) fltk_xyline(xx - leftline_, yy + 1,
	                               xx - leftline_ + ww);
              xx += ww;
	    }

	    needspace = 0;
	  }
	  else
	  {
            s = buf;

	    while (isspace(*ptr))
              ptr ++;
	  }
	}

	if (*ptr == '<')
	{
	  ptr ++;

          if (strncmp(ptr, "!--", 3) == 0)
	  {
	    // Comment...
	    ptr += 3;
	    if ((ptr = strstr(ptr, "-->")) != NULL)
	    {
	      ptr += 3;
	      continue;
	    }
	    else
	      break;
	  }

	  while (*ptr && *ptr != '>' && !isspace(*ptr))
            if (s < (buf + sizeof(buf) - 1))
	      *s++ = *ptr++;
	    else
	      ptr ++;

	  *s = '\0';
	  s = buf;

	  attrs = ptr;
	  while (*ptr && *ptr != '>')
            ptr ++;

	  if (*ptr == '>')
            ptr ++;

	  if (strcasecmp(buf, "HEAD") == 0)
            head = 1;
	  else if (strcasecmp(buf, "BR") == 0)
	  {
	    if (line < 31)
	      line ++;
	    xx = block->line[line];
            yy += hh;
	    hh = 0;
	  }
	  else if (strcasecmp(buf, "HR") == 0)
	  {
	    fltk_line(block->x, yy, block->w, yy);

	    if (line < 31)
	      line ++;
	    xx = block->line[line];
            yy += 2 * hh;
	    hh = 0;
	  }
	  else if (strcasecmp(buf, "CENTER") == 0 ||
        	   strcasecmp(buf, "P") == 0 ||
        	   strcasecmp(buf, "H1") == 0 ||
		   strcasecmp(buf, "H2") == 0 ||
		   strcasecmp(buf, "H3") == 0 ||
		   strcasecmp(buf, "H4") == 0 ||
		   strcasecmp(buf, "H5") == 0 ||
		   strcasecmp(buf, "H6") == 0 ||
		   strcasecmp(buf, "UL") == 0 ||
		   strcasecmp(buf, "OL") == 0 ||
		   strcasecmp(buf, "DL") == 0 ||
		   strcasecmp(buf, "LI") == 0 ||
		   strcasecmp(buf, "DD") == 0 ||
		   strcasecmp(buf, "DT") == 0 ||
		   strcasecmp(buf, "PRE") == 0)
	  {
            if (tolower(buf[0]) == 'h')
	    {
	      font  = HELVETICA_BOLD;
	      fsize = (uchar)(textsize_ + '7' - buf[1]);
	    }
	    else if (strcasecmp(buf, "DT") == 0)
	    {
              font = textfont_->italic();
	      fsize = textsize_;
	    }
	    else if (strcasecmp(buf, "PRE") == 0)
	    {
	      font  = COURIER;
	      fsize = textsize_;
	      pre   = 1;
	    }

            if (strcasecmp(buf, "LI") == 0)
	    {
#ifdef __APPLE_QUARTZ__
              setfont(SYMBOL_FONT, (float) fsize); 
              drawtext ("\245", xx - fsize - leftline_, yy );
#else
              setfont(SYMBOL_FONT, (float) fsize);
              drawtext ("\267",(float) xx - fsize - leftline_, (float) yy);
#endif
	    }

	    pushfont(font, fsize);
	  }
	  else if (strcasecmp(buf, "A") == 0 &&
	           get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	  {
	    setcolor(linkcolor_);
	    underline = 1;
	  }
	  else if (strcasecmp(buf, "/A") == 0)
	  {
	    setcolor(textcolor_);
	    underline = 0;
	  }
	  else if (strcasecmp(buf, "FONT") == 0)
	  {
	    if (get_attr(attrs, "COLOR", attr, sizeof(attr)) != NULL) {
	      setcolor(get_color(attr, textcolor_));
	    }

            if (get_attr(attrs, "FACE", attr, sizeof(attr)) != NULL) {
	      if (!strncasecmp(attr, "helvetica", 9) ||
	          !strncasecmp(attr, "arial", 5) ||
		  !strncasecmp(attr, "sans", 4)) font = HELVETICA;
              else if (!strncasecmp(attr, "times", 5) ||
	               !strncasecmp(attr, "serif", 5)) font = TIMES;
              else if (!strncasecmp(attr, "symbol", 6)) font = SYMBOL_FONT;
	      else font = COURIER;
            }

            if (get_attr(attrs, "SIZE", attr, sizeof(attr)) != NULL) {
              if (isdigit(attr[0] & 255)) {
	        // Absolute size
	        fsize = (int)(textsize_ * pow(1.2, atof(attr) - 3.0));
	      } else {
	        // Relative size
	        fsize = (int)(fsize * pow(1.2, atof(attr) - 3.0));
	      }
	    }

            pushfont(font, fsize);
	  }
	  else if (strcasecmp(buf, "/FONT") == 0)
	  {
	    setcolor(textcolor_);
	    popfont(font, fsize);
	  }
	  else if (strcasecmp(buf, "U") == 0)
	    underline = 1;
	  else if (strcasecmp(buf, "/U") == 0)
	    underline = 0;
	  else if (strcasecmp(buf, "B") == 0 ||
	           strcasecmp(buf, "STRONG") == 0)
	    pushfont(textfont_->bold(), fsize);
	  else if (strcasecmp(buf, "TD") == 0 ||
	           strcasecmp(buf, "TH") == 0)
          {
	    int tx, ty, tw, th;

	    if (tolower(buf[1]) == 'h')
	      pushfont(textfont_->bold(), fsize);
	    else
	      pushfont(font = textfont_, fsize);

            tx = block->x - 4 - leftline_;
	    ty = block->y - topline_ - fsize - 3;
            tw = block->w - block->x + 7;
	    th = block->h + fsize - 5;

            if (tx < 0)
	    {
	      tw += tx;
	      tx  = 0;
	    }

	    if (ty < 0)
	    {
	      th += ty;
	      ty  = 0;
	    }

            if (block->bgcolor != bgcolor_)
	    {
	      setcolor(block->bgcolor);
	      tmp.set(tx, ty, tw, th);
              fillrect(tmp);
              setcolor(textcolor_);
	    }

            if (block->border) {
	      tmp.set(tx, ty, tw, th);
              strokerect(tmp);
	    }
	  }
	  else if (strcasecmp(buf, "I") == 0 ||
                   strcasecmp(buf, "EM") == 0)
	    pushfont(textfont_->italic(), fsize);
	  else if (strcasecmp(buf, "CODE") == 0 ||
	           strcasecmp(buf, "TT") == 0)
	    pushfont(font = COURIER, fsize);
	  else if (strcasecmp(buf, "KBD") == 0)
	    pushfont(font = COURIER_BOLD, fsize);
	  else if (strcasecmp(buf, "VAR") == 0)
	    pushfont(font = COURIER_ITALIC, fsize);
	  else if (strcasecmp(buf, "/HEAD") == 0)
            head = 0;
	  else if (strcasecmp(buf, "/H1") == 0 ||
		   strcasecmp(buf, "/H2") == 0 ||
		   strcasecmp(buf, "/H3") == 0 ||
		   strcasecmp(buf, "/H4") == 0 ||
		   strcasecmp(buf, "/H5") == 0 ||
		   strcasecmp(buf, "/H6") == 0 ||
		   strcasecmp(buf, "/B") == 0 ||
		   strcasecmp(buf, "/STRONG") == 0 ||
		   strcasecmp(buf, "/I") == 0 ||
		   strcasecmp(buf, "/EM") == 0 ||
		   strcasecmp(buf, "/CODE") == 0 ||
		   strcasecmp(buf, "/TT") == 0 ||
		   strcasecmp(buf, "/KBD") == 0 ||
		   strcasecmp(buf, "/VAR") == 0)
	    popfont(font, fsize);
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    popfont(font, fsize);
	    pre = 0;
	  }
	  else if (strcasecmp(buf, "IMG") == 0)
	  {
	    SharedImage *img = 0;
	    int		width, height;
	    char	wattr[8], hattr[8];


            get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
            get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
	    width  = get_length(wattr);
	    height = get_length(hattr);

	    if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
	      img = get_image(attr, width, height);
	      if (!width) width = img->w();
	      if (!height) height = img->h();
	    }

	    if (!width || !height) {
              if (get_attr(attrs, "ALT", attr, sizeof(attr)) == NULL) {
	        strcpy(attr, "IMG");
              }
	    }

	    ww = width;
	    hh = height;

	    if (needspace && xx > block->x)
	      xx += (int)getwidth(" ",1);

	    if ((xx + ww) > block->w)
	    {
	      if (line < 31)
		line ++;

	      xx = block->line[line];
	      yy += hh;
	      hh = 0;
	    }

	    if (img) 
	      img->draw(
		  Rectangle(xx - leftline_,
			    yy - int(textsize()+leading()-getdescent()) + 2,
			    ww,hh)
		);
	    xx += ww;
	    if ((height + 2) > hh)
	      hh = height + 2;

	    needspace = 0;
	  }
	}
	else if (*ptr == '\n' && pre)
	{
	  *s = '\0';
	  s = buf;

          drawtext(buf, (float) xx - leftline_, (float) yy );

	  if (line < 31)
	    line ++;
	  xx = block->line[line];
	  yy += hh;
	  hh = fsize + 2;
	  needspace = 0;

	  ptr ++;
	}
	else if (isspace(*ptr))
	{
	  if (pre)
	  {
	    if (*ptr == ' ')
	      *s++ = ' ';
	    else
	    {
	      // Do tabs every 8 columns...
	      while (((s - buf) & 7))
	        *s++ = ' ';
            }
	  }

          ptr ++;
	  needspace = 1;
	}
	else if (*ptr == '&')
	{
	  ptr ++;

          int qch = quote_char(ptr);

	  if (qch < 0)
	    *s++ = '&';
	  else {
	    *s++ = qch;
	    ptr = strchr(ptr, ';') + 1;
	  }

          if ((fsize + 2) > hh)
	    hh = fsize + 2;
	}
	else
	{
	  *s++ = *ptr++;

          if ((fsize + 2) > hh)
	    hh = fsize + 2;
        }
      }

      *s = '\0';

      if (s > buf && !pre && !head)
      {
	ww = (int)getwidth(buf);

        if (needspace && xx > block->x)
	  xx += (int) getwidth(" ",1);

	if ((xx + ww) > block->w)
	{
	  if (line < 31)
	    line ++;
	  xx = block->line[line];
	  yy += hh;
	  hh = 0;
	}
      }

      if (s > buf && !head)
      {
        drawtext(buf, (float) xx - leftline_, (float) yy );
	if (underline) fltk_xyline(xx - leftline_, yy + 1,
	                         xx - leftline_ + ww);
      }
    }

  pop_clip();
}


/** Find the specified string.
  \param s The string to find
  \param p The starting position to search from
  \return Returns the position the match begins, or -1 if no match is found
*/
int HelpView::find(const char *s, int p) {
  int		i,				// Looping var
		c;				// Current character
  HelpBlock	*b;				// Current block
  const char	*bp,				// Block matching pointer
		*bs,				// Start of current comparison
		*sp;				// Search string pointer


  // Range check input and value...
  if (!s || !value_) return -1;

  if (p < 0 || p >= (int)strlen(value_)) p = 0;
  else if (p > 0) p ++;

  // Look for the string...
  for (i = nblocks_, b = blocks_; i > 0; i --, b ++) {
    if (b->end < (value_ + p))
      continue;

    if (b->start < (value_ + p)) bp = value_ + p;
    else bp = b->start;

    for (sp = s, bs = bp; *sp && *bp && bp < b->end; bp ++) {
      if (*bp == '<') {
        // skip to end of element...
	while (*bp && bp < b->end && *bp != '>') bp ++;
	continue;
      } else if (*bp == '&') {
        // decode HTML entity...
	if ((c = quote_char(bp + 1)) < 0) c = '&';
	else bp = strchr(bp + 1, ';') + 1;
      } else c = *bp;

      if (tolower(*sp) == tolower(c)) sp ++;
      else {
        // No match, so reset to start of search...
	sp = s;
	bs ++;
	bp = bs;
      }
    }

    if (!*sp) {
      // Found a match!
      topline(b->y - b->h);
      return (b->end - value_);
    }
  }

  // No match!
  return (-1);
}


/** Format the help text.

  This looks like it converts everything into HTML from standard text and draws it properly
  \todo FIX THIS DOCUMENTATION!
*/

void HelpView::format() {
  int		i;		// Looping var
  int		done;		// Are we done yet?
  HelpBlock	*block,		// Current block
		*cell;		// Current table cell
  int		cells[MAX_COLUMNS],
				// Cells in the current row...
		row;		// Current table row (block number)
  const char	*ptr,		// Pointer into block
		*start,		// Pointer to start of element
		*attrs;		// Pointer to start of element attributes
  char		*s,		// Pointer into buffer
		buf[1024],	// Text buffer
		attr[1024],	// Attribute buffer
		wattr[1024],	// Width attribute buffer
		hattr[1024],	// Height attribute buffer
		linkdest[1024];	// Link destination
  int		xx, yy, ww, hh;	// Size of current text fragment
  int		line;		// Current line in block
  int		links;		// Links for current line
  Font*		font; int fsize;// Current font and size
  unsigned char	border;		// Draw border?
  int		talign,		// Current alignment
		newalign,	// New alignment
		head,		// In the <HEAD> section?
		pre,		// <PRE> text?
		needspace;	// Do we need whitespace?
  int		table_width,	// Width of table
		table_offset;	// Offset of table
  int		column,		// Current table column number
		columns[MAX_COLUMNS];
				// Column widths
  Color	tc, rc;		// Table/row background color


  // Reset document width...
  hsize_ = w() - 24;

  done = 0;
  while (!done)
  {
    // Reset state variables...
    done       = 1;
    nblocks_   = 0;
    nlinks_    = 0;
    ntargets_  = 0;
    size_      = 0;
    bgcolor_   = color();
    textcolor_ = textcolor();
    linkcolor_ = selection_color();

    tc = rc = bgcolor_;

    strcpy(title_, "Untitled");

    if (!value_)
      return;

    // Setup for formatting...
    initfont(font, fsize);

    line         = 0;
    links        = 0;
    xx           = 4;
    yy           = fsize + 2;
    ww           = 0;
    column       = 0;
    border       = 0;
    hh           = 0;
    block        = add_block(value_, xx, yy, hsize_, 0);
    row          = 0;
    head         = 0;
    pre          = 0;
    talign       = LEFT;
    newalign     = LEFT;
    needspace    = 0;
    linkdest[0]  = '\0';
    table_offset = 0;

    for (ptr = value_, s = buf; *ptr;)
    {
      if ((*ptr == '<' || isspace(*ptr)) && s > buf)
      {
        // Get width...
        *s = '\0';
        ww = (int)getwidth(buf);

	if (!head && !pre)
	{
          // Check width...
          if (ww > hsize_) {
	    hsize_ = ww;
	    done   = 0;
	    break;
	  }

          if (needspace && xx > block->x)
	    ww += (int)getwidth(" ",1);

  //        printf("line = %d, xx = %d, ww = %d, block->x = %d, block->w = %d\n",
  //	       line, xx, ww, block->x, block->w);

          if ((xx + ww) > block->w)
	  {
            line     = do_align(block, line, xx, newalign, links);
	    xx       = block->x;
	    yy       += hh;
	    block->h += hh;
	    hh       = 0;
	  }

          if (linkdest[0])
	    add_link(linkdest, xx, yy - fsize, ww, fsize);

	  xx += ww;
	  if ((fsize + 2) > hh)
	    hh = fsize + 2;

	  needspace = 0;
	}
	else if (pre)
	{
          // Add a link as needed...
          if (linkdest[0])
	    add_link(linkdest, xx, yy - hh, ww, hh);

	  xx += ww;
	  if ((fsize + 2) > hh)
	    hh = fsize + 2;

          // Handle preformatted text...
	  while (isspace(*ptr))
	  {
	    if (*ptr == '\n')
	    {
              if (xx > hsize_) break;

              line     = do_align(block, line, xx, newalign, links);
              xx       = block->x;
	      yy       += hh;
	      block->h += hh;
	      hh       = fsize + 2;
	    }
	    else
              xx += (int)getwidth(" ",1);

            if ((fsize + 2) > hh)
	      hh = fsize + 2;

            ptr ++;
	  }

          if (xx > hsize_) {
	    hsize_ = xx;
	    done   = 0;
	    break;
	  }

	  needspace = 0;
	}
	else
	{
          // Handle normal text or stuff in the <HEAD> section...
	  while (isspace(*ptr))
            ptr ++;
	}

	s = buf;
      }

      if (*ptr == '<')
      {
	start = ptr;
	ptr ++;

        if (strncmp(ptr, "!--", 3) == 0)
	{
	  // Comment...
	  ptr += 3;
	  if ((ptr = strstr(ptr, "-->")) != NULL)
	  {
	    ptr += 3;
	    continue;
	  }
	  else
	    break;
	}

	while (*ptr && *ptr != '>' && !isspace(*ptr))
          if (s < (buf + sizeof(buf) - 1))
            *s++ = *ptr++;
	  else
	    ptr ++;

	*s = '\0';
	s = buf;

//        puts(buf);

	attrs = ptr;
	while (*ptr && *ptr != '>')
          ptr ++;

	if (*ptr == '>')
          ptr ++;

	if (strcasecmp(buf, "HEAD") == 0)
          head = 1;
	else if (strcasecmp(buf, "/HEAD") == 0)
          head = 0;
	else if (strcasecmp(buf, "TITLE") == 0)
	{
          // Copy the title in the document...
          for (s = title_;
	       *ptr != '<' && *ptr && s < (title_ + sizeof(title_) - 1);
	       *s++ = *ptr++);

	  *s = '\0';
	  s = buf;
	}
	else if (strcasecmp(buf, "A") == 0)
	{
          if (get_attr(attrs, "NAME", attr, sizeof(attr)) != NULL)
	    add_target(attr, yy - fsize - 2);

	  if (get_attr(attrs, "HREF", attr, sizeof(attr)) != NULL)
	    strlcpy(linkdest, attr, sizeof(linkdest));
	}
	else if (strcasecmp(buf, "/A") == 0)
          linkdest[0] = '\0';
	else if (strcasecmp(buf, "BODY") == 0)
	{
          bgcolor_   = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)),
	                	 color());
          textcolor_ = get_color(get_attr(attrs, "TEXT", attr, sizeof(attr)),
	                	 textcolor());
          linkcolor_ = get_color(get_attr(attrs, "LINK", attr, sizeof(attr)),
	                	 selection_color());
	}
	else if (strcasecmp(buf, "BR") == 0)
	{
          line     = do_align(block, line, xx, newalign, links);
          xx       = block->x;
	  block->h += hh;
          yy       += hh;
	  hh       = 0;
	}
	else if (strcasecmp(buf, "CENTER") == 0 ||
        	 strcasecmp(buf, "P") == 0 ||
        	 strcasecmp(buf, "H1") == 0 ||
		 strcasecmp(buf, "H2") == 0 ||
		 strcasecmp(buf, "H3") == 0 ||
		 strcasecmp(buf, "H4") == 0 ||
		 strcasecmp(buf, "H5") == 0 ||
		 strcasecmp(buf, "H6") == 0 ||
		 strcasecmp(buf, "UL") == 0 ||
		 strcasecmp(buf, "OL") == 0 ||
		 strcasecmp(buf, "DL") == 0 ||
		 strcasecmp(buf, "LI") == 0 ||
		 strcasecmp(buf, "DD") == 0 ||
		 strcasecmp(buf, "DT") == 0 ||
		 strcasecmp(buf, "HR") == 0 ||
		 strcasecmp(buf, "PRE") == 0 ||
		 strcasecmp(buf, "TABLE") == 0)
	{
          block->end = start;
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->h   += hh;

          if (strcasecmp(buf, "UL") == 0 ||
	      strcasecmp(buf, "OL") == 0 ||
	      strcasecmp(buf, "DL") == 0)
          {
	    block->h += fsize + 2;
	    xx       += 4 * fsize;
	  }
          else if (strcasecmp(buf, "TABLE") == 0)
	  {
	    if (get_attr(attrs, "BORDER", attr, sizeof(attr)))
	      border = (uchar)atoi(attr);
	    else
	      border = 0;

            tc = rc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), bgcolor_);

	    block->h += fsize + 2;

            format_table(&table_width, columns, start);

            if ((xx + table_width) > hsize_) {
#ifdef DEBUG
              printf("xx=%d, table_width=%d, hsize_=%d\n", xx, table_width,
	             hsize_);
#endif // DEBUG
	      hsize_ = xx + table_width;
	      done   = 0;
	      break;
	    }

            switch (get_align(attrs, talign))
	    {
	      default :
	          table_offset = 0;
	          break;

	      case CENTER :
	          table_offset = (hsize_ - table_width) / 2 - textsize_;
	          break;

	      case RIGHT :
	          table_offset = hsize_ - table_width - textsize_;
	          break;
	    }

	    column = 0;
	  }

          if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
	  {
	    font  = HELVETICA_BOLD;
	    fsize = (uchar)(textsize_ + '7' - buf[1]);
	  }
	  else if (strcasecmp(buf, "DT") == 0)
	  {
            font = textfont_->italic();
	    fsize = textsize_;
	  }
	  else if (strcasecmp(buf, "PRE") == 0)
	  {
	    font  = COURIER;
	    fsize = textsize_;
	    pre   = 1;
	  }
	  else
	  {
	    font  = textfont_;
	    fsize = textsize_;
	  }

	  pushfont(font, fsize);

          yy = block->y + block->h;
          hh = 0;

          if ((tolower(buf[0]) == 'h' && isdigit(buf[1])) ||
	      strcasecmp(buf, "DD") == 0 ||
	      strcasecmp(buf, "DT") == 0 ||
	      strcasecmp(buf, "P") == 0)
            yy += fsize + 2;
	  else if (strcasecmp(buf, "HR") == 0)
	  {
	    hh += 2 * fsize;
	    yy += fsize;
	  }

          if (row)
	    block = add_block(start, xx, yy, block->w, 0);
	  else
	    block = add_block(start, xx, yy, hsize_, 0);

	  needspace = 0;
	  line      = 0;

	  if (strcasecmp(buf, "CENTER") == 0)
	    newalign = talign = CENTER;
	  else
	    newalign = get_align(attrs, talign);
	}
	else if (strcasecmp(buf, "/CENTER") == 0 ||
		 strcasecmp(buf, "/P") == 0 ||
		 strcasecmp(buf, "/H1") == 0 ||
		 strcasecmp(buf, "/H2") == 0 ||
		 strcasecmp(buf, "/H3") == 0 ||
		 strcasecmp(buf, "/H4") == 0 ||
		 strcasecmp(buf, "/H5") == 0 ||
		 strcasecmp(buf, "/H6") == 0 ||
		 strcasecmp(buf, "/PRE") == 0 ||
		 strcasecmp(buf, "/UL") == 0 ||
		 strcasecmp(buf, "/OL") == 0 ||
		 strcasecmp(buf, "/DL") == 0 ||
		 strcasecmp(buf, "/TABLE") == 0)
	{
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->end = ptr;

          if (strcasecmp(buf, "/UL") == 0 ||
	      strcasecmp(buf, "/OL") == 0 ||
	      strcasecmp(buf, "/DL") == 0)
	  {
	    xx       -= 4 * fsize;
	    block->h += fsize + 2;
	  }
          else if (strcasecmp(buf, "/TABLE") == 0) 
          {
	    block->h += fsize + 2;
            // the current block is *not* the table block, so the current xx is 
            // meaningless. Set it back to page x, so the next block will be aligned 
            // reasonably. This fails fro table-in-table html!
            xx = 4;
          }
	  else if (strcasecmp(buf, "/PRE") == 0)
	  {
	    pre = 0;
	    hh  = 0;
	  }
	  else if (strcasecmp(buf, "/CENTER") == 0)
	    talign = LEFT;

          popfont(font, fsize);

          while (isspace(*ptr))
	    ptr ++;

          block->h += hh;
          yy       += hh;

          if (tolower(buf[2]) == 'l')
            yy += fsize + 2;

          if (row)
	    block = add_block(ptr, xx, yy, block->w, 0);
	  else
	    block = add_block(ptr, xx, yy, hsize_, 0);

	  needspace = 0;
	  hh        = 0;
	  line      = 0;
	  newalign  = talign;
	}
	else if (strcasecmp(buf, "TR") == 0)
	{
          block->end = start;
          line       = do_align(block, line, xx, newalign, links);
          xx         = block->x;
          block->h   += hh;

          if (row)
	  {
            yy = blocks_[row].y + blocks_[row].h;

	    for (cell = blocks_ + row + 1; cell <= block; cell ++)
	      if ((cell->y + cell->h) > yy)
		yy = cell->y + cell->h;

            block = blocks_ + row;

            block->h = yy - block->y + 2;

	    for (i = 0; i < column; i ++)
	      if (cells[i])
	      {
		cell = blocks_ + cells[i];
		cell->h = block->h;
	      }
	  }

          memset(cells, 0, sizeof(cells));

	  yy        = block->y + block->h - 4;
	  hh        = 0;
          block     = add_block(start, xx, yy, hsize_, 0);
	  row       = block - blocks_;
	  needspace = 0;
	  column    = 0;
	  line      = 0;

          rc = get_color(get_attr(attrs, "BGCOLOR", attr, sizeof(attr)), tc);
	}
	else if (strcasecmp(buf, "/TR") == 0 && row)
	{
          line       = do_align(block, line, xx, newalign, links);
          block->end = start;
	  block->h   += hh;

          xx = blocks_[row].x;

          yy = blocks_[row].y + blocks_[row].h;

	  for (cell = blocks_ + row + 1; cell <= block; cell ++)
	    if ((cell->y + cell->h) > yy)
	      yy = cell->y + cell->h;

          block = blocks_ + row;

          block->h = yy - block->y + 2;

	  for (i = 0; i < column; i ++)
	    if (cells[i])
	    {
	      cell = blocks_ + cells[i];
	      cell->h = block->h;
	    }

	  yy        = block->y + block->h - 4;
          block     = add_block(start, xx, yy, hsize_, 0);
	  needspace = 0;
	  row       = 0;
	  line      = 0;
	}
	else if ((strcasecmp(buf, "TD") == 0 ||
                  strcasecmp(buf, "TH") == 0) && row)
	{
          int	colspan;		// COLSPAN attribute


          line       = do_align(block, line, xx, newalign, links);
          block->end = start;
	  block->h   += hh;

          if (strcasecmp(buf, "TH") == 0)
            font = textfont_->bold();
	  else
	    font = textfont_;

          fsize = textsize_;

          xx = blocks_[row].x + fsize + 3 + table_offset;
	  for (i = 0; i < column; i ++)
	    xx += columns[i] + 6;

          if (get_attr(attrs, "COLSPAN", attr, sizeof(attr)) != NULL)
	    colspan = atoi(attr);
	  else
	    colspan = 1;

          for (i = 0, ww = -6; i < colspan; i ++)
	    ww += columns[column + i] + 6;

          if (block->end == block->start && nblocks_ > 1)
	  {
	    nblocks_ --;
	    block --;
	  }

	  pushfont(font, fsize);

	  yy        = blocks_[row].y;
	  hh        = 0;
          block     = add_block(start, xx, yy, xx + ww, 0, border);
	  needspace = 0;
	  line      = 0;
	  newalign  = get_align(attrs, tolower(buf[1]) == 'h' ? CENTER : LEFT);
	  talign    = newalign;

          cells[column] = block - blocks_;

	  column += colspan;

          block->bgcolor = get_color(get_attr(attrs, "BGCOLOR", attr,
	                                      sizeof(attr)), rc);
	}
	else if ((strcasecmp(buf, "/TD") == 0 ||
                  strcasecmp(buf, "/TH") == 0) && row)
	{
          popfont(font, fsize);
	}
	  else if (strcasecmp(buf, "FONT") == 0)
	  {
            if (get_attr(attrs, "FACE", attr, sizeof(attr)) != NULL) {
	      if (!strncasecmp(attr, "helvetica", 9) ||
	          !strncasecmp(attr, "arial", 5) ||
		  !strncasecmp(attr, "sans", 4)) font = HELVETICA;
              else if (!strncasecmp(attr, "times", 5) ||
	               !strncasecmp(attr, "serif", 5)) font = TIMES;
              else if (!strncasecmp(attr, "symbol", 6)) font = SYMBOL_FONT;
	      else font = COURIER;
            }

            if (get_attr(attrs, "SIZE", attr, sizeof(attr)) != NULL) {
              if (isdigit(attr[0] & 255)) {
	        // Absolute size
	        fsize = (int)(textsize_ * pow(1.2, atoi(attr) - 3.0));
	      } else {
	        // Relative size
	        fsize = (int)(fsize * pow(1.2, atoi(attr)));
	      }
	    }

            pushfont(font, fsize);
	  }
	  else if (strcasecmp(buf, "/FONT") == 0)
	    popfont(font, fsize);
	else if (strcasecmp(buf, "B") == 0 ||
        	 strcasecmp(buf, "STRONG") == 0)
	  pushfont(textfont_->bold(), fsize);
	else if (strcasecmp(buf, "I") == 0 ||
        	 strcasecmp(buf, "EM") == 0)
	  pushfont(textfont_->italic(), fsize);
	else if (strcasecmp(buf, "CODE") == 0 ||
	         strcasecmp(buf, "TT") == 0)
	  pushfont(font = COURIER, fsize);
	else if (strcasecmp(buf, "KBD") == 0)
	  pushfont(font = COURIER_BOLD, fsize);
	else if (strcasecmp(buf, "VAR") == 0)
	  pushfont(font = COURIER_ITALIC, fsize);
	else if (strcasecmp(buf, "/B") == 0 ||
		 strcasecmp(buf, "/STRONG") == 0 ||
		 strcasecmp(buf, "/I") == 0 ||
		 strcasecmp(buf, "/EM") == 0 ||
		 strcasecmp(buf, "/CODE") == 0 ||
		 strcasecmp(buf, "/TT") == 0 ||
		 strcasecmp(buf, "/KBD") == 0 ||
		 strcasecmp(buf, "/VAR") == 0)
	  popfont(font, fsize);
	else if (strcasecmp(buf, "IMG") == 0)
	{
	  SharedImage	*img = 0;
	  int		width;
	  int		height;


          get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
          get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
	  width  = get_length(wattr);
	  height = get_length(hattr);

	  if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
	    img    = get_image(attr, width, height);
	    width  = img->w();
	    height = img->h();
	  }

	  ww = width;

          if (ww > hsize_) {
	    hsize_ = ww;
	    done   = 0;
	    break;
	  }

	  if (needspace && xx > block->x)
	    ww += (int)getwidth(" ",1);

	  if ((xx + ww) > block->w)
	  {
	    line     = do_align(block, line, xx, newalign, links);
	    xx       = block->x;
	    yy       += hh;
	    block->h += hh;
	    hh       = 0;
	  }

	  if (linkdest[0])
	    add_link(linkdest, xx, yy - height, ww, height);

	  xx += ww;
	  if ((height + 2) > hh)
	    hh = height + 2;

	  needspace = 0;
	}
      }
      else if (*ptr == '\n' && pre)
      {
	if (linkdest[0])
	  add_link(linkdest, xx, yy - hh, ww, hh);

        if (xx > hsize_) {
	  hsize_ = xx;
          done   = 0;
	  break;
	}

	line      = do_align(block, line, xx, newalign, links);
	xx        = block->x;
	yy        += hh;
	block->h  += hh;
	needspace = 0;
	ptr ++;
      }
      else if (isspace(*ptr))
      {
	needspace = 1;

	ptr ++;
      }
      else if (*ptr == '&' && s < (buf + sizeof(buf) - 1))
      {
	ptr ++;

        int qch = quote_char(ptr);

	if (qch < 0)
	  *s++ = '&';
	else {
	  *s++ = qch;
	  ptr = strchr(ptr, ';') + 1;
	}

	if ((fsize + 2) > hh)
          hh = fsize + 2;
      }
      else
      {
	if (s < (buf + sizeof(buf) - 1))
          *s++ = *ptr++;
	else
          ptr ++;

	if ((fsize + 2) > hh)
          hh = fsize + 2;
      }
    }

    if (s > buf && !head)
    {
      *s = '\0';
      ww = (int)getwidth(buf);

  //    printf("line = %d, xx = %d, ww = %d, block->x = %d, block->w = %d\n",
  //	   line, xx, ww, block->x, block->w);

      if (ww > hsize_) {
	hsize_ = ww;
	done   = 0;
	break;
      }

      if (needspace && xx > block->x)
	ww += (int)getwidth(" ",1);

      if ((xx + ww) > block->w)
      {
	line     = do_align(block, line, xx, newalign, links);
	xx       = block->x;
	yy       += hh;
	block->h += hh;
	hh       = 0;
      }

      if (linkdest[0])
	add_link(linkdest, xx, yy - fsize, ww, fsize);

      xx += ww;
    }

    do_align(block, line, xx, newalign, links);

    block->end = ptr;
    size_      = yy + hh;
  }


  if (ntargets_ > 1)
    qsort(targets_, ntargets_, sizeof(HelpTarget),
          (compare_func_t)compare_targets);

  // Reset scrolling if it needs to be...
  if (scrollbar_->visible()) {
    int temph = h() - 8;
    if (hscrollbar_->visible()) temph -= 16;
    if ((topline_ + temph) > size_) topline(size_ - temph);
    else topline(topline_);
  } else topline(0);

  if (hscrollbar_->visible()) {
    int tempw = w() - 24;
    if ((leftline_ + tempw) > hsize_) leftline(hsize_ - tempw);
    else leftline(leftline_);
  } else leftline(0);
}


/** Format a table in the HelpView
  \param[out] table_width The total table width, returned
  \param[out] columns The column widths, returned
  \param table Pointer to the start of the HTML table to format
*/
void HelpView::format_table(int *table_width, int *columns, const char *table) {
  int		column,					// Current column
		num_columns,				// Number of columns
		colspan,				// COLSPAN attribute
		width,					// Current width
		temp_width,				// Temporary width
		max_width,				// Maximum width
		incell,					// In a table cell?
		pre,					// <PRE> text?
		needspace;				// Need whitespace?
  char		*s,					// Pointer into buffer
		buf[1024],				// Text buffer
		attr[1024],				// Other attribute
		wattr[1024],				// WIDTH attribute
		hattr[1024];				// HEIGHT attribute
  const char	*ptr,					// Pointer into table
		*attrs,					// Pointer to attributes
		*start;					// Start of element
  int		minwidths[MAX_COLUMNS];			// Minimum widths for each column
  Font* font; int fsize;				// Current font and size


  // Clear widths...
  *table_width = 0;
  for (column = 0; column < MAX_COLUMNS; column ++)
  {
    columns[column]   = 0;
    minwidths[column] = 0;
  }

  num_columns = 0;
  colspan     = 0;
  max_width   = 0;
  pre         = 0;
  needspace   = 0;
  font        = fonts_[nfonts_];
  fsize       = fontsizes_[nfonts_];

  // Scan the table...
  for (ptr = table, column = -1, width = 0, s = buf, incell = 0; *ptr;)
  {
    if ((*ptr == '<' || isspace(*ptr)) && s > buf && incell)
    {
      // Check width...
      if (needspace)
      {
        *s++      = ' ';
	needspace = 0;
      }

      *s         = '\0';
      temp_width = (int)getwidth(buf);
      s          = buf;

      if (temp_width > minwidths[column])
        minwidths[column] = temp_width;

      width += temp_width;

      if (width > max_width)
        max_width = width;
    }

    if (*ptr == '<')
    {
      start = ptr;

      for (s = buf, ptr ++; *ptr && *ptr != '>' && !isspace(*ptr);)
        if (s < (buf + sizeof(buf) - 1))
          *s++ = *ptr++;
	else
	  ptr ++;

      *s = '\0';
      s = buf;

      attrs = ptr;
      while (*ptr && *ptr != '>')
        ptr ++;

      if (*ptr == '>')
        ptr ++;

      if (strcasecmp(buf, "BR") == 0 ||
	  strcasecmp(buf, "HR") == 0)
      {
        width     = 0;
	needspace = 0;
      }
      else if (strcasecmp(buf, "TABLE") == 0 && start > table)
        break;
      else if (strcasecmp(buf, "CENTER") == 0 ||
               strcasecmp(buf, "P") == 0 ||
               strcasecmp(buf, "H1") == 0 ||
	       strcasecmp(buf, "H2") == 0 ||
	       strcasecmp(buf, "H3") == 0 ||
	       strcasecmp(buf, "H4") == 0 ||
	       strcasecmp(buf, "H5") == 0 ||
	       strcasecmp(buf, "H6") == 0 ||
	       strcasecmp(buf, "UL") == 0 ||
	       strcasecmp(buf, "OL") == 0 ||
	       strcasecmp(buf, "DL") == 0 ||
	       strcasecmp(buf, "LI") == 0 ||
	       strcasecmp(buf, "DD") == 0 ||
	       strcasecmp(buf, "DT") == 0 ||
	       strcasecmp(buf, "PRE") == 0)
      {
        width     = 0;
	needspace = 0;

        if (tolower(buf[0]) == 'h' && isdigit(buf[1]))
	{
	  font  = HELVETICA_BOLD;
	  fsize = (uchar)(textsize_ + '7' - buf[1]);
	}
	else if (strcasecmp(buf, "DT") == 0)
	{
          font = textfont_->italic();
	  fsize = textsize_;
	}
	else if (strcasecmp(buf, "PRE") == 0)
	{
	  font  = COURIER;
	  fsize = textsize_;
	  pre   = 1;
	}
	else if (strcasecmp(buf, "LI") == 0)
	{
	  width  += 4 * fsize;
	  font   = textfont_;
	  fsize  = textsize_;
	}
	else
	{
	  font  = textfont_;
	  fsize = textsize_;
	}

	pushfont(font, fsize);
      }
      else if (strcasecmp(buf, "/CENTER") == 0 ||
	       strcasecmp(buf, "/P") == 0 ||
	       strcasecmp(buf, "/H1") == 0 ||
	       strcasecmp(buf, "/H2") == 0 ||
	       strcasecmp(buf, "/H3") == 0 ||
	       strcasecmp(buf, "/H4") == 0 ||
	       strcasecmp(buf, "/H5") == 0 ||
	       strcasecmp(buf, "/H6") == 0 ||
	       strcasecmp(buf, "/PRE") == 0 ||
	       strcasecmp(buf, "/UL") == 0 ||
	       strcasecmp(buf, "/OL") == 0 ||
	       strcasecmp(buf, "/DL") == 0)
      {
        width     = 0;
	needspace = 0;

        popfont(font, fsize);
      }
      else if (strcasecmp(buf, "TR") == 0 || strcasecmp(buf, "/TR") == 0 ||
               strcasecmp(buf, "/TABLE") == 0)
      {
//        printf("%s column = %d, colspan = %d, num_columns = %d\n",
//	       buf, column, colspan, num_columns);

        if (column >= 0)
	{
	  // This is a hack to support COLSPAN...
	  max_width /= colspan;

	  while (colspan > 0)
	  {
	    if (max_width > columns[column])
	      columns[column] = max_width;

	    column ++;
	    colspan --;
	  }
	}

	if (strcasecmp(buf, "/TABLE") == 0)
	  break;

	needspace = 0;
	column    = -1;
	width     = 0;
	max_width = 0;
	incell    = 0;
      }
      else if (strcasecmp(buf, "TD") == 0 ||
               strcasecmp(buf, "TH") == 0)
      {
//        printf("BEFORE column = %d, colspan = %d, num_columns = %d\n",
//	       column, colspan, num_columns);

        if (column >= 0)
	{
	  // This is a hack to support COLSPAN...
	  max_width /= colspan;

	  while (colspan > 0)
	  {
	    if (max_width > columns[column])
	      columns[column] = max_width;

	    column ++;
	    colspan --;
	  }
	}
	else
	  column ++;

        if (get_attr(attrs, "COLSPAN", attr, sizeof(attr)) != NULL)
	  colspan = atoi(attr);
	else
	  colspan = 1;

//        printf("AFTER column = %d, colspan = %d, num_columns = %d\n",
//	       column, colspan, num_columns);

        if ((column + colspan) >= num_columns)
	  num_columns = column + colspan;

	needspace = 0;
	width     = 0;
	incell    = 1;

        if (strcasecmp(buf, "TH") == 0)
              font = textfont_->bold();
	else
	  font = textfont_;

        fsize = textsize_;

	pushfont(font, fsize);

        if (get_attr(attrs, "WIDTH", attr, sizeof(attr)) != NULL)
	  max_width = get_length(attr);
	else
	  max_width = 0;

//        printf("max_width = %d\n", max_width);
      }
      else if (strcasecmp(buf, "/TD") == 0 ||
               strcasecmp(buf, "/TH") == 0)
      {
	incell = 0;
        popfont(font, fsize);
      }
      else if (strcasecmp(buf, "B") == 0 ||
               strcasecmp(buf, "STRONG") == 0)
	pushfont(textfont_->bold(), fsize);
      else if (strcasecmp(buf, "I") == 0 ||
               strcasecmp(buf, "EM") == 0)
	pushfont(textfont_->italic(), fsize);
      else if (strcasecmp(buf, "CODE") == 0 ||
               strcasecmp(buf, "TT") == 0)
	pushfont(font = COURIER, fsize);
      else if (strcasecmp(buf, "KBD") == 0)
	pushfont(font = COURIER_BOLD, fsize);
      else if (strcasecmp(buf, "VAR") == 0)
	pushfont(font = COURIER_ITALIC, fsize);
      else if (strcasecmp(buf, "/B") == 0 ||
	       strcasecmp(buf, "/STRONG") == 0 ||
	       strcasecmp(buf, "/I") == 0 ||
	       strcasecmp(buf, "/EM") == 0 ||
	       strcasecmp(buf, "/CODE") == 0 ||
	       strcasecmp(buf, "/TT") == 0 ||
	       strcasecmp(buf, "/KBD") == 0 ||
	       strcasecmp(buf, "/VAR") == 0)
	popfont(font, fsize);
      else if (strcasecmp(buf, "IMG") == 0 && incell)
      {
	SharedImage	*img = 0;
	int		iwidth, iheight;


        get_attr(attrs, "WIDTH", wattr, sizeof(wattr));
        get_attr(attrs, "HEIGHT", hattr, sizeof(hattr));
	iwidth  = get_length(wattr);
	iheight = get_length(hattr);

        if (get_attr(attrs, "SRC", attr, sizeof(attr))) {
	  img     = get_image(attr, iwidth, iheight);
	  img->measure(iwidth, iheight);
	}

	if (iwidth > minwidths[column])
          minwidths[column] = iwidth;

        width += iwidth;
	if (needspace)
	  width += (int)getwidth(" ",1);

	if (width > max_width)
          max_width = width;

	needspace = 0;
      }
    }
    else if (*ptr == '\n' && pre)
    {
      width     = 0;
      needspace = 0;
      ptr ++;
    }
    else if (isspace(*ptr))
    {
      needspace = 1;

      ptr ++;
    }
    else if (*ptr == '&' && s < (buf + sizeof(buf) - 1))
    {
      ptr ++;

      int qch = quote_char(ptr);

      if (qch < 0)
	*s++ = '&';
      else {
	*s++ = qch;
	ptr = strchr(ptr, ';') + 1;
      }
    }
    else
    {
      if (s < (buf + sizeof(buf) - 1))
        *s++ = *ptr++;
      else
        ptr ++;
    }
  }

  // Now that we have scanned the entire table, adjust the table and
  // cell widths to fit on the screen...
  if (get_attr(table + 6, "WIDTH", attr, sizeof(attr)))
    *table_width = get_length(attr);
  else
    *table_width = 0;

#ifdef DEBUG
  printf("num_columns = %d, table_width = %d\n", num_columns, *table_width);
#endif // DEBUG

  if (num_columns == 0)
    return;

  // Add up the widths...
  for (column = 0, width = 0; column < num_columns; column ++)
    width += columns[column];

#ifdef DEBUG
  printf("width = %d, w() = %d\n", width, w());
  for (column = 0; column < num_columns; column ++)
    printf("    columns[%d] = %d, minwidths[%d] = %d\n", column, columns[column],
           column, minwidths[column]);
#endif // DEBUG

  // Adjust the width if needed...
  int scale_width = *table_width;

  if (scale_width == 0) {
    if (width > (hsize_ - 24)) scale_width = hsize_ - 24;
    else scale_width = width;
  }

  if (width < scale_width) {
#ifdef DEBUG
    printf("Scaling table up to %d from %d...\n", scale_width, width);
#endif // DEBUG

    *table_width = 0;

    scale_width = (scale_width - width) / num_columns;

#ifdef DEBUG
    printf("adjusted scale_width = %d\n", scale_width);
#endif // DEBUG

    for (column = 0; column < num_columns; column ++) {
      columns[column] += scale_width;

      (*table_width) += columns[column];
    }
  }
  else if (width > scale_width) {
#ifdef DEBUG
    printf("Scaling table down to %d from %d...\n", scale_width, width);
#endif // DEBUG

    for (column = 0; column < num_columns; column ++) {
      width       -= minwidths[column];
      scale_width -= minwidths[column];
    }

#ifdef DEBUG
    printf("adjusted width = %d, scale_width = %d\n", width, scale_width);
#endif // DEBUG

    if (width > 0) {
      for (column = 0; column < num_columns; column ++) {
	columns[column] -= minwidths[column];
	columns[column] = scale_width * columns[column] / width;
	columns[column] += minwidths[column];
      }
    }

    *table_width = 0;
    for (column = 0; column < num_columns; column ++) {
      (*table_width) += columns[column];
    }
  }
  else if (*table_width == 0)
    *table_width = width;

#ifdef DEBUG
  printf("FINAL table_width = %d\n", *table_width);
  for (column = 0; column < num_columns; column ++)
    printf("    columns[%d] = %d\n", column, columns[column]);
#endif // DEBUG
}


/** Get an alignment attribute (i.e. HTML's LEFT/CENTER/RIGHT).
  \param p Pointer to the start of attributes
  \param a The default alignment
  \returns The alignment attrbute
*/
int HelpView::get_align(const char *p, int a) {
  char	buf[255];			// Alignment value


  if (get_attr(p, "ALIGN", buf, sizeof(buf)) == NULL)
    return (a);

  if (strcasecmp(buf, "CENTER") == 0)
    return (CENTER);
  else if (strcasecmp(buf, "RIGHT") == 0)
    return (RIGHT);
  else
    return (LEFT);
}


/** Get a HTML attribute value from the string.
  \param p Pointer to the start of the attribute
  \param n Name of the attribute
  \param[out] buf Buffer for the attribute values, returned to the user. Terminated by a single '\0'
  \param bufsize Length of the buffer
  \return Pointer to the start of the buffer, or NULL if no attributes exist.
*/
const char* HelpView::get_attr(const char *p, const char *n, char *buf, int bufsize) {
  char	name[255],				// Name from string
	*ptr,					// Pointer into name or value
	quote;					// Quote


  buf[0] = '\0';

  while (*p && *p != '>')
  {
    while (isspace(*p))
      p ++;

    if (*p == '>' || !*p)
      return (NULL);

    for (ptr = name; *p && !isspace(*p) && *p != '=' && *p != '>';)
      if (ptr < (name + sizeof(name) - 1))
        *ptr++ = *p++;
      else
        p ++;

    *ptr = '\0';

    if (isspace(*p) || !*p || *p == '>')
      buf[0] = '\0';
    else
    {
      if (*p == '=')
        p ++;

      for (ptr = buf; *p && !isspace(*p) && *p != '>';)
        if (*p == '\'' || *p == '\"')
	{
	  quote = *p++;

	  while (*p && *p != quote)
	    if ((ptr - buf + 1) < bufsize)
	      *ptr++ = *p++;
	    else
	      p ++;

          if (*p == quote)
	    p ++;
	}
	else if ((ptr - buf + 1) < bufsize)
	  *ptr++ = *p++;
	else
	  p ++;

      *ptr = '\0';
    }

    if (strcasecmp(n, name) == 0)
      return (buf);
    else
      buf[0] = '\0';

    if (*p == '>')
      return (NULL);
  }

  return (NULL);
}


/** Get a colour's alignment attribute.
  \param n Colour name
  \param c Default fltk::Color value
  \return The fltk::Color corresponding to this allignment attribute
*/
Color HelpView::get_color(const char *n, Color c) {
  int	i;				// Looping var
  int	rgb, r, g, b;			// RGB values
  static const struct {			// Color name table
    const char *name;
    int   r, g, b;
  }	colors[] = {
    { "black",		0x00, 0x00, 0x00 },
    { "red",		0xff, 0x00, 0x00 },
    { "green",		0x00, 0x80, 0x00 },
    { "yellow",		0xff, 0xff, 0x00 },
    { "blue",		0x00, 0x00, 0xff },
    { "magenta",	0xff, 0x00, 0xff },
    { "fuchsia",	0xff, 0x00, 0xff },
    { "cyan",		0x00, 0xff, 0xff },
    { "aqua",		0x00, 0xff, 0xff },
    { "white",		0xff, 0xff, 0xff },
    { "gray",		0x80, 0x80, 0x80 },
    { "grey",		0x80, 0x80, 0x80 },
    { "lime",		0x00, 0xff, 0x00 },
    { "maroon",		0x80, 0x00, 0x00 },
    { "navy",		0x00, 0x00, 0x80 },
    { "olive",		0x80, 0x80, 0x00 },
    { "purple",		0x80, 0x00, 0x80 },
    { "silver",		0xc0, 0xc0, 0xc0 },
    { "teal",		0x00, 0x80, 0x80 }
  };


  if (!n || !n[0]) return c;

  if (n[0] == '#') {
    // Do hex color lookup
    rgb = strtol(n + 1, NULL, 16);

    if (strlen(n) > 4) {
      r = rgb >> 16;
      g = (rgb >> 8) & 255;
      b = rgb & 255;
    } else {
      r = (rgb >> 8) * 17;
      g = ((rgb >> 4) & 15) * 17;
      b = (rgb & 15) * 17;
    }
    return fltk::color(r, g, b);
  } else {
    for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
      if (!strcasecmp(n, colors[i].name)) {
	  return fltk::color(colors[i].r, colors[i].g, colors[i].b);
      }
    return c;
  }
}

/** Get an inline image.
  \param name Image name
  \param W image width
  \param H image height
  \return The image requested, or broken_image if the requested image doesn't exist
*/
SharedImage* HelpView::get_image(const char *name, int W, int H) {
  const char	*localname;		// Local filename
  char		dir[1024];		// Current directory
  char		temp[1024],		// Temporary filename
		*tempptr;		// Pointer into temporary name
  SharedImage *ip;			// Image pointer...

  // See if the image can be found...
  if (strchr(directory_, ':') != NULL && strchr(name, ':') == NULL) {
    if (name[0] == '/') {
      strlcpy(temp, directory_, sizeof(temp));

      if ((tempptr = strrchr(strchr(directory_, ':') + 3, '/')) != NULL) {
        strlcpy(tempptr, name, sizeof(temp) - (tempptr - temp));
      } else {
        strlcat(temp, name, sizeof(temp));
      }
    } else {
      snprintf(temp, sizeof(temp), "%s/%s", directory_, name);
    }

    if (link_) localname = (*link_)(this, temp);
    else localname = temp;
  } else if (name[0] != '/' && strchr(name, ':') == NULL) {
    if (directory_[0]) snprintf(temp, sizeof(temp), "%s/%s", directory_, name);
    else {
      if(getcwd(dir, sizeof(dir))); //ignore the return value
      snprintf(temp, sizeof(temp), "file:%s/%s", dir, name);
    }

    if (link_) localname = (*link_)(this, temp);
    else localname = temp;
  } else if (link_) localname = (*link_)(this, name);
  else localname = name;

  if (!localname) return 0;

  if (strncmp(localname, "file:", 5) == 0) localname += 5;

  if ((ip = SharedImage::get(localname /*, W, H*/)) == NULL)
    ip = (SharedImage *) & broken_image;

  return ip;
}


/** Get a length value, either absolute or percentage
  \param l Value to be converted, as a string
  \returns A converted length value
*/
int HelpView::get_length(const char *l) {
  int	val;					// Integer value

  if (!l[0]) return 0;

  val = atoi(l);
  if (l[strlen(l) - 1] == '%') {
    if (val > 100) val = 100;
    else if (val < 0) val = 0;

    val = val * (hsize_ - 24) / 100;
  }

  return val;
}


/** Handle events in the widget.
  \param event The event to handle
  \return 1 if the event was handled, 0 if the HelpView couldn't determine the correct thing to do with this event
*/
int HelpView::handle(int event)	{
  int		i;		// Looping var
  int		xx, yy;		// Adjusted mouse position
  HelpLink	*linkp;		// Current link
  char		target[32];	// Current target


  switch (event)
  {
    case PUSH :
	if (Group::handle(event))
	  return (1);

    case MOVE :
        xx = event_x() + leftline_;
        yy = event_y() + topline_;
	break;

    case LEAVE :
        cursor(CURSOR_DEFAULT);

    default :
	return (Group::handle(event));
  }

  // Handle mouse clicks on links...
  for (i = nlinks_, linkp = links_; i > 0; i --, linkp ++)
    if (xx >= linkp->x && xx < linkp->w &&
        yy >= linkp->y && yy < linkp->h)
      break;

  if (!i)
  {
    cursor(CURSOR_DEFAULT);
    return (1);
  }

  // Change the cursor for MOTION events, and go to the link for
  // clicks...
  if (event == MOVE)
    cursor(CURSOR_HAND);
  else
  {
    cursor(CURSOR_DEFAULT);

    strlcpy(target, linkp->name, sizeof(target));

    set_changed();
    relayout();

    if (strcmp(linkp->filename, filename_) != 0 && linkp->filename[0])
    {
      char	dir[1024];	// Current directory
      char	temp[1024],	// Temporary filename
		*tempptr;	// Pointer into temporary filename


      if (strchr(directory_, ':') != NULL &&
          strchr(linkp->filename, ':') == NULL)
      {
	if (linkp->filename[0] == '/')
	{
          strlcpy(temp, directory_, sizeof(temp));
          if ((tempptr = strrchr(strchr(directory_, ':') + 3, '/')) != NULL)
	    strlcpy(tempptr, linkp->filename, sizeof(temp));
	  else
	    strlcat(temp, linkp->filename, sizeof(temp));
	}
	else
	  snprintf(temp, sizeof(temp), "%s/%s", directory_, linkp->filename);
      }
      else if (linkp->filename[0] != '/' && strchr(linkp->filename, ':') == NULL)
      {
	if (directory_[0])
	  snprintf(temp, sizeof(temp), "%s/%s", directory_, linkp->filename);
	else
	{
	  if(getcwd(dir, sizeof(dir))); //ignore the return value
	  snprintf(temp, sizeof(temp), "file:%s/%s", dir, linkp->filename);
	}
      }
      else
        strlcpy(temp, linkp->filename, sizeof(temp));

      if (linkp->name[0])
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "#%s",
	         linkp->name);

      load(temp);
    }
    else if (target[0])
      topline(target);
    else
      topline(0);

    leftline(0);
  }

  return (1);
}

// externalize the font manips non trivial methods as it would be inlined otherwise at  the cost of lib size
void HelpView::initfont (Font *&f, int &s) {
    nfonts_ = 0;
	f = fonts_[0] = textfont_;
	s = fontsizes_[0] = textsize_;
    setfont (f, (float)s-1);
} 

void HelpView::pushfont (Font *f, int s) {
    if (nfonts_ < 99) nfonts_++;
	fonts_[nfonts_] = f;
	fontsizes_[nfonts_] = s;
    setfont (f, (float)s-1);
}

void HelpView::popfont (Font *&f, int &s) {
    if (nfonts_ > 0) nfonts_--;
    f = fonts_[nfonts_];
	s = fontsizes_[nfonts_];
    setfont(f, (float)s-1);
}

/** Sets the current (or default) text colour
 \param c the fltk::Color of the new color
*/
void HelpView::textcolor (Color c) {
if (textcolor_ == defcolor_)
  textcolor_ = c;
defcolor_ = c;
}

/** Sets the current text font and reformats the HelpView
  \param f A pointer to a fltk::Font structure
*/
void HelpView::textfont (Font *f) {
textfont_ = f;
format();
}

/** Changes the text size within the HelpView, then reformats
  \param s Integer height of the text
*/
void HelpView::textsize (int s) {
textsize_ = s;
format ();
}

/** Build a HelpView widget.
  \param xx Left X position
  \param yy Top Y positoin
  \param ww Width in pixels
  \param hh Height in pixels
  \param l  Name of the HelpView
*/
HelpView::HelpView(int xx, int yy, int ww, int hh, const char *l)
    : Group(xx, yy, ww, hh, l)
{
  begin();

  color(BACKGROUND2_COLOR);
  selection_color(SELECTION_COLOR);

  title_[0]     = '\0';
  defcolor_     = FOREGROUND_COLOR;
  bgcolor_      = BACKGROUND_COLOR;
  textcolor_    = FOREGROUND_COLOR;
  linkcolor_    = SELECTION_COLOR;
  textfont_     = TIMES;
  textsize_     = 12;
  value_        = NULL;

  ablocks_      = 0;
  nblocks_      = 0;
  blocks_       = (HelpBlock *)0;

  nfonts_       = 0;

  link_         = (HelpFunc *)0;

  alinks_       = 0;
  nlinks_       = 0;
  links_        = (HelpLink *)0;

  atargets_     = 0;
  ntargets_     = 0;
  targets_      = (HelpTarget *)0;

  directory_[0] = '\0';
  filename_[0]  = '\0';

  topline_      = 0;
  leftline_     = 0;
  size_         = 0;
  hsize_        = 0;

  scrollbar_ = new Scrollbar(ww - 17, yy, 17, hh - 17);
  scrollbar_->value(0, hh, 0, 1);
  scrollbar_->step(8.0);
  scrollbar_->callback(scrollbar_callback);
  scrollbar_->set_vertical();

  hscrollbar_= new Scrollbar(xx, hh - 17, ww - 17, 17);
  hscrollbar_->value(0, ww, 0, 1);
  hscrollbar_->step(8.0);
  hscrollbar_->callback(hscrollbar_callback);

  end();

  //resize(xx, yy, ww, hh);
}


/** Destroy a HelpView widget.

    calls free() on everything the HelpView has dynamically allocated
*/
HelpView::~HelpView()
{
  if (nblocks_)
    free(blocks_);
  if (nlinks_)
    free(links_);
  if (ntargets_)
    free(targets_);
  if (value_)
    free((void *)value_);
}


/** Load the specified file.
  \param f Filename to load (which may also be a target, like http://www.fltk.org/)
  \return 0 on success or -1 for an error
*/
int HelpView::load(const char *f) {
  FILE		*fp;		// File to read from
  long		len;		// Length of file
  char		*target;	// Target in file
  char		*slash;		// Directory separator
  const char	*localname;	// Local filename
  char		error[1024];	// Error buffer
  char		newname[1024];	// New filename buffer


  strlcpy(newname, f, sizeof(newname));
  if ((target = strrchr(newname, '#')) != NULL)
    *target++ = '\0';

  if (link_)
    localname = (*link_)(this, newname);
  else
    localname = filename_;

  if (!localname)
    return (0);

  strlcpy(filename_, newname, sizeof(filename_));
  strlcpy(directory_, newname, sizeof(directory_));

  // Note: We do not support Windows backslashes, since they are illegal
  //       in URLs...
  if ((slash = strrchr(directory_, '/')) == NULL)
    directory_[0] = '\0';
  else if (slash > directory_ && slash[-1] != '/')
    *slash = '\0';

  if (value_ != NULL)
  {
    free((void *)value_);
    value_ = NULL;
  }

  if (strncmp(localname, "ftp:", 4) == 0 ||
      strncmp(localname, "http:", 5) == 0 ||
      strncmp(localname, "https:", 6) == 0 ||
      strncmp(localname, "ipp:", 4) == 0 ||
      strncmp(localname, "mailto:", 7) == 0 ||
      strncmp(localname, "news:", 5) == 0)
  {
    // Remote link wasn't resolved...
    snprintf(error, sizeof(error),
             "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
             "<BODY><H1>Error</H1>"
	     "<P>Unable to follow the link \"%s\" - "
	     "no handler exists for this URI scheme.</P></BODY>",
	     localname);
    value_ = strdup(error);
  }
  else
  {
    if (strncmp(localname, "file:", 5) == 0)
      localname += 5;	// Adjust for local filename...

    if ((fp = fopen(localname, "rb")) != NULL)
    {
      fseek(fp, 0, SEEK_END);
      len = ftell(fp);
      rewind(fp);

      value_ = (const char *)calloc(len + 1, 1);
      if(fread((void *)value_, 1, len, fp)); //ignore the return value
      fclose(fp);
    }
    else
    {
      snprintf(error, sizeof(error),
               "<HTML><HEAD><TITLE>Error</TITLE></HEAD>"
               "<BODY><H1>Error</H1>"
	       "<P>Unable to follow the link \"%s\" - "
	       "%s.</P></BODY>",
	       localname, strerror(errno));
      value_ = strdup(error);
    }
  }

  format();

  if (target)
    topline(target);
  else
    topline(0);

  return (0);
}

/** HelpView's layout method.
  Similar to Widget::layout() in use.
*/
void HelpView::layout() {
  Rectangle inside(0, 0, w(), h());
  Box *b = box() ? box () : DOWN_BOX;
  b->inset(inside);

  if (hsize_ > (inside.w() - 17)) {
    hscrollbar_->show();

    if (size_ <= (inside.h() - 17)) {
      scrollbar_->hide();
      hscrollbar_->resize(inside.x(), inside.b()-17, inside.w(), 17);
	  hscrollbar_->layout();
    } else {
      scrollbar_->show();
      scrollbar_->resize(inside.r()-17, inside.y(), 17, inside.h()-17);
      hscrollbar_->resize(inside.x(), inside.b()-17, inside.w()-17, 17);
    }

  } else {
    hscrollbar_->hide();
    if (size_ <= inside.h())
      scrollbar_->hide();
    else {
      scrollbar_->resize(inside.r()-17, inside.y(), 17, inside.h());
      scrollbar_->show();
    }
  }

  // scrollbars will format() update additionaly
  format();
  Widget::layout();
}


/** Set the top line to the named target.
  \param n The target name
*/
void HelpView::topline(const char *n) {
  HelpTarget key,			// Target name key
		*target;		// Pointer to matching target


  if (ntargets_ == 0)
    return;

  strlcpy(key.name, n, sizeof(key.name));

  target = (HelpTarget *)bsearch(&key, targets_, ntargets_, sizeof(HelpTarget),
                                 (compare_func_t)compare_targets);

  if (target != NULL)
    topline(target->y);
}


/** Set the top line by number.
  \param t The top line number
*/
void HelpView::topline(int t) {
  if (!value_)
    return;

  if (size_ < (h() - 24) || t < 0)
    t = 0;
  else if (t > size_)
    t = size_;

  topline_ = t;

  scrollbar_->value(topline_, h() - 24, 0, size_);

  do_callback();

  redraw();
}


/** Set the left position.
  \param l Left position
*/
void HelpView::leftline(int l) {
  if (!value_)
    return;

  if (hsize_ < (w() - 24) || l < 0)
    l = 0;
  else if (l > hsize_)
    l = hsize_;

  leftline_ = l;

  hscrollbar_->value(leftline_, w() - 24, 0, hsize_);

  redraw();
}


/** Set the help text directly.
  To use this function, store all your HTML in a buffer and then call
  \code
  my_help_view.value(buffer);
  \endcode
  HelpView will automatically attempt to format your HTML
  \param v The text to view
*/
void HelpView::value(const char *v) {
  if (!v)
    return;

  if (value_ != NULL)
    free((void *)value_);

  value_ = strdup(v);

  format();

  set_changed();
  relayout();
  topline(0);
  leftline(0);
}


//
// 'quote_char()' - Return the character code associated with a quoted char.
//

static int			// O - Code or -1 on error
quote_char(const char *p) {	// I - Quoted string
  int	i;			// Looping var
  static struct {
    const char	*name;
    int		namelen;
    int		code;
  }	*nameptr,		// Pointer into name array
	names[] = {		// Quoting names
    { "Aacute;", 7, 193 },
    { "aacute;", 7, 225 },
    { "Acirc;",  6, 194 },
    { "acirc;",  6, 226 },
    { "acute;",  6, 180 },
    { "AElig;",  6, 198 },
    { "aelig;",  6, 230 },
    { "Agrave;", 7, 192 },
    { "agrave;", 7, 224 },
    { "amp;",    4, '&' },
    { "Aring;",  6, 197 },
    { "aring;",  6, 229 },
    { "Atilde;", 7, 195 },
    { "atilde;", 7, 227 },
    { "Auml;",   5, 196 },
    { "auml;",   5, 228 },
    { "brvbar;", 7, 166 },
    { "Ccedil;", 7, 199 },
    { "ccedil;", 7, 231 },
    { "cedil;",  6, 184 },
    { "cent;",   5, 162 },
    { "copy;",   5, 169 },
    { "curren;", 7, 164 },
    { "deg;",    4, 176 },
    { "divide;", 7, 247 },
    { "Eacute;", 7, 201 },
    { "eacute;", 7, 233 },
    { "Ecirc;",  6, 202 },
    { "ecirc;",  6, 234 },
    { "Egrave;", 7, 200 },
    { "egrave;", 7, 232 },
    { "ETH;",    4, 208 },
    { "eth;",    4, 240 },
    { "Euml;",   5, 203 },
    { "euml;",   5, 235 },
    { "frac12;", 7, 189 },
    { "frac14;", 7, 188 },
    { "frac34;", 7, 190 },
    { "gt;",     3, '>' },
    { "Iacute;", 7, 205 },
    { "iacute;", 7, 237 },
    { "Icirc;",  6, 206 },
    { "icirc;",  6, 238 },
    { "iexcl;",  6, 161 },
    { "Igrave;", 7, 204 },
    { "igrave;", 7, 236 },
    { "iquest;", 7, 191 },
    { "Iuml;",   5, 207 },
    { "iuml;",   5, 239 },
    { "laquo;",  6, 171 },
    { "lt;",     3, '<' },
    { "macr;",   5, 175 },
    { "micro;",  6, 181 },
    { "middot;", 7, 183 },
    { "nbsp;",   5, ' ' },
    { "not;",    4, 172 },
    { "Ntilde;", 7, 209 },
    { "ntilde;", 7, 241 },
    { "Oacute;", 7, 211 },
    { "oacute;", 7, 243 },
    { "Ocirc;",  6, 212 },
    { "ocirc;",  6, 244 },
    { "Ograve;", 7, 210 },
    { "ograve;", 7, 242 },
    { "ordf;",   5, 170 },
    { "ordm;",   5, 186 },
    { "Oslash;", 7, 216 },
    { "oslash;", 7, 248 },
    { "Otilde;", 7, 213 },
    { "otilde;", 7, 245 },
    { "Ouml;",   5, 214 },
    { "ouml;",   5, 246 },
    { "para;",   5, 182 },
    { "plusmn;", 7, 177 },
    { "pound;",  6, 163 },
    { "quot;",   5, '\"' },
    { "raquo;",  6, 187 },
    { "reg;",    4, 174 },
    { "sect;",   5, 167 },
    { "shy;",    4, 173 },
    { "sup1;",   5, 185 },
    { "sup2;",   5, 178 },
    { "sup3;",   5, 179 },
    { "szlig;",  6, 223 },
    { "THORN;",  6, 222 },
    { "thorn;",  6, 254 },
    { "times;",  6, 215 },
    { "Uacute;", 7, 218 },
    { "uacute;", 7, 250 },
    { "Ucirc;",  6, 219 },
    { "ucirc;",  6, 251 },
    { "Ugrave;", 7, 217 },
    { "ugrave;", 7, 249 },
    { "uml;",    4, 168 },
    { "Uuml;",   5, 220 },
    { "uuml;",   5, 252 },
    { "Yacute;", 7, 221 },
    { "yacute;", 7, 253 },
    { "yen;",    4, 165 },
    { "yuml;",   5, 255 }
  };

  if (!strchr(p, ';')) return -1;
  if (*p == '#') {
    if (*(p+1) == 'x' || *(p+1) == 'X') return strtol(p+2, NULL, 16);
    else return atoi(p+1);
  }
  for (i = (int)(sizeof(names) / sizeof(names[0])), nameptr = names; i > 0; i --, nameptr ++)
    if (strncmp(p, nameptr->name, nameptr->namelen) == 0)
      return nameptr->code;

  return -1;
}


//
// 'scrollbar_callback()' - A callback for the scrollbar.
//

static void
scrollbar_callback(Widget *s, void *)
{
  ((HelpView *)(s->parent()))->topline(int(((Scrollbar*)s)->value()));
}


//
// 'hscrollbar_callback()' - A callback for the horizontal scrollbar.
//

static void
hscrollbar_callback(Widget *s, void *)
{
  ((HelpView *)(s->parent()))->leftline(int(((Scrollbar*)s)->value()));
}

//
// End of "$Id: HelpView.cxx 8799 2011-06-10 06:37:28Z bgbnbigben $".
//
