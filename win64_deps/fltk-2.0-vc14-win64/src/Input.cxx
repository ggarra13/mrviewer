//
// "$Id: Input.cxx 8751 2011-05-28 13:16:09Z bgbnbigben $"
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

#include <fltk/Input.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/math.h>
#include <fltk/utf.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

using namespace fltk;

extern void fl_set_spot(fltk::Font *f, Widget *w, int x, int y);

extern Widget* fl_pending_callback;

// Called by any changes to the text, this correctly triggers callbacks:
static void changed_stuff(Input* i) {
  i->set_changed();
  if (i->when() & WHEN_CHANGED) {
    i->do_callback();
  } else if (i->when() & (WHEN_RELEASE|WHEN_ENTER_KEY)) {
    Widget* w = fl_pending_callback;
    if (i == w) return;
    if (w) {fl_pending_callback = 0; w->do_callback();}
    fl_pending_callback = i;
  }
}

////////////////////////////////////////////////////////////////

/*! \class fltk::Input

  This is the FLTK text input widget. It displays a single line of
  text and lets the user edit it. The text may contain any bytes
  (even '\\0'). The bytes 0..31 are displayed in ^X notation, the
  rest are interpreted as UTF-8 (see fltk::utf8decode()).

  The default when() is WHEN_RELEASE. This is fine for a popup
  control panel where nothing happens until the panel is closed.
  But for most other uses of the input field you want to change
  it. Useful values are:

  - fltk::WHEN_NEVER: The callback is not done, but changed() is turned on.
  - fltk::WHEN_CHANGED: The callback is done each time the text is
    changed by the user.
  - fltk::WHEN_ENTER_KEY: Hitting the enter key after changing the
    text will cause the callback. 
  - fltk::WHEN_ENTER_KEY_ALWAYS: The Enter key will do the callback
    even if the text has not changed. Useful for command fields.
    Also you need to do this if you want both the enter key and
    either fltk::WHEN_CHANGED or fltk::WHEN_RELEASE, in this case you 
	can tell if Enter was typed by testing fltk::event_key()==fltk::ReturnKey.
  - fltk::WHEN_RELEASE: Deprecated. The callback is done if the
    text has changed and the user clicks on another widget or the
    focus moves (which can happen due to the window this widget
    is on being closed).

  If you wish to restrict the text the user can type (such as limiting
  it to numbers, a particular length, etc), you should subclass this
  and override the replace() function with a version that rejects
  changes you don't want to allow.

  If you don't like the keybindings you can override handle() to
  change them.

  All arguments that are lengths or offsets into the strings are in
  bytes, not the UTF-8 characters they represent.
*/

#define MAXBUF 1024

/*! Copy string p to the buffer, replacing control characters with ^X.
  Stop at then end of the line, or truncate if necessary so the resulting
  string and null terminator fits in a buffer of size MAXBUF.  Returns
  pointer to the character after the last one copied. */
const char* Input::expand(const char* p, char* buf,int wordwrap) const {
  char* o = buf;
  char* e = buf+(MAXBUF-4);
  const char* lastspace = p;
  char* lastspace_out = o;
  int width_to_lastspace = 0;
  int word_count = 0;

  if (type() == SECRET) {
    while (o<e && p < text_+size_) {*o++ = '*'; p++;}
  } else while (o<e) {
    if (wordwrap && (p >= text_+size_ || isspace(*p))) {
      width_to_lastspace += (int)getwidth(lastspace_out, o-lastspace_out);
      if (p > lastspace+1) {
	if (word_count && width_to_lastspace > wordwrap) {
	  p = lastspace; o = lastspace_out; break;
	}
	word_count++;
      }
      lastspace = p;
      lastspace_out = o;
    }
    if (p >= text_+size_) break;
    int c = *p++;
    if (c & 0xE0) {
      *o++ = c;
    } else {
      if (c == '\n' && type() >= MULTILINE) {p--; break;}
      if (c == '\t' && type() >= MULTILINE) {
	for (c = (o-buf)%8; c<8 && o<e; c++) *o++ = ' ';
      } else {
	*o++ = '^';
	*o++ = c ^ 0x40;
      }
    }
  }
  *o = 0;
  return p;
}

/*! After filling in a buffer with expand(), find the width to e. */
float Input::expandpos(
  const char* p,	// real string
  const char* e,	// pointer into real string
  const char* buf,	// conversion of real string by expand()
  int* returnn		// return offset into buf here
) const {
  int n = 0;
  if (type() == SECRET) n = e-p;
  else while (p<e) {
    int c = *p++;
    if (c & 0xE0) {
      n++;
    } else {
      if (c == '\t' && type() >= MULTILINE) n += 8-(n%8);
      else n += 2;
    }
  }
  if (returnn) *returnn = n;
  return getwidth(buf, n);
}

////////////////////////////////////////////////////////////////
// minimal update:

// Characters from mu_p to end of widget are redrawn.
// If this == erase_cursor_only, small part at mu_p is redrawn.
static Input* erase_cursor_only;

/*! Indicate that the characters after position \a p must be redrawn.
  This is done with a clip region to work with xft. The purpose is to
  avoid blinking, not to make draw() less expensive. A few minor attempts
  to make draw() not think about clipped text are implemented, though.
*/

void Input::minimal_update(int p) {
  if (erase_cursor_only == this) erase_cursor_only = 0;
  // Sometimes DAMAGE_ALL is set, but widget not redrawed because it's out of clip region.
  // So widget remains undrawed before next full redraw (window resize)
  //if (damage() & DAMAGE_ALL) return; // don't waste time if it won't be done
  if (damage() & DAMAGE_VALUE) {
    if (p < mu_p) mu_p = p;
  } else {
    mu_p = p;
  }
  redraw(DAMAGE_VALUE);
}

/*! Indicate that the characters between positions \a p and \a q must be
  redrawn. Currently this just does minimal_update(min(p,q)). */
void Input::minimal_update(int p, int q) {
  if (q < p) p = q;
  minimal_update(p);
}

/*! Indicate that a cursor drawn a postion \a p must be redrawn. */
void Input::erase_cursor_at(int p) {
  if (!damage()) {
    mu_p = p;
    redraw(DAMAGE_VALUE);
    erase_cursor_only = this;
  } else {
    minimal_update(p);
  }
}

////////////////////////////////////////////////////////////////

static float up_down_pos;
static bool was_up_down;
static Input* dnd_target;
static int dnd_target_position;

void Input::setfont() const {
  fltk::setfont(textfont(), textsize());
}

static int line_spacing(float leading) {
  return int(getsize() + leading + .5);
}

static float line_ascent(float leading) {
  return (line_spacing(leading) + getascent() - getdescent() - 1) / 2;
}

/*! The default version draws the box and calls draw(x,y,w,h) with
  the area inside the box. It also draws any inside labels at the
  left of the text.
*/
void Input::draw() {
  Rectangle r(w(),h());
  box()->inset(r);
  if (damage() & DAMAGE_ALL) {
    draw_frame();
    // draw and measure the inside label:
    if (label() && label()[0] && (!(flags()&15)||(flags()&ALIGN_INSIDE))) {
      const float leading = this->leading();
      int height = line_spacing(leading);
      float desc = line_ascent(leading);
      fltk::setfont(labelfont(), labelsize());
      float width = getwidth(label());
      label_width = int(width+getwidth(":")+2.5);
      Color color = this->color();
      setcolor(color);
      Rectangle lr(r); lr.w(label_width);
      fillrect(lr);
      if (flag(INACTIVE_R)) setcolor(inactive(labelcolor(), color));
      else setcolor(labelcolor());
      float y = r.y()+((r.h()-height)>>1)+desc;
      drawtext(label(), float(r.x()+2), y);
      drawtext(":", r.x()+2+width, y);
    } else {
      label_width = 0;
    }
  }
  r.move_x(label_width);
  draw(r);
}

/*!
  Draw the text, where the passed rectangle is the area the text is to be
  drawn into. This method is provided so a subclass can place the text
  into a subrectangle.

  If damage()&fltk::DAMAGE_ALL is true, this assummes the area has
  already been erased to color(). Otherwise it does minimal update and
  erases the area itself.
*/
void Input::draw(const Rectangle& r)
{

  // ignore HIGHLIGHT so it does not change color when mouse points at it:
  drawstyle(style(), this->flags()&~HIGHLIGHT);

  const Color background = getbgcolor();
  const Color textcolor = getcolor();
  const float leading = this->leading();
  int height = line_spacing(leading);
  float desc = line_ascent(leading);

  bool erase_cursor_only =
    this == ::erase_cursor_only &&
    !(damage() & (DAMAGE_ALL|DAMAGE_EXPOSE));

  // handle a totally blank one quickly:
  if (!size() && !focused() && this != dnd_target) {
    setcolor(background);
    fillrect(r);
    return;
  }

  int selstart, selend;
  if (!focused() && !pushed())
    selstart = selend = 0;
  else if (position() <= mark()) {
    selstart = position(); selend = mark();
  } else {
    selend = position(); selstart = mark();
  }

  int wordwrap = (type() > MULTILINE) ? r.w()-8 : 0;

  const char *p, *e;
  char buf[MAXBUF];

  // count how many lines and put the last one into the buffer:
  // And figure out where the cursor is:
  int lines;
  int curx, cury;
  int cursor_position = (this==dnd_target) ? dnd_target_position : position();
  for (p=text_, curx=cury=lines=0; ;) {
    e = expand(p, buf, wordwrap);
    if (cursor_position >= p-text_ && cursor_position <= e-text_) {
      curx = int(expandpos(p, text_+cursor_position, buf, 0)+.5);
      if (focused() && !was_up_down) up_down_pos = float(curx);
      cury = lines*height;
      int newscroll = xscroll_;
      if (curx > newscroll+r.w()-20) {
	// figure out scrolling so there is space after the cursor:
	newscroll = curx+20-r.w();
	// figure out the furthest left we ever want to scroll:
	int ex = int(expandpos(p, e, buf, 0))-r.w()+8;
	// use minimum of both amounts:
	if (ex < newscroll) newscroll = ex;
      } else if (curx < newscroll+20) {
	newscroll = curx-20;
      }
      if (newscroll < 0) newscroll = 0;
      if (newscroll != xscroll_) {
	xscroll_ = newscroll;
	mu_p = 0;
	erase_cursor_only = false;
      }
    }
    lines++;
    if (e >= text_+size_) break;
    if (*e == '\n' || *e == ' ') e++;
    p = e;
  }

  // adjust the scrolling:
  if (type() >= MULTILINE) {
    int newy = yscroll_;
    if (cury < newy) newy = cury;
    if (cury > newy+r.h()-height) newy = cury-r.h()+height;
    if (newy < -1) newy = -1;
    if (newy != yscroll_) {
      yscroll_ = newy;
      mu_p = 0;
      erase_cursor_only = false;
    }
  } else {
    yscroll_ = -((r.h()-height)>>1);
  }

  // if we are not doing minimal update a single erase is done,
  // rather than one per line:
  if (damage() & DAMAGE_ALL) {
    setcolor(background);
    fillrect(r);
  }

  push_clip(r);

  // I leave a small border on the edge which shrinks as the input field
  // gets smaller than 12:
  int xpos = r.w()-9; if (xpos > 3) xpos = 3; else if (xpos < 1) xpos = 1;
  xpos += r.x()-xscroll_;

  int ypos = -yscroll_;

  // visit each line and draw it:
  p = text_;
  int spot_x = r.x();
  int spot_y = r.y();
  for (; ypos < r.h();) {

    // re-expand line unless it is the last one calculated above:
    if (lines>1) e = expand(p, buf, wordwrap);

    if (ypos <= -height) goto CONTINUE; // clipped off top

    if (!(damage()&DAMAGE_ALL)) {	// for minimal update:
      const char* pp = text_+mu_p; // pointer to where minimal update starts
      if (e < pp) goto CONTINUE2; // this line is before the changes
      if (erase_cursor_only && p > pp) goto CONTINUE2; // this line is after
      // calculate area to erase:
      Rectangle er(r.x(), r.y()+ypos, r.w(), height);
      if (p >= pp) {
	if (erase_cursor_only) er.set_r(xpos+1);
      } else {
	er.set_x(xpos+int(expandpos(p, pp, buf, 0))-1);
	if (erase_cursor_only) er.w(2);
      }
      // clip to and erase it:
      setcolor(background);
      fillrect(er);
      push_clip(er);
      // it now draws entire line over it
      // this should not draw letters to left of erased area, but
      // that is nyi.
    }

    // Draw selection area if required:
    if (selstart < selend && selstart <= e-text_ && selend > p-text_) {
      const char* pp = text_+selstart;
      // draw unselected text before the selection:
      float x1 = float(xpos);
      int offset1 = 0;
      if (pp > p) {
	setcolor(textcolor);
	x1 += expandpos(p, pp, buf, &offset1);
	drawtext(buf, offset1, float(xpos), float(r.y()+ypos+desc));
      }
      // draw selected text for this line:
      pp = text_+selend;
      float x2 = float(r.r());
      int offset2;
      if (pp <= e) x2 = xpos+expandpos(p, pp, buf, &offset2);
      else offset2 = strlen(buf);
      Color bg = selection_color();
      //if (!focused()) bg = lerp(color(), bg, 0.5f);
      Color fg = contrast(selection_textcolor(), bg);
      setcolor(bg);
      int xx=int(x1);
      fillrect(xx, r.y()+ypos, int(x2+.5)-xx, height);
      setcolor(fg);
      drawtext(buf+offset1, offset2-offset1, x1, r.y()+ypos+desc);
      // draw unselected text after the selection:
      if (pp < e) {
	setcolor(textcolor);
	drawtext(buf+offset2, x2, r.y()+ypos+desc);
      }
    } else {
      // draw unselected text:
      setcolor(textcolor);
      drawtext(buf, float(xpos), float(r.y()+ypos+desc));
    }

    if (!(damage()&DAMAGE_ALL)) pop_clip();

  CONTINUE2:
    // draw the cursor:
    if ((this==dnd_target || (focused() && selstart == selend)) &&
	cursor_position >= p-text_ && cursor_position <= e-text_) {
      setcolor(textcolor);
      fillrect(xpos+curx-1, r.y()+ypos, 2, height);
      spot_x = xpos+curx;
      spot_y = r.y()+ypos;
    }

  CONTINUE:
    ypos += height;
    if (e >= text_+size_) break;
    if (*e == '\n' || *e == ' ') e++;
    p = e;
  }

  // for minimal update, erase all lines below last one if necessary:
  if (!(damage()&DAMAGE_ALL) && type() >= MULTILINE && ypos<r.h()
      && (!erase_cursor_only || p <= text_+mu_p)) {
    if (ypos < 0) ypos = 0;
    setcolor(background);
    Rectangle er(r); er.move_y(ypos);
    fillrect(er);
  }

  pop_clip();
  if (focused()) {
    transform(spot_x, spot_y);
    fl_set_spot(textfont(), this, spot_x, spot_y);
  }
}

static int isword(char c) {
  return (c&128 || isalnum(c) || strchr("#%&-/@\\_~", c));
}

/*! Returns the location of the next word boundary at or after position. */
int Input::word_end(int i) const {
  if (type() == SECRET) return size();
  while (!i || !isword(at(i-1))) i++;
  while (i < size() && isword(at(i))) i++;
  return i;
}

/*! Returns the location of the first word boundary at or before position. */
int Input::word_start(int i) const {
  if (type() == SECRET) return 0;
  while (!isword(at(i))) i--;
  while (i > 0 && isword(at(i-1))) i--;
  return i;
}

/*! Returns the location of the next newline or wordwrap space at or
  after position. */
int Input::line_end(int i) const {
  if (type() >= WORDWRAP) {
    // go to the start of the paragraph:
    int j = i;
    while (j > 0 && at(j-1) != '\n') j--;
    // now measure lines until we get past i, end of that line is real eol:
    Rectangle r(w(),h()); box()->inset(r); int wordwrap = r.w()-6;
    setfont();
    for (const char* p=text_+j; ;) {
      char buf[MAXBUF];
      p = expand(p, buf, wordwrap);
      if (p-text_ >= i) return p-text_;
      p++;
    }
  } else if (type() >= MULTILINE) {
    while (i < size() && at(i) != '\n') i++;
    return i;
  } else {
    return size();
  }
}

/*! Returns the location of the start of the line containing the position. */
int Input::line_start(int i) const {
  if (type() < MULTILINE) return 0;
  int j = i;
  while (j > 0 && at(j-1) != '\n') j--;
  if (type() >= WORDWRAP) {
    // now measure lines until we get past i, start of that line is real eol:
    Rectangle r(w(),h()); box()->inset(r); int wordwrap = r.w()-6;
    setfont();
    for (const char* p=text_+j; ;) {
      char buf[MAXBUF];
      const char* e = expand(p, buf, wordwrap);
      if (e-text_ >= i) return p-text_;
      p = e+1;
    }
  }
  return j;
}

/*! Figure out what character the most recent mouse event would be pointing
  at, assumming you drew the text by calling draw() with the same rectangle.
  Returns 0 if the mouse is before the first character, and size() if it is
  after the last one.
*/
int Input::mouse_position(const Rectangle& r) const
{
  if (!size()) return 0;

  setfont();

  // figure out what line we are pointing at:
  int theline = 0;
  if (type() >= MULTILINE) {
    theline = event_y()-r.y()+yscroll_;
    if (theline < 0) return 0;
    // CET - FIXME - this widget should keep track of the line heights
    // internally.  Using the style accessor functions is not guaranteed
    // to give correct results for what is actually displayed!  They
    // should _only_ be used in layout() and draw().
    // WAS: I think this is ok. Calls in response to user events can
    // assumme layout() has been called recently.
    theline /= line_spacing(leading());
  }

  int wordwrap = (type() > MULTILINE) ? r.w()-8 : 0;

  // Step through all the lines until we reach the pointed-to line.
  // Expand the lines to printed representation into the buffer:
  const char *p, *e;
  char buf[MAXBUF];
  for (p=text_;; ) {
    e = expand(p, buf, wordwrap);
    theline--; if (theline < 0) break;
    if (*e == '\n' || *e == ' ') e++;
    p = e;
    if (e >= text_+size_) break;
  }

  // Do a binary search for the character that starts before this position:
  int xpos = r.x()-xscroll_; if (r.w() > 12) xpos += 3;
  const char *a, *b; float f0 = float(event_x()-xpos);
  for (a = p, b = e; a<b; ) {
    const char* t = a+(b-a+1)/2;
    if (t < b) t = utf8fwd(t, a, b);
    int f = xpos+int(expandpos(p, t, buf, 0)+.5);
    if (f <= event_x()) {a = t; f0 = float(event_x()-f); }
    else b = utf8back(t-1,a,b);
  }
  // see if closer to character on the right:
  if (a < e) {
    const char* t = utf8fwd(a+1, p, e);
    int f1 = xpos+int(expandpos(p, t, buf, 0)+.5)-event_x();
    if (f1 < f0) a = t;
  }
  return a-text_;
}

/*! \fn void Input::position(int n)
  Same as position(n,n)
*/

/*! \fn void Input::mark(int n)
  Same as position(position(),n)
*/

/*!
  The input widget maintains two pointers into the string. The
  "position" is where the cursor is. The "mark" is the other end of
  the selected text. If they are equal then there is no
  selection. Changing this does not affect the X selection, call
  copy() if you want that.

  Changing these values causes a redraw(). The new values are bounds
  checked and limited to the size of the string.

  It is up to the caller to make sure the position and mark are
  at the borders of UTF-8 characters!
*/
void Input::position(int new_position, int new_mark) {
  was_up_down = false;
  if (new_position<0) new_position = 0;
  if (new_position>size()) new_position = size();
  if (new_mark<0) new_mark = 0;
  if (new_mark>size()) new_mark = size();

  if (new_position == position_ && new_mark == mark_) return;
  if (new_position != new_mark) {
    // new position is a selection:
    if (new_position != position_) minimal_update(position_, new_position);
    if (new_mark != mark_) minimal_update(mark_, new_mark);
  } else {
    // new position is a cursor:
    if (position_ == mark_) erase_cursor_at(position_);
    else minimal_update(position_, mark_);
  }
  position_ = new_position;
  mark_ = new_mark;
}

/*! Do the correct thing for arrow keys. \a i must be the location of the
  start of a line. Sets the position (and mark if \a keepmark is false) to
  somewhere after \a i, such that pressing the arrows repeatedly will
  cause the point to move up and down. */
void Input::up_down_position(int i, bool keepmark) {
  // cursor must already be at start of line!
  setfont();
  int wordwrap;
  if (type() > MULTILINE) {
    Rectangle r(w(),h()); box()->inset(r); wordwrap = r.w()-6;
  } else {
    wordwrap = 0;
  }
  char buf[MAXBUF];
  const char* p = text_+i;
  const char* e = expand(p, buf, wordwrap);
  const char *l, *r, *t;
  for (l = p, r = e; l<r; ) {
    t = l+(r-l+1)/2;
    if (t < r) t = utf8fwd(t, l, r);
    int f = (int)expandpos(p, t, buf, 0);
    if (f <= up_down_pos) l = t;
    else r = utf8back(t-1,l,r);
  }
  int j = l-text_;
  position(j, keepmark ? mark_ : j);
  was_up_down = true;
}

/*!
  Put the current selection between mark() and position() into the
  selection or clipboard by calling fltk::copy(). If position() and
  mark() are equal this does nothing (ie it does not clear the clipboard).

  If \a clipboard is true the text is put into the user-visible cut &
  paste clipboard (this is probably what you want). If \a clipboard is
  false it is put into the less-visible selection buffer that is used
  to do middle-mouse paste and drag & drop.

  To paste the clipboard, call fltk::paste(true) and fltk will send
  the widget a fltk::PASTE event with the text, which will cause it to
  be inserted.
*/
bool Input::copy(bool clipboard) {
  int b = position();
  int e = mark();
  if (b != e) {
    if (b > e) {b = mark(); e = position();}
    if (type() == SECRET) e = b;
    fltk::copy(text_+b, e-b, clipboard);
    return true;
  }
  return false;
}

#define MAXFLOATSIZE 40

static char* undobuffer;
static int undobufferlength;
static Input* undowidget;
static bool undo_is_redo;
static int undoat;	// points after insertion
static int undocut;	// number of characters deleted there
static int undoinsert;	// number of characters inserted
static int yankcut;	// length of valid contents of buffer, even if undocut=0

static void undobuffersize(int n) {
  if (n > undobufferlength) {
    if (undobuffer) {
      int m = 2*undobufferlength;
      while (m < n) m *= 2;
      char* newbuffer = new char[m];
      memcpy(newbuffer, undobuffer, undobufferlength);
      delete[] undobuffer;
      undobuffer = newbuffer;
      undobufferlength = m;
    } else {
      undobufferlength = n+9;
      undobuffer = new char[undobufferlength];
    }
  }
}

/*!
  This call does all editing of the text. It deletes the region
  between \a b and \a e (either one may be less or equal to the other), and
  then inserts \a ilen (which may be zero) characters from the string
  \a text at that point and leaves the mark() and position() after the
  insertion. If the text is changed the callback is done if the when()
  flags indicate it should be done.

  Subclasses of Input can override this method to control what
  characters can be inserted into the text. A typical implementation
  will check the characters in the insertion for legality and then
  call Input::replace() only if they are all ok.

  \a b and \a e are bounds checked so don't worry about sending values
  outside the length of the string.

  Subclasses should return true if the keystroke that produced this
  call should be "eaten". If false is returned the keystroke is
  allowed to be tested as a shortcut for other widgets. In our
  experience it is best to return true even if you don't make
  changes. The base class version returns true always.
*/
bool Input::replace(int b, int e, const char* text, int ilen) {

  was_up_down = false;

  if (b<0) b = 0;
  if (e<0) e = 0;
  if (b>size_) b = size_;
  if (e>size_) e = size_;
  if (e<b) {int t=b; b=e; e=t;}

  // Submitted utf-8 patch tried to fix b,e to point at legal utf-8
  // characters. I removed this as I feel this code should be called
  // with literal bytes and the caller must make sure the pointers
  // are correct:
#if 0
  // this can be done by a subclass!
  if (size_+ilen-(e-b) > maximum_size_) {
    ilen = maximum_size_-size_+(e-b);
    if (ilen < 0) ilen = 0;
  }
#endif

  reserve(size_+ilen);

  if (undo_is_redo) {undowidget = 0; undo_is_redo = false;}

  if (e>b) {
    if (undowidget == this && b == undoat) {
      undobuffersize(undocut+(e-b));
      memcpy(undobuffer+undocut, text_+b, e-b);
      undocut += e-b;
    } else if (undowidget == this && e == undoat && !undoinsert) {
      undobuffersize(undocut+(e-b));
      memmove(undobuffer+(e-b), undobuffer, undocut);
      memcpy(undobuffer, text_+b, e-b);
      undocut += e-b;
    } else if (undowidget == this && e == undoat && (e-b)<undoinsert) {
      undoinsert -= e-b;
    } else {
      undobuffersize(e-b);
      memcpy(undobuffer, text_+b, e-b);
      undocut = e-b;
      undoinsert = 0;
    }
    memmove(buffer+b, buffer+e, size_-e+1);
    size_ -= e-b;
    undowidget = this;
    undoat = b;
    if (type() == SECRET) yankcut = 0; else yankcut = undocut;
  }

  if (ilen) {
    if (undowidget == this && b == undoat)
      undoinsert += ilen;
    else {
      undocut = 0;
      undoinsert = ilen;
    }
    memmove(buffer+b+ilen, buffer+b, size_-b+1);
    memcpy(buffer+b, text, ilen);
    size_ += ilen;
  }
  undowidget = this;
  undoat = b+ilen;

  // When editing with word wrap, it is possible to effectively turn
  // the space before the current word into a newline or back, so the
  // minimal update pointer must point at it.  This will
  // result in sub-optimal update when such wrapping does not happen
  // but it is too hard to figure out for now...
  if (type() > MULTILINE) {
    int c = b - 1;
    while (c > 0 && !isspace(at(c))) c--;
    if (c > 0) b = c;
  }

  // make sure we redraw the old selection or cursor:
  if (mark_ < b) b = mark_;
  if (position_ < b) b = position_;

  minimal_update(b);

  mark_ = position_ = undoat;

  changed_stuff(this);
  return true;
}

/*! \fn bool Input::cut()
  Wrapper around replace(), this deletes the region between the point
  and the mark. It does nothing if they are equal.
*/

/*! \fn bool Input::cut(int n)
  Wrapper around replace() this deletes up to \a n characters after the
  point, or before the point if \a n is negative. N is bounds checked.
*/

/*! \fn bool Input::cut(int a, int b)
  Wrapper around replace() this deletes the characters between \a a and
  \a b. The values are clamped to the ends of the string, and \a b can be
  less than \a a.
*/

/*!
  Wrapper around replace(). This inserts the string at the point and
  leaves the point after it.
*/
bool Input::insert(const char* text) {
  if (!text) return 0;
  return insert(text, strlen(text));
}

/*! \fn bool Input::insert(const char* text, int n)
  Wrapper around replace(). This inserts \a n characters from the
  \a text (including '\\0' characters!) at the point and
  leaves the point after it.
*/

/*! \fn bool Input::replace(int a, int b, char c)
  Wrapper around replace(). Deletes the characters between \a a and \a b
  and then inserts the single byte \a c. */

/*! If this is the most recent widget on which replace() was done on,
  this will undo that replace() and probably several others (ie if the
  user has typed a lot of text it will undo all of it even though that
  was probably many calls to replace()). Returns true if any change
  was made. */
bool Input::undo() {
  if (undowidget != this || !(undocut || undoinsert)) return false;
  was_up_down = false;

  int ilen = undocut;
  int xlen = undoinsert;
  int b = undoat-xlen;
  int b1 = b;

  reserve(size_+ilen);

  if (ilen) {
    memmove(buffer+b+ilen, buffer+b, size_-b+1);
    memcpy(buffer+b, undobuffer, ilen);
    size_ += ilen;
    b += ilen;
  }

  if (xlen) {
    undobuffersize(xlen);
    memcpy(undobuffer, buffer+b, xlen);
    memmove(buffer+b, buffer+b+xlen, size_-xlen-b+1);
    size_ -= xlen;
  }

  undocut = xlen;
  if (xlen) yankcut = xlen;
  undoinsert = ilen;
  undoat = b;
  mark_ = b /* -ilen */;
  position_ = b;
  undo_is_redo = !undo_is_redo;

  minimal_update(b1);
  changed_stuff(this);
  return true;
}

#if 0
/*! Does Emacs-style yank of appended cuts by retrieving them from
  the saved undo() information. */
bool Input::yank() {
  return yankcut && change(position(), position(), undobuffer, yankcut);
}
#endif

////////////////////////////////////////////////////////////////

static void revert(Style *s) {
  //s->leading = 2; // this is the default now
}
static NamedStyle style("Input", revert, &Input::default_style);
NamedStyle* Input::default_style = &::style;

/*! The constructor initializes:
  - label alignment to ALIGN_LEFT (puts label to left)
  - value to ""
*/
Input::Input(int x, int y, int w, int h, const char* l)
  : Widget(x, y, w, h, l)
{
  clear_flag(ALIGN_MASK);
  set_flag(ALIGN_LEFT);
  set_click_to_focus();
  mark_ = position_ = size_ = 0;
  bufsize = 0;
  buffer  = 0;
  text_ = "";
  xscroll_ = yscroll_ = 0;
  style(default_style);
  label_width = 0;
}

/*! \fn const char* Input::text() const
  The current string, as edited by the user. There is an extra NUL
  character at the end, so this returned pointer can be used directly
  by C functions. size() returns how many bytes are in the string,
  which is useful if you want to look at embedded NUL characters.

  Pointer is to an internal buffer and is only value until the next
  event is handled by the Input widget.
*/

/*! \fn char Input::at(int i) const
  Same as text()[n], but may be faster in plausible
  implementations. No bounds checking is done.
*/

/*! \fn int Input::size() const
  Returns the number of characters in text(). This may be greater
  than strlen(text()) if there are NUL characters in it.
*/

/*! Reserve the interal private buffer of at least \a len bytes, even
  if the current text() is not that long. Can be used to avoid unnecessary
  memory reallocations if you know you will be replacing the text() with
  a longer one later.
*/
void Input::reserve(int len) {
  if (!bufsize) {
    bufsize = (len > size_) ? len+9 : size_+1;
    buffer = new char[bufsize];
    memcpy(buffer, text_, size_);
  } else if (bufsize <= len) {
    int newsize = (len > size_) ? len*2 : size_+1;
    // we may need to move old value in case it points into buffer:
    if (text_ >= buffer && text_ < buffer+bufsize) {
      char* nbuffer = new char[newsize];
      memcpy(nbuffer, text_, size_);
      delete[] buffer;
      buffer = nbuffer;
    } else {
      delete[] buffer;
      buffer = new char[newsize];
      memcpy(buffer, text_, size_);
    }
    bufsize = newsize;
  } else if (text_ != buffer) {
    memmove(buffer, text_, size_);
  }
  buffer[size_] = 0;
  text_ = buffer;
}

/*!
  Same as text(str,len), except it does not copy the string, instead
  it makes text() return a pointer to \a str (unless \a len is 0, in
  which case it makes it point to a zero-length string).

  \a str must point to static memory that will not be altered until at
  least the Input widget is destroyed or the text() is changed
  again. If the user attempts to edit the string it is then copied to
  the internal buffer and the editing done there. This can save a lot
  of time and memory if your program is changing the string to various
  constants a lot but the user rarely edits it.
*/
bool Input::static_text(const char* str, int len) {
  if (fl_pending_callback == this) fl_pending_callback = 0;
  clear_changed();
  if (undowidget == this) undowidget = 0;
  bool ret = true;
  if (str == text_ && len == size_) {
    ret = false;
  } else if (len) { // non-empty new value:
    if (xscroll_ || (type() >= MULTILINE && yscroll_)) {
      xscroll_ = yscroll_ = 0;
      minimal_update(0);
    }
    int i = 0;
    // find first different character:
    if (text_) {
      for (; i<size_ && i<len; ){
	 int length, dummy;
	 int strBytes = utf8bytes(str[i]), textBytes = utf8bytes(text_[i]);
	 if(utf8decode(str+i, str+i+strBytes, &length) == utf8decode(text_+i, text_+i+textBytes, &dummy))
	   i += length;
	 else break;
      }
      if (i==size_ && i==len) ret = false;
    }
    if (ret) minimal_update(i);
    text_ = str;
    size_ = len;
  } else { // empty new value:
    if (!size_) return false; // both old and new are empty.
    size_ = 0;
    text_ = "";
    xscroll_ = yscroll_ = 0;
    minimal_update(0);
  }
  position(0, size());
  return ret;
}

/*! Same as static_text(str, str ? strlen(str) : 0) */
bool Input::static_text(const char* str) {
  return static_text(str, str ? strlen(str) : 0);
}

/*!
  Change the text() to return the first \a len bytes of \a str and
  size() to return \a len, and set the position() to \a len and the
  mark() to zero (thus highlighting the entire value).

  Returns true if the bytes in the new string are different than the
  old string.
*/
bool Input::text(const char* str, int len) {
  bool ret = static_text(str, len);
  if (len) reserve(len);
  return ret;
}

/*! Same as text(str, str ? strlen(str) : 0) */
bool Input::text(const char* str) {
  return text(str, str ? strlen(str) : 0);
}

/*! The destructor destroys the memory used by text() */
Input::~Input() {
  if (fl_pending_callback == this) fl_pending_callback = 0;
  if (undowidget == this) undowidget = 0;
  delete[] buffer;
}

////////////////////////////////////////////////////////////////

/*! Move the point, move the mark only if shift is not held down. */
void Input::shift_position(int p) {
  position(p, event_state(SHIFT) ? mark() : p);
}

/*! Move the point, move the mark only if shift is not held down. */
void Input::shift_up_down_position(int p) {
  up_down_position(p, event_state(SHIFT));
}

/*! Handle KEY events. The handle() method calls this. This provides
  an Emacs and Windows style of editing. Most Emacs commands are first
  run through fltk::try_shortcut() to test if they are menu items for
  the program.

  - Shift: do not move the mark when moving the point
  - LeftKey, Ctrl+B: move left one character
  - Ctrl+LeftKey, Alt+B: move left one word
  - RightKey, Ctrl+F: move right one character
  - Ctrl+RightKey, Alt+F: move right one word
  - Ctrl+A: go to start of line, if already there select all text
  - HomeKey: go to start of line
  - EndKey, Ctrl+E: go to end of line
  - Ctrl+Insert: copy
  - Shift+Insert: paste
  - Shift+Delete: cut
  - Delete, Ctrl+D: delete region or one character
  - Ctrl+Delete, Alt+D: delete region or one word
  - BackSpace, Ctrl+H: delete region or left one character
  - Ctrl+BackSpace, Alt+H: delete region or left one word
  - Return, KeypadEnter: if when()&WHEN_ENTER_KEY, and no shift keys held
    down, this selects all and does the callback. Otherwise key is ignored.
  - Ctrl+K: cuts from the position to the end of line
  - Ctrl+C: copy
  - Ctrl+T: swap the two characters around point. If point is at end swap the
    last two characters.
  - Ctrl+U: delete all the text
  - Ctrl+V: paste
  - Ctrl+X, Ctrl+W: cut
  - Ctrl+Y: redo
  - Ctrl+Z, Ctrl+/: undo
  - All printing characters are run through fltk::compose() and the result
    used to insert text.

  For the (obsolete) MULTILINE mode you can also do these:

  - UpKey, Ctrl+P: move up one line (MULTILINE only)
  - DownKey, Ctrl+N: move down one line (MULTILINE only)
  - PageUpKey: move up 1 line less than the vertical widget size
  - PageDownKey: move down 1 line less than the vertical widget size
  - Ctrl+HomeKey, Alt+A: got to start of text
  - Ctrl+EndKey, Alt+E: go to end of text
  - Return, KeypadEnter: inserts a newline
  - Ctrl+O: insert a newline and leave the cursor before it.

*/
bool Input::handle_key() {

  int i;

  // insert any text:
  if (compose(i)) {
    if (!i && !event_length()) return 1;
    return replace(position(), i ? position()-i : mark(),
		   event_text(), event_length());
  }

  bool command = event_state(COMMAND);
  bool ctrl = event_state(CTRL);
  bool shift = event_state(SHIFT);
  bool alt = event_state(OPTION);

  switch (event_key()) {

  case 'b':
    if (try_shortcut()) return true;
    ctrl = alt;
  case LeftKey:
    if (!shift && mark_<position_) i = mark_;
    else {
      i = position_-1;
      if (i > 0) i = utf8back(text_+i,text_,text_+size_)-text_;
    }
    shift_position(ctrl ? word_start(i) : i);
    return true;

  case 'f':
    if (try_shortcut()) return true;
    ctrl = alt;
  case RightKey:
    if (!shift && mark_>position_) i = mark_;
    else {
      i = position_+1;
      if (i < size_) i = utf8fwd(text_+i,text_,text_+size_)-text_;
    }
    shift_position(ctrl ? word_end(i) : i);
    return true;

  case 'p':
    if (try_shortcut()) return true;
    ctrl = alt;
  case UpKey:
    if (type() < MULTILINE) return false;
    i = position_; if (!shift && mark_<i) i = mark_;
    i = line_start(i);
    if (!i) shift_position(0);
    else up_down_position(line_start(i-1), shift);
    return true;

  case 'n':
    if (try_shortcut()) return true;
    ctrl = alt;
  case DownKey:
    if (type() < MULTILINE) return false;
    i = position_; if (!shift && mark_>i) i = mark_;
    i = line_end(i);
    if (i >= size()) shift_position(i);
    else up_down_position(i+1, shift);
    return true;

  case PageUpKey: {
    if (type() < MULTILINE) return false;
    i = line_start(position());
    setfont();
    for (int n = h()/line_spacing(leading()); n--;) i = line_start(i-1);
    shift_position(i);
    return true;}

  case PageDownKey: {
    if (type() < MULTILINE) return false;
    i = line_end(position());
    setfont();
    for (int n = h()/line_spacing(leading()); n--;) i = line_end(i)+1;
    shift_position(i+1);
    return true;}

  case 'a':
    if (command) {
      // if already at start of line, select the entire buffer. This
      // makes two ^A's do a select-all for both Emacs & Win32 compatability
      i = line_start(position());
      if (i == position() && i == mark()) position(0, size());
      else shift_position(i);
      return true;
    }
    if (try_shortcut()) return true;
    ctrl = alt;
  case HomeKey:
    shift_position(ctrl ? 0 : line_start(position()));
    return true;

  case 'e':
    if (try_shortcut()) return true;
    ctrl = alt;
  case EndKey:
    shift_position(ctrl ? size() : line_end(position()));
    return true;

  case InsertKey:
    if (ctrl) copy();
    else if (shift) paste(*this, true);
    else return false; // CUA toggles insert mode on/off
    return true;

  case 'd':
    if (try_shortcut()) return true;
    ctrl = alt;
  case DeleteKey:
    // I don't know what CUA does with ctrl+delete, I made it delete words
    if (shift) copy();
    if (mark() != position()) cut();
    else if (position() < size()) {
      i = utf8fwd(text_+position_+1,text_,text_+size_)-text_;
      cut(ctrl ? word_end(i)-position() : i-position());
    }
    return true;

  case 'h': // retro-Emacs, modern versions do "help"
    if (try_shortcut()) return true;
    ctrl = alt;
  case BackSpaceKey:
    // I don't know what CUA does with ctrl+backspace, I made it delete words
    if (mark() != position()) cut();
    else if (position()>0) {
      i = utf8back(text_+position_-1,text_,text_+size_)-text_;
      cut(ctrl ? word_start(i)-position() : i-position());
    }
    return true;

  case ReturnKey:
  case KeypadEnter:
    if (command || shift) return false;
    if (when() & WHEN_ENTER_KEY) {
      position(size(), 0);
      if (fl_pending_callback==this || (when()&WHEN_NOT_CHANGED)) {
	if (fl_pending_callback == this) fl_pending_callback = 0;
	do_callback();
      }
      return true;
    }
    if (type() >= MULTILINE)
      return replace(position(), mark(), '\n');
    return false;
  case TabKey:
    /*if (type() < MULTILINE || ctrl || shift)*/ return false;
    //return replace(position(), mark(), event_text(), 1);

  case 'k': // Emacs clear-to-end-of-line
    if (try_shortcut()) return true;
    // alt should clear to end of paragraph, nyi
    i = line_end(position());
    if (i == position() && i < size()) i++;
    if (cut(position(), i))
      // Make all the adjacent ^K's go into the clipboard, like Emacs:
      fltk::copy(undobuffer, yankcut, true);
    return true;

  case 'c':
    if (!command && try_shortcut()) return true;
    return copy();

  case 'o': // Emacs insert newline after cursor
    if (type() < MULTILINE || try_shortcut()) return true;
    if (replace(position(),mark(),"\n",1)) {
      position(position()-1);
      return 1;
    }
    return 0;

  case 't': // Emacs swap characters
    if (try_shortcut()) return true;
    if (size()<2) return 1;
    i = position();
    if (i <= 0 || text_[i-1]=='\n') i++;
    if (i >= size() || text_[i]=='\n') i--;
    // just punt on utf-8 characters:
    if ((text_[i-1]|text_[i])&0x80) return 1;
    {char t[2]; t[0] = text_[i]; t[1] = text_[i-1];
    if (!replace(i-1,i+1,t,2)) break;}
    position(i+1);
    return 1;

  case 'u': // Clear the field
    if (try_shortcut()) return true;
    position(size(), 0);
    return cut();

  case 'v':
    if (!command && try_shortcut()) return true;
    paste(*this,true);
    return true;

  case 'w': // Emacs cut
    if (try_shortcut()) return true;
    copy();
    return cut();

  case 'x':
    if (!command && try_shortcut()) return true;
    copy();
    return cut();

  case 'y':
    // Check for more global redo action first:
    if (try_shortcut()) return true;
    return undo_is_redo && undo();
#if 0
    // This is actually Emacs paste so do that if nothing else:
    paste(*this,true);
    return true;
#endif

  case '/': // Emacs undo
    if (!ctrl || try_shortcut()) return true;
  case 'z':
  case EscapeKey:
    // For undo we undo local typing first. Only if this fails do
    // we run some appliation menu item for undo:
    return !undo_is_redo && undo();

    // Other interesting Emacs characters:
    // 'q' quotes next character
    // 'r' backward incremental search
    // 's' incremental search
    // 'u' repeat count prefix

  case LeftShiftKey:
  case RightShiftKey:
  case LeftCtrlKey:
  case RightCtrlKey:
#if defined(__APPLE__)
  case LeftCmdKey:
  case RightCmdKey:
#endif
    // Don't pass these to other widgets. The user considers them
    // part of typing.
    return true;
  }

  // Insert any other keys (like ^J) into the text, if no shortcuts eat them:
  if (event_length()) {
    if (try_shortcut()) return true;
    return replace(position(), mark(), event_text(), event_length());
  }

  return false;
}

/*! Calls handle(x,y,w,h) with the area inside the box(). */
int Input::handle(int event) {
  Rectangle r(w(),h());
  box()->inset(r);
  r.move_x(label_width);
  return handle(event, r);
}

// Set this to 1 to get the ability to drag selected text out to other
// widgets. Qt and KDE does this. Other systems seem to support this only in
// multiline or rich-text type widgets.
#define DND_OUT 1

#if DND_OUT
static int drag_start;
#endif

/*!
  Default handler for all event types, where the passed rectangle is the
  area the text is to be drawn into. This method is provided so
  a subclass can place the text into a subrectangle.

  - Handles FOCUS, UNFOCUS
  - May do callback on HIDE
  - Any keystrokes call handle_key()
  - Handles PUSH, DRAG, RELEASE to select regions of text, move the
    cursor, and start drag & drop. Double click selects words, triple
    click selects lines (triple click is broken on Windows)
  - Receives drag&drop and accepts.
  - Handles PASTE events caused by accepting the drag&drop or by
    calling fltk::paste() (which handle_key() does for ^V)
*/
int Input::handle(int event, const Rectangle& r) {
  int newpos, newmark;

  switch (event) {

  case ENTER:
  case MOVE:
  case LEAVE:
    return 1; // For tooltips

  case FOCUS:
#if 0
    switch (event_key()) {
    case Right:
      position(0);
      break;
    case Left:
      position(size());
      break;
    case Down:
      up_down_position(0, 0);
      break;
    case Up:
      up_down_position(line_start(size()), 0);
      break;
    case Tab:
      position(size(),0);
      break;
    default:
      position(position(),mark());// turns off the saved up/down arrow position
      break;
    }
#else
    if (fltk::event() == KEY) {
      position(size(),0);
    } else {
      position(position(), mark());
    }
#endif
    // redraw the highlight area:
    if (mark_ != position_) minimal_update(mark_, position_);
    // else just make the cursor appear:
    else minimal_update(size()+1);
    return 2; // returns 2 to make Group think it really important

  case UNFOCUS:
    // disable input method
    fl_set_spot(NULL, this, 0, 0);
    // redraw the highlight area:
    if (mark_ != position_) minimal_update(mark_, position_);
    // else make the cursor disappear:
    else erase_cursor_at(position_);
    return 1;

  case HIDE:
    return 1;

  case KEY:
    return handle_key();

  case PUSH:
    newpos = mouse_position(r);
#if DND_OUT
    // See if they clicked in the selected test, if so we start a timeout
    // to see if they hold it long enough to start dragging:
    if (!event_state(ALT|META|CTRL|SHIFT) &&
	focused() && type()!=SECRET &&
	((newpos >= mark() && newpos < position()) ||
	 (newpos >= position() && newpos < mark()))) {
      drag_start = newpos;
      add_timeout(.25);
      return 1;
    }
    drag_start = -1;
#endif
    //take_focus();
    newmark = event_state(SHIFT) ? mark() : newpos;
    goto HANDLE_MOUSE;

#if DND_OUT
  case TIMEOUT:
    // This is called if they hold the mouse still long enough to indicate
    // that a drag is being started.
    if (drag_start >= 0) {
      drag_start = -1;
      copy(false);
      fltk::dnd();
    }
    return 1;
#endif

  case DRAG:
#if DND_OUT
    // If it has been long enough that we think the user is dragging the
    // mouse, but not long enough for the above timeout to happen, then
    // the user is probably selecting text and is not doing dnd.
    if (drag_start >= 0) {
      if (event_is_click()) return 1; // wait until debounce is done
      remove_timeout();
      newmark = event_state(SHIFT) ? mark() : drag_start;
      drag_start = -1;
    } else
#endif
      newmark = mark();
    newpos = mouse_position(r);
  HANDLE_MOUSE:
    // Move the mark & point to word/line ends depending on mouse click count:
    if (event_clicks()) {
      int savepos = newpos;
      if (newpos >= newmark) {
	if (newpos == newmark) {
	  if (newpos < size()) newpos++;
	  else newmark--;
	}
	if (event_clicks() > 1) {
	  newpos = line_end(newpos);
	  newmark = line_start(newmark);
	} else {
	  newpos = word_end(newpos);
	  newmark = word_start(newmark);
	}
      } else {
	if (event_clicks() > 1) {
	  newpos = line_start(newpos);
	  newmark = line_end(newmark);
	} else {
	  newpos = word_start(newpos);
	  newmark = word_end(newmark);
	}
      }
      // If the new click did not increase the selection then cycle
      // back to single-click behavior:
      if (event == PUSH && (mark() > position() ?
		(newmark >= position() && newpos <= mark()) :
		(newmark >= mark() && newpos <= position()))) {
	event_clicks(0);
	newmark = newpos = savepos;
      }
    }
    position(newpos, newmark);
    if (event != RELEASE) return 1;

  case RELEASE:
#if DND_OUT
    // If we are still waiting to see if they are doing dnd or selection,
    // act like a single click at that point:
    if (drag_start >= 0) {
      newpos = newmark = drag_start; drag_start = -1;
      remove_timeout();
      goto HANDLE_MOUSE;
    }
#endif
    if (event_button() == 2) {
      // If user selects with button 2 then it replaces the selected
      // text with the clipboard:
      event_is_click(0); // stop double-click from picking a word
      paste(*this,false);
    } else if (position_ != mark_) {
      // copy to clipboard if the selection is non-zero
      copy(false);
    }
    return 1;

  case DND_ENTER:
    fltk::belowmouse(this); // send the leave events first
    // fall through:
  case DND_DRAG: {
    int p = mouse_position(r);
#if 0 //DND_OUT
    // detect if they are dropping atop the original selection:
    if (focused() &&
	(p >= position() && p < mark() || p >= mark() && p < position())) {
      if (dnd_target == this) {
	dnd_target = 0;
	erase_cursor_at(dnd_target_position);
      }
      return 0;
    }
#endif
    if (dnd_target != this) {
      dnd_target = this;
      dnd_target_position = p;
      minimal_update(size()+1); // turn on the cursor
    } else if (dnd_target_position != p) {
      erase_cursor_at(dnd_target_position);
      dnd_target_position = p;
      minimal_update(size()+1); // turn on the cursor
    }
    return 1;}

  case DND_LEAVE:
    if (dnd_target == this) {
      dnd_target = 0;
      erase_cursor_at(dnd_target_position);
    }
    return 1;

  case DND_RELEASE:
#if DND_OUT
    if (dnd_target != this) return 0;
    if (focused()) {
      int p = dnd_target_position;
      if (p <= position() && p <= mark()) {
	// we are inserting before the selection
	if (!event_state(SHIFT|CTRL|ALT|META)) cut();
      } else if (p >= position() && p >= mark()) {
	// we are inserting after the selection, cut & adjust offset
	if (!event_state(SHIFT|CTRL|ALT|META)) {
	  dnd_target_position -= abs(mark()-position());
	  cut();
	}
      } else {
	// attempting to insert into the selection does nothing
	dnd_target = 0;
	erase_cursor_at(dnd_target_position);
	return 0;
      }
    }
#endif
    dnd_target = 0;
    position(dnd_target_position);
    take_focus();
    return 1;

  case PASTE: {
    const char* t = event_text();
	if (!t) return 0;
    int n = event_length();
    // strip trailing nulls:
    while (n > 0 && !t[n-1]) n--;
    // strip all trailing control & whitespace for single-line inputs:
    if (type() < MULTILINE)
      while (n > 0 && ((unsigned char*)t)[n-1] <= ' ') n--;
    return replace(position(), mark(), t, n);}

  default:
    return 0;
  }
}

//
// End of "$Id: Input.cxx 8751 2011-05-28 13:16:09Z bgbnbigben $".
//
