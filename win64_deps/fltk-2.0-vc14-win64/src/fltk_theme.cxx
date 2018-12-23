//
// "$Id: fltk_theme.cxx 8799 2011-06-10 06:37:28Z bgbnbigben $"
//
// Copyright 2004 Bill Spitzak and others.
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

/*! \fn bool fltk_theme();

  This is the default value of fltk::theme().

  This is declared as a "C" function to allow it to be replaced with
  a function from a loaded plugin. If you are statically linking fltk
  you can also override this by declaring a fltk_theme() function in
  your own code (some Unix systems, but not Linux, will do this even
  if fltk is a shared library).

  <b>Unix/Linux/X11:</b>

  Does not do anything. Previous versions of FLTK 2.0 tried to read
  the KDE setup but this has not worked recently and sometimes
  produces weird results. FLTK does not bother looking at xrdb
  databases. In any case, if any standard appears it can be
  implemented here.

  <b>Windows:</b>

  On Windows it will read colors and fonts using GetSysColor() and
  SystemParametersInfo(). FLTK will call fltk::reload_theme() when a
  WM_SYSCOLORCHANGE, WM_DISPLAYCHANGE, or WM_SETTINGCHANGE event
  happens, even if you replace fltk_theme(), so your replacement
  should be prepared for this.

*/

#include <fltk/Style.h>
#include <fltk/Button.h>
#include <fltk/Symbol.h>
#include <fltk/draw.h>
#include <fltk/Color.h>
#include <fltk/SharedImage.h>
#include <cstdio>
#include <cstring>

#define PAPERCOLOUR  0xFFFFFF00
#define BGCOLOUR     0xF4F4EC00
#define SLIDERCOLOUR 0xC7DAFF00
#define BUTTONCOLOUR fltk::GRAY75
#define BARCOLOUR    fltk::lerp(fltk::GRAY75, 0x8EA2E200, .1)

/* BS: The below is FLTK's attempt at a XP theme. 
   This may change though (2.0 being alpha, and all), so as a reference
   for the future I've got this comment here.
   TODO: move the #include into a header to avoid bloating the library, maybe.
   Either way, this will result in the final executable being slightly larger
   than it was, so I'm not sure how much this will help.
*/

#include "scrollbar_frames.dat"
#include "fourbuttons.dat"

#define FRAME_SIZE 17

//static unsigned char pixel_frame[FRAME_SIZE][FRAME_SIZE];
static unsigned char const * const frames[26] = {
	frame01, frame02, frame03, frame04, frame05, 
	frame06, frame07, frame08, frame09, frame10,
	frame11, frame12, frame13, frame14, frame15,
	frame16, frame17, frame18, frame19, frame20,
	frame21, frame22, frame23, frame24, frame25,
	frame26};
static fltk::Image* img_frame, *images[26];
static fltk::Image* pixie = 0, *pixie_hilight, *pixie_return_butt,
                    *pixie_norm, *pixie_down;

static bool scroll_glyph_initialised, xp_glyph_initialised;

static void pixie_box_init_check() {
  if (!xp_glyph_initialised) {
    xp_glyph_initialised = true;
    pixie_hilight = fltk::bmpImage::get("xp_button_hi", xp_button_hi);
    pixie_return_butt = fltk::bmpImage::get("xp_button_return", xp_button_return);
    pixie_down = fltk::bmpImage::get("xp_button_dn", xp_button_dn);
    pixie = pixie_norm = fltk::bmpImage::get("xp_button_up", xp_button_up);
  }
}

static void scroll_glyph_init_check() {
  if (!scroll_glyph_initialised) {
    scroll_glyph_initialised = true;
    img_frame = new fltk::Image(FRAME_SIZE, FRAME_SIZE, "current_image");
    char buffer[10];
    for (int i = 0; i < 26; i++){
      if (i < 10)
        sprintf(buffer, "frame0%d", i+1);
      else
        sprintf(buffer, "frame%d",  i);
      images[i] = fltk::gifImage::get(buffer, frames[i]);
    }
  }
}

static void set_frame(int frame) {
  scroll_glyph_init_check();
  img_frame = images[frame];
}

enum {
  H_TRACK = 0,
  H_SLIDER,
  H_LEFT_ARROW,
  H_RIGHT_ARROW,
  H_SLIDER_HI,
  H_LEFT_ARROW_HI,
  H_RIGHT_ARROW_HI,
  H_SLIDER_PUSHED,
  H_LEFT_ARROW_PUSHED,
  H_RIGHT_ARROW_PUSHED,
  H_KNURL,
  H_KNURL_HI,
  H_KNURL_PUSHED,

  V_TRACK,
  V_SLIDER,
  V_UP_ARROW,
  V_DOWN_ARROW,
  V_SLIDER_HI,
  V_UP_ARROW_HI,
  V_DOWN_ARROW_HI,
  V_SLIDER_PUSHED,
  V_UP_ARROW_PUSHED,
  V_DOWN_ARROW_PUSHED,
  V_KNURL,
  V_KNURL_HI,
  V_KNURL_PUSHED
}; // 13 frames each

static void draw_arrows(const fltk::Rectangle& R, int drawflags) {
  if (drawflags & fltk::ALIGN_TOP) {
    if (drawflags & fltk::PUSHED)
      set_frame(V_UP_ARROW_PUSHED);
    else if (drawflags & fltk::HIGHLIGHT)
      set_frame(V_UP_ARROW_HI);
    else
      set_frame(V_UP_ARROW);
    img_frame->draw(R);
  } else if (drawflags & fltk::ALIGN_BOTTOM) {
    if (drawflags & fltk::PUSHED)
      set_frame(V_DOWN_ARROW_PUSHED);
    else if (drawflags & fltk::HIGHLIGHT)
      set_frame(V_DOWN_ARROW_HI);
    else
      set_frame(V_DOWN_ARROW);
    img_frame->draw(R);
    
  } else if (drawflags & fltk::ALIGN_LEFT) {
    if (drawflags & fltk::PUSHED)
      set_frame(H_LEFT_ARROW_PUSHED);
    else if (drawflags & fltk::HIGHLIGHT)
      set_frame(H_LEFT_ARROW_HI);
    else
      set_frame(H_LEFT_ARROW);
    img_frame->draw(R);
  } else if (drawflags & fltk::ALIGN_RIGHT) {
    if (drawflags & fltk::PUSHED)
      set_frame(H_RIGHT_ARROW_PUSHED);
    else if (drawflags & fltk::HIGHLIGHT)
      set_frame(H_RIGHT_ARROW_HI);
    else
      set_frame(H_RIGHT_ARROW);
    img_frame->draw(R);
  }
}

static class XPScrollBarGlyph : public fltk::Symbol {
  public:
    void _draw(const fltk::Rectangle& R) const {
      scroll_glyph_init_check();
      if (fltk::drawflags() & fltk::ALIGN_MASK) {
	draw_arrows(R, fltk::drawflags());
	if (fltk::drawflags() & fltk::ALIGN_BOTTOM) {
	  if (R.h() > R.w()) {
            set_frame(V_TRACK);
            img_frame->draw_diced(R);
          } else if (R.w() > R.h()) {
            set_frame(H_TRACK);
            img_frame->draw_diced(R);
          }
	}
      } else {
	if (R.x() == 0) { // Vertical
	  if (fltk::drawflags() & fltk::PUSHED)
	    set_frame(V_SLIDER_PUSHED);
	  else if (fltk::drawflags() & fltk::HIGHLIGHT)
	    set_frame(V_SLIDER_HI);
	  else
	    set_frame(V_SLIDER);
	  img_frame->draw_diced(R);
	  
	  if (fltk::drawflags() & fltk::PUSHED)
	    set_frame(V_KNURL_PUSHED);
	  else if (fltk::drawflags() & fltk::HIGHLIGHT)
	    set_frame(V_KNURL_HI);
	  else
	    set_frame(V_KNURL);

	  fltk::Rectangle rr(R);
	  if (rr.h() <= 18) {
	    rr.move_y(4);
	    rr.move_b(-4);
	  } else {
	    rr.move_y((rr.h() / 2) - 7); // Magical constants c/o rainbowsally
	    rr.h(FRAME_SIZE);
	  }
	  fltk::push_clip(rr);
	  img_frame->draw_diced(rr);
	  fltk::pop_clip();
	} else if (R.y() == 0){
	  if (fltk::drawflags() & fltk::PUSHED)
	    set_frame(H_SLIDER_PUSHED);
	  else if (fltk::drawflags() & fltk::HIGHLIGHT)
	    set_frame(H_SLIDER_HI);
	  else
	    set_frame(H_SLIDER);
	  img_frame->draw_diced(R);

	  if (fltk::drawflags() & fltk::PUSHED)
	    set_frame(H_KNURL_PUSHED);
	  else if (fltk::drawflags() & fltk::HIGHLIGHT)
	    set_frame(H_KNURL_HI);
	  else
	    set_frame(H_KNURL);

	  fltk::Rectangle rr(R);
	  if (rr.w() <= 18) {
	    rr.move_x(4);
	    rr.move_r(-4);
	  } else {
	    rr.move_x((rr.w() / 2) - 7); // Magical constants again c/o rainbowsally
	    rr.w(FRAME_SIZE);
	  }
	  fltk::push_clip(rr);
	  img_frame->draw(rr);
	  fltk::pop_clip();
	}
      }
    }
    XPScrollBarGlyph() : fltk::Symbol("XPScrollBarGlyph") {}
    ~XPScrollBarGlyph() {
      delete img_frame;
    }
} xp_glyph;

static class ArrowGlyph : public fltk::Symbol {
  public:
    void _draw(const fltk::Rectangle& R) const {
      draw_arrows(R, fltk::drawflags());
    }
    ArrowGlyph() : fltk::Symbol("arrow glyph") {}
} arrow_glyph;

static class NoGlyph : public fltk::Symbol {
  public:
    void _draw(const fltk::Rectangle& r) const{}
    NoGlyph() : fltk::Symbol("no_glyph") {}
} no_glyph;

class NewFocusBox : public fltk::Symbol {
  public:
    NewFocusBox() : fltk::Symbol("NewFocusBox") {}
    void _draw(const fltk::Rectangle& R) const {
      fltk::Rectangle r(R);
      r.inset(3);
      fltk::DEFAULT_FOCUS_BOX->_draw(r);
    }
} new_focusbox;

static class XPBox : public fltk::Box {
  protected:
    const char* data_;
    const fltk::Box* down_;
    bool type_return_;
  public:
    inline const char* data() const {return data_;}
    inline void data(const char* d) {data_ = d;}
    void _draw(const fltk::Rectangle& R) const {
      pixie_box_init_check();

      if (fltk::drawflags() & fltk::PUSHED || (fltk::drawflags() & fltk::STATE))
        pixie = pixie_down;
      else if (fltk::drawflags() & fltk::HIGHLIGHT)
        pixie = pixie_hilight;
      else if (is_return_box())
        pixie = pixie_return_butt;
      else pixie = pixie_norm;
       pixie->draw_diced(R);
    }

    bool fills_rectangle() const {return true;}
    bool is_frame() const {return false;}
    bool is_return_box() const {return type_return_;}
    XPBox(const char* name, int dx, int dy, int dw, int dh,
          const char* pattern, const fltk::Box* down = 0) : fltk::Box(name),
          data_(pattern), down_(down) {
      set_inset(dx, dy, -dw, -dh);
      if(strcmp(pattern, "return") == 0)
        type_return_ = true;
      else
        type_return_ = false;
    }

} xp_box("XPDownBox", 5, 5, 10, 10, "down"), xp_return_box("XPDownBox", 5, 5, 10, 10, "return", &xp_box);

extern "C" bool xp_theme() {
  fltk::Style* s;
  if((s = fltk::Style::find("default"))) {
    s->scrollbar_width(17);
    s->color(PAPERCOLOUR);
    s->buttoncolor(BUTTONCOLOUR);
    s->labelsize(12.0);
    s->textsize(12.0);
    s->leading(4);
    s->selection_color(0x316AC500);
    s->alt_color(BARCOLOUR); // used by some buttons
  }
  if ((s = fltk::Style::find("Window"))) {
    s->color(BGCOLOUR);
  }
  if ((s = fltk::Style::find("Scrollbar"))) {
    s->highlight_color(s->color());
    s->color(-1);
    s->glyph(&xp_glyph);
    s->box(&xp_glyph);
    s->buttonbox(&xp_glyph);
  }
  if ((s = fltk::Style::find("PopupMenu"))) {
    s->glyph(&arrow_glyph);
    s->buttonbox(&xp_box);
    s->highlight_color(s->color());
  }
  if ((s = fltk::Style::find("Choice"))) {
    s->glyph(&arrow_glyph);
    s->color(fltk::WHITE);
    s->highlight_color(s->color());
    s->box(&xp_box);
  }
  if ((s = fltk::Style::find("Button"))) {
    s->buttonbox(&xp_box);
    s->highlight_color(fltk::WHITE);
    s->focusbox(&new_focusbox);
    s->glyph(&no_glyph);
  }
  if ((s = fltk::Style::find("ReturnButton"))) {
    s->buttonbox(&xp_return_box);
    s->highlight_color(fltk::WHITE);
    s->focusbox(&new_focusbox);
    s->glyph(&no_glyph);
  }
  if ((s = fltk::Style::find("Adjuster"))) {
    s->buttonbox(&xp_box);
    s->color(BGCOLOUR);
    s->buttoncolor(SLIDERCOLOUR);
    s->highlight_color(s->color());
  }
  if ((s = fltk::Style::find("Slider"))) {
    s->buttonbox(&xp_box);
    s->color(BGCOLOUR);
    s->buttoncolor(SLIDERCOLOUR);
    s->highlight_color(s->color());
  }
  if ((s = fltk::Style::find("InputBrowser"))) {
    s->glyph(&arrow_glyph);
    s->highlight_color(s->color());
  }
  if ((s = fltk::Style::find("ValueInput"))) {
    s->glyph(&arrow_glyph);
    s->color(PAPERCOLOUR);
    s->buttoncolor(SLIDERCOLOUR);
    s->highlight_color(s->color());
  }
  if ((s = fltk::Style::find("CycleButton"))) {
    s->box(&xp_box);
    s->highlight_color(s->color());
  }
  if ((s = fltk::Style::find("Menu"))) {
    s->color(BGCOLOUR);
    s->highlight_color(fltk::DARK_BLUE);
  }
  if ((s = fltk::Style::find("BarGroup"))) {
    s->glyph(&arrow_glyph);
    s->highlight_color(s->color());
  }
  if ((s = fltk::Style::find("Item"))) {
    s->color(PAPERCOLOUR);
    s->buttoncolor(SLIDERCOLOUR);
    s->selection_color(0x316AC500);
    s->leading(5);
  }
  return true;
}

#undef PAPERCOLOUR
#undef BGCOLOUR
#undef SLIDERCOLOUR
#undef BUTTONCOLOUR
#undef BARCOLOUR
#undef FRAME_SIZE
///////////////////////////////////////////////
// End of XP theme

#include <fltk/FL_API.h>

extern "C" FL_API bool fltk_theme();

#if USE_X11

extern "C" bool fltk_theme() {return false; /* true? */}
/* Maybe _WIN32 should use the Windows version anyway? It would work! */
// This doesn't seem to want to compile at the moment.....
//# include "x11/fltk_theme.cxx"

#elif defined(_WIN32)

# include "win32/fltk_theme.cxx"

#elif USE_QUARTZ

//+++ implement Quartz support for themes
extern "C" bool fltk_theme() {return false; /* true? */}

#else

extern "C" bool fltk_theme() {return false; /* true? */}

#endif

//
// End of "$Id"
//
