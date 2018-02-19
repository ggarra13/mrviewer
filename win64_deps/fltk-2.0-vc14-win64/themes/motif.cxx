//
// "$Id: motif.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Theme plugin file for FLTK
//
// Copyright 1999 Bill Spitzak and others.
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

// Motif L&F theme plugin for fltk.  A good example of the capabilities
// of theme plugins.

#include <fltk/Widget.h>
#include <fltk/CheckButton.h>
#include <fltk/Scrollbar.h>
#include <fltk/Input.h>
#include <fltk/Output.h>
#include <fltk/Style.h>
#include <fltk/Box.h>
#include <fltk/draw.h>

// some new boxtypes (look familiar?)
static fltk::FrameBox
thick_motif_down_box("motif thick down", "HHVVHHVVHHVV");

static fltk::FrameBox
thick_motif_up_box("motif thick up", "VVHHVVHHVVHH", &thick_motif_down_box);

static fltk::HighlightBox
thick_motif_highlight_box("motif thick highlight", &thick_motif_up_box);

static fltk::HighlightBox
thick_motif_menu_box("motif thick menu", &thick_motif_up_box);

static fltk::FrameBox
thin_motif_down_box("motif down", "HHVVHHVV");

static fltk::FrameBox
thin_motif_up_box("motif up","VVHHVVHH",&thin_motif_down_box);

static fltk::HighlightBox
thin_motif_highlight_box("motif thin highlight", &thin_motif_up_box);

static fltk::FrameBox
thin_motif_always_up_box("motif up","VVHHVVHH");

static fltk::HighlightBox
thin_motif_menu_box("motif thin menu", &thin_motif_always_up_box);

static void motif_check_glyph(const fltk::Widget* widget, int t,
			int x, int y, int w, int h, fltk::Flags f)
{
  fltk::Color fc = fltk::inactive(widget->textcolor(), f);
  fltk::Color bc = widget->color();
  if (widget->type()==fltk::Button::RADIO) {
    w = (w-1)|1; h = (h-1)|1;
    int x1 = x+w/2;
    int y1 = y+h/2;
    fltk::Color light = fltk::GRAY90, dark = fltk::GRAY33;
    if (f&fltk::VALUE) { light = fltk::GRAY33; dark = fltk::GRAY90; }
//  if (f&fltk::INACTIVE)
//    { light = fltk::inactive(light); dark = fltk::inactive(dark); }
    fltk::setcolor((f & fltk::VALUE) ? fc : bc);
    fltk::addvertex(x+3,y1); fltk::addvertex(x1+1,y+2); fltk::addvertex(x+w-3,y1); fltk::addvertex(x1,y+h-4);
    fltk::fillpath();

    fltk::setcolor(light); fltk::drawline(x, y1, x1, y); fltk::drawline(x1, y, x+w-1, y1);
    fltk::setcolor(light); fltk::drawline(x+1, y1, x1, y+1); fltk::drawline(x1, y+1, x+w-2, y1);
    fltk::setcolor(light); fltk::drawline(x+2, y1, x1, y+2); fltk::drawline(x1, y+2, x+w-3, y1);

    fltk::setcolor(dark); fltk::drawline(x, y1, x1, y+h-1); fltk::drawline(x1, y+h-1, x+w-1, y1);
    fltk::setcolor(dark); fltk::drawline(x+1, y1, x1, y+h-2); fltk::drawline(x1, y+h-2, x+w-2, y1);
    fltk::setcolor(dark); fltk::drawline(x+2, y1, x1, y+h-3); fltk::drawline(x1, y+h-3, x+w-3, y1);
  } else {
    x += 2; y += 2; w -= 4; h -= 4; // fudge factor
    thin_motif_up_box.draw(x, y, w, h,
			   (f & fltk::VALUE) ? fc : bc, f);
  }
}

static void motif_glyph(const fltk::Widget* widget, int t,
			int x, int y, int w, int h, fltk::Flags f)
{
  fltk::Color bc = widget->buttoncolor();
  switch (t) {
    case fltk::GLYPH_RIGHT_BUTTON:
    case fltk::GLYPH_LEFT_BUTTON:
    case fltk::GLYPH_UP_BUTTON:
    case fltk::GLYPH_DOWN_BUTTON:
      // draw background of scroll buttons
      fltk::setcolor(widget->color());
      fltk::fillrect(x,y,w,h);
      goto JUMP1;
    case fltk::GLYPH_RIGHT:
    case fltk::GLYPH_LEFT:
    case fltk::GLYPH_UP:
    case fltk::GLYPH_DOWN:
      // menu fudge factor
      if (w > 10) {x += (w-10)/2; y += (w-10)/2; w = h = 10;}
//	x += 2; y += 2; w -= 4; h -= 4;
//	x += 4; y += 4; w -= 8; h -= 8;
    JUMP1:
      if (h > w) {y += (h-w)/2; h = w;}
      else if (w > h) {x += (w-h)/2; w = h;}
      {fltk::Color d1, d2, l1, l2;
      if (f&fltk::VALUE) {
        d1 = fltk::GRAY90; d2 = fltk::GRAY90; l1 = fltk::GRAY33; l2 = fltk::GRAY33;
      } else{
        l1 = fltk::GRAY90; l2 = fltk::GRAY90; d1 = fltk::GRAY33; d2 = fltk::GRAY33;
      }

//       if (f&fltk::INACTIVE) {
//         l1 = fltk::inactive(l1); l2 = fltk::inactive(l2);
//         d1 = fltk::inactive(d1); d2 = fltk::inactive(d2);
//       }

      switch (t) {
      case fltk::GLYPH_RIGHT:
      case fltk::GLYPH_RIGHT_BUTTON:
        fltk::setcolor(bc); fltk::newpath(); fltk::addvertex(x, y); fltk::addvertex(x+w-1, y+h/2);
        fltk::addvertex(x,y+h-1); fltk::closepath(); fltk::fillpath();
        fltk::setcolor(l2); fltk::drawline(x+2,y+2, x+w-3,y+h/2);
        fltk::setcolor(d2); fltk::drawline(x+2,y+h-3, x+w-3,y+h/2);
        fltk::setcolor(l2); fltk::drawline(x+1,y+h-2, x+1,y+1); fltk::drawline(x+1,y+1, x+w-2,y+h/2);
        fltk::setcolor(d2); fltk::drawline(x+1,y+h-2, x+w-2,y+h/2);
        fltk::setcolor(l1); fltk::drawline(x,y+h-1, x,y); fltk::drawline(x,y, x+w-1,y+h/2);
        fltk::setcolor(d1); fltk::drawline(x,y+h-1, x+w-1,y+h/2);
        break;
      case fltk::GLYPH_LEFT:
      case fltk::GLYPH_LEFT_BUTTON:
        fltk::setcolor(bc); fltk::newpath(); fltk::addvertex(x+w-1, y); fltk::addvertex(x+w-1, y+h-1);
        fltk::addvertex(x,y+h/2); fltk::closepath(); fltk::fillpath();
        fltk::setcolor(d2); fltk::drawline(x+w-3,y+h-3, x+2,y+h/2);
        fltk::setcolor(l2); fltk::drawline(x+w-3,y+2, x+2,y+h/2);
        fltk::setcolor(d2); fltk::drawline(x+w-2,y+1, x+w-2,y+h-2); fltk::drawline(x+w-2,y+h-2, x+1,y+h/2);
        fltk::setcolor(l2); fltk::drawline(x+w-2,y+1, x+1,y+h/2);
        fltk::setcolor(d1); fltk::drawline(x+w-1,y, x+w-1,y+h-1); fltk::drawline(x+w-1,y+h-1, x,y+h/2);
        fltk::setcolor(l1); fltk::drawline(x+w-1,y, x,y+h/2);
        break;
      case fltk::GLYPH_DOWN:
      case fltk::GLYPH_DOWN_BUTTON:
        fltk::setcolor(bc); fltk::newpath(); fltk::addvertex(x, y); fltk::addvertex(x+w/2, y+h-1);
        fltk::addvertex(x+w-1,y); fltk::closepath(); fltk::fillpath();
        fltk::setcolor(l2); fltk::drawline(x+2,y+2, x+w/2,y+h-3);
        fltk::setcolor(d2); fltk::drawline(x+w-3,y+2, x+w/2,y+h-3);
        fltk::setcolor(l2); fltk::drawline(x+w-2,y+1, x+1,y+1); fltk::drawline(x+1,y+1, x+w/2,y+h-2);
        fltk::setcolor(d2); fltk::drawline(x+w-2,y+1, x+w/2,y+h-2);
        fltk::setcolor(l1); fltk::drawline(x+w-1,y, x,y); fltk::drawline(x,y, x+w/2,y+h-1);
        fltk::setcolor(d1); fltk::drawline(x+w-1,y, x+w/2,y+h-1);
        break;
      case fltk::GLYPH_UP:
      case fltk::GLYPH_UP_BUTTON:
        fltk::setcolor(bc); fltk::newpath(); fltk::addvertex(x, y+h-1); fltk::addvertex(x+w-1, y+h-1);
        fltk::addvertex(x+w/2,y); fltk::closepath(); fltk::fillpath();
        fltk::setcolor(d2); fltk::drawline(x+w-3,y+h-3, x+w/2,y+2);
        fltk::setcolor(l2); fltk::drawline(x+2,y+h-3, x+w/2,y+2);
        fltk::setcolor(d2); fltk::drawline(x+1,y+h-2, x+w-2,y+h-2); fltk::drawline(x+w-2,y+h-2, x+w/2,y+1);
        fltk::setcolor(l2); fltk::drawline(x+1,y+h-2, x+w/2,y+1);
        fltk::setcolor(d1); fltk::drawline(x,y+h-1, x+w-1,y+h-1); fltk::drawline(x+w-1,y+h-1, x+w/2,y);
        fltk::setcolor(l1); fltk::drawline(x,y+h-1, x+w/2,y);
        break;
      }
      break;
    }
    default:
      fltk::Widget::default_glyph(widget, t, x,y,w,h, f);
  }
}

static void choice_glyph(const fltk::Widget* widget, int,
			 int x,int y,int w,int h, fltk::Flags f)
{
  fltk::Color color = widget->color();
  fltk::setcolor(color);
  fltk::fillrect(x,y,w,h);
  int H = h/3;
  y += (h-H)/2;
  h = H;
  widget->box()->draw(x,y,w,h,color,f);
}

#include <fltk/LabelType.h>
// Disable the engraving of inactive labels:
class Motif_Labeltype : public fltk::LabelType {
public:
  void draw(const char* label,
	    int X, int Y, int W, int H,
	    fltk::Color c, fltk::Flags f) const
    {
      if (f & fltk::INACTIVE) {
	c = fltk::inactive(c);
	f &= ~fltk::INACTIVE;
      }
      fltk::NORMAL_LABEL->draw(label, X,Y,W,H,c,f);
    }
  Motif_Labeltype(const char*n) : fltk::LabelType(n) {}
};
static Motif_Labeltype motif_label(0);

// This is the function that is called by fltk when the plugin is loaded:
extern "C" bool fltk_theme()
{
  // On X fltk::get_system_colors reads the XResource database for
  // some Motif-like things like *background:
  fltk::get_system_colors();

  fltk::Style::draw_boxes_inactive = false;

  fltk::Widget::default_style->buttonbox = &thin_motif_up_box;
  fltk::Widget::default_style->focusbox = &thin_motif_down_box;
  fltk::Widget::default_style->box = &thin_motif_down_box;
  fltk::Widget::default_style->color = fltk::GRAY75;
  fltk::Widget::default_style->selection_color = fltk::BLACK;
  fltk::Widget::default_style->selection_textcolor = fltk::GRAY75;
  fltk::Widget::default_style->glyph = motif_glyph;
  fltk::Widget::default_style->labeltype = &motif_label;

  fltk::Style* s;

  if ((s = fltk::Style::find("button"))) {
    s->box = &thin_motif_up_box;
  }

  if ((s = fltk::Style::find("menu"))) {
    s->buttonbox = &thin_motif_menu_box; // this does not work anymore...
    //s->leading = 0;
  }

  if ((s = fltk::Style::find("menu bar"))) {
    s->buttonbox = &thin_motif_menu_box;
  }

  if ((s = fltk::Style::find("menu button"))) {
    s->buttonbox = &thin_motif_menu_box;
  }

  if ((s = fltk::Style::find("choice"))) {
    s->box = &thin_motif_up_box;
    s->glyph = choice_glyph;
  }

  if ((s = fltk::Style::find("scrollbar"))) {
    s->box = &thin_motif_down_box;
    s->color = fltk::GRAY66;
  }

  if ((s = fltk::Style::find("slider"))) {
    s->box = &thin_motif_down_box;
    s->color = fltk::GRAY66;
  }

  if ((s = fltk::Style::find("value slider"))) {
    s->color = fltk::GRAY66;
  }

  if ((s = fltk::Style::find("highlight button"))) {
    s->box = &thin_motif_highlight_box;
    s->highlight_color = fltk::GRAY75;
  }

  if ((s = fltk::Style::find("check button"))) {
    s->buttoncolor = fltk::GRAY66;
    s->glyph = motif_check_glyph;
  }

  if ((s = fltk::Style::find("radio button"))) {
    s->buttoncolor = fltk::GRAY66;
  }

  return true;
}

//
// End of "$Id: motif.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
