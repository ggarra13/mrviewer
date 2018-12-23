//
// "$Id: FileInput.cxx 8316 2011-01-26 14:59:50Z bgbnbigben $"
//
// File_Input header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2006 by Bill Spitzak and others.
// Original version Copyright 1998 by Curtis Edwards.
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

#include <fltk/FileInput.h>
#include <fltk/filename.h>
#include <fltk/Window.h>
#include <fltk/draw.h>
#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>
#include <fltk/Rectangle.h>
#include <fltk/run.h>
#include <fltk/Box.h>
#include <fltk/string.h>

// temporary for new 1.1.x button FileInput features to come
//#define DIR_HEIGHT	10

//
// Height of directory buttons...
//

#define DIR_HEIGHT	10


//
// Redraw bit for directory bar...
//

#define DAMAGE_BAR	0x10

using namespace fltk;
//
// 'FileInput::FileInput()' - Create a FileInput widget.
//

FileInput::FileInput(int X, int Y, int W, int H, const char *l)
: Input(X, Y, W, H, l) {
    buttons_[0] = 0;
    errorcolor_ = RED;
    ok_entry_   = 1;
    pressed_    = -1;
    set_damage(DAMAGE_ALL);
    box(fltk::UP_BOX);
}

//
// 'FileInput::draw_buttons()' - Draw directory buttons.
//

void FileInput::draw_boxes(bool pressed,const Rectangle& r) {
    box(pressed ? fltk::DOWN_BOX : fltk::UP_BOX);
    draw_box(r);
}

void FileInput::draw_buttons() {
  int	i,					// Looping var
	X;					// Current X position

  if (damage() & (DAMAGE_BAR | DAMAGE_ALL)) {
    update_buttons();
  }

  Color c = color();
  color(buttoncolor());

  for (X = 0, i = 0; buttons_[i]; i ++)
  {
    if ((X + buttons_[i]) > xscroll()) {
      if (X < xscroll()) {
        draw_boxes(pressed_ == i ,
                 Rectangle(0, 0, X + buttons_[i] - xscroll(), DIR_HEIGHT));
      } else if ((X + buttons_[i] - xscroll()) > w()) {
	draw_boxes(pressed_ == i ,
        	 Rectangle(0 + X - xscroll(), 0, w() - X + xscroll(), DIR_HEIGHT));
      } else {
        draw_boxes(pressed_ == i ,
	         Rectangle(0 + X - xscroll(), 0, buttons_[i], DIR_HEIGHT));
      }
    }

    X += buttons_[i];
  }

  if (X < w()) {
    draw_boxes(pressed_==i,
             Rectangle(0 + X - xscroll(), 0, w() - X + xscroll(), DIR_HEIGHT));
  }
  color(c);
}

//
// 'FileInput::update_buttons()' - Update the sizes of the directory buttons.
//

void
FileInput::update_buttons() {
    int		i;				// Looping var
    const char	*start,				// Start of path component
	*end;				// End of path component
    Box * b = box();

    //  puts("update_buttons()");

    // Set the current font & size...
    float  ts = textsize();
    Font * f = textfont();
    //textfont(f);  textsize(ts);
    fltk::setfont(f,ts);
    // Loop through the value string, setting widths...
    for (i = 0, start = text();
    start && i < (int)(sizeof(buttons_) / sizeof(buttons_[0]) - 1);
    start = end, i ++) {
	//    printf("    start = \"%s\"\n", start);
	if ((end = strchr(start, '/')) == NULL)
#if defined(WIN32) || defined(__EMX__)
	    if ((end = strchr(start, '\\')) == NULL)
#endif // WIN32 || __EMX__
		break;

	    end ++;

	    buttons_[i] = (short)getwidth(start, end - start);
	    if (!i) buttons_[i] += b->dx() + 6;
    }

    //printf("    found %d components/buttons...\n", i);

    buttons_[i] = 0;
}


//
// 'FileInput::text()' - Set the string stored in the widget...
//

int						// O - TRUE on success
FileInput::text(const char *str,		// I - New string value
		int        len) {		// I - Length of value
    set_damage(DAMAGE_BAR);
    int ret = Input::text( str,len);
    update_buttons();
    return ret;
}


int						// O - TRUE on success
FileInput::text(const char *str) {		// I - New string value
    set_damage(DAMAGE_BAR);
    int ret = Input::text(str);
    update_buttons();
    return ret;
}

//
// 'FileInput::draw()' - Draw the file input widget...
//
static int line_spacing(float leading) {
  return int(getsize() + leading + .5);
}

static float line_ascent(float leading) {
  return (line_spacing(leading) + getascent() - getdescent() - 1) / 2;
}


void FileInput::draw() {
    Box *b = box();

    if (damage() & (DAMAGE_BAR | DAMAGE_ALL))
	draw_buttons();
    // this flag keeps Input_::drawtext from drawing a bogus box!
    char must_trick_Input_ =
	fltk::focus()!=this && !size() && !(damage()&DAMAGE_ALL);

    if (!must_trick_Input_) {
      Rectangle r(0,DIR_HEIGHT,w(),h()-DIR_HEIGHT);
      b->inset(r);
      //Input::b->inset(r);
      int label_width ;
      if (1 || damage() & DAMAGE_ALL) {
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
      Input::draw(r);
    }
}


//
// 'FileInput::handle()' - Handle events in the widget...
//

int						// O - TRUE if we handled event
FileInput::handle(int event)		// I - Event
{
    //  printf("handle(event = %d)\n", event);

    switch (event) {
    case MOVE :
    case ENTER :
	if (active_r()) {
	    if (event_y() < DIR_HEIGHT) window()->cursor(CURSOR_DEFAULT);
	    else window()->cursor(CURSOR_INSERT);
	}

	return 1;

    case LEAVE:
        window()->cursor(CURSOR_DEFAULT);
	return 1;

    case PUSH :
    case RELEASE :
    case DRAG :
	if (event_y() < DIR_HEIGHT || pressed_ >= 0) 
	    return handle_button(event);

	return Input::handle(event);

    case KEY:
    case KEYUP:
	if (Input::handle(event)) {
	    fltk::damage(DAMAGE_BAR);
	    if (when()&(WHEN_CHANGED|WHEN_ENTER_KEY_ALWAYS |WHEN_ENTER_KEY_ALWAYS) ) {
		clear_changed(); do_callback();
	    }
	    return 1;
	}
      return 0;
    default :
	if (Input::handle(event)) {
	    fltk::damage(DAMAGE_BAR);
	    return 1;
	}

	return 0;
    }
}


//
// 'FileInput::handle_button()' - Handle button events in the widget...
//

int						// O - TRUE if we handled event
FileInput::handle_button(int event)		// I - Event
{
    int	i;
    int X;	// Current X position
    char *start;// Start of path component
    char *end;	// End of path component
    char newtext[1024];

    // Figure out which button is being pressed...
    for (X = 0, i = 0; buttons_[i]; i ++)
    {
	X += buttons_[i];

	if (X > xscroll() && event_x() < (X - xscroll())) 
	    break;
    }

    //  printf("handle_button(event = %d), button = %d\n", event, i);

    // Redraw the directory bar...
    if (event == RELEASE) pressed_ = -1;
    else pressed_ = (short)i;

    this->make_current();
    draw_buttons();

    // Return immediately if the user is clicking on the last button or
    // has not released the mouse button...
    if (!buttons_[i] || event != RELEASE) return 1;

    // Figure out where to truncate the path...
    strlcpy(newtext, text(), sizeof(newtext));

    for (start = newtext, end = start; start && i >= 0; start = end, i --) {
	//    printf("    start = \"%s\"\n", start);
	if ((end = strchr(start, '/')) == NULL)
#if defined(WIN32) || defined(__EMX__)
	    if ((end = strchr(start, '\\')) == NULL)
#endif // WIN32 || __EMX__
		break;

	    end ++;
    }

    if (i < 0) {
	char c = *start; // possibly start == basename
	const char *basename = 0;
	if (!fltk::filename_isdir(newtext))
	    if (!*(basename = fltk::filename_name(newtext)))
		basename = 0;

	// Found the end; truncate the text and update the buttons...
	*start = '\0';

	// Prevent users from cursing us: keep basename, if not a directory
	if (basename) {
	    *start = c;
	    memmove(start, basename, strlen(basename)+1);
	    // Should have some 'ftype_' field to reflect the caller's
	    // intentions, but for now we get along without...
	    //if (!(ftype_ & CREATE) && !fltk::filename_isfile(newtext))
		//*start = 0;
	    //else
		if (start == basename) i = 0; // unchanged!
	}
	if (i < 0) {
	    text(newtext);

	    // Then do the callbacks, if necessary...
	    set_changed();
	    if (when() & WHEN_CHANGED) do_callback();
	}
    }

    return 1;
}


//
// End of "$Id: FileInput.cxx 8316 2011-01-26 14:59:50Z bgbnbigben $".
//
