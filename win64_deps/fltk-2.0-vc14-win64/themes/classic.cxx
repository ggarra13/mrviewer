//
// "$Id: classic.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// fltk classic appearance

#include <fltk/Widget.h>
#include <fltk/CheckButton.h>
#include <fltk/Scrollbar.h>
#include <fltk/Input.h>
#include <fltk/Output.h>
#include <fltk/Style.h>
#include <fltk/Box.h>
#include <string.h>

static void choice_glyph(const fltk::Widget* widget, int,
			 int x,int y,int w,int h, fltk::Flags f)
{
  int H = 7;
  fltk::THIN_UP_BOX->draw(x, y+(h-H)/2, w-4, H, widget->buttoncolor(), f);
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
static const Motif_Labeltype motif_label(0);

static fltk::FrameBox always_up_box(0, "AAAAWUJJUTNN");
static fltk::HighlightBox menu_title_box(0, &always_up_box);

static fltk::FrameBox classic_down_box(0, "NNTUJJUWAAAA");
static fltk::FrameBox classic_up_box(0, "AAAAWUJJUTNN", &classic_down_box);

extern "C" bool fltk_theme() {

  fltk::get_system_colors();

  fltk::Widget::default_style->buttonbox = &classic_up_box;
  fltk::Widget::default_style->box = &classic_down_box;
  fltk::Widget::default_style->highlight_color = 0;
  fltk::Widget::default_style->labelsize = 14;
  fltk::Widget::default_style->textsize = 14;
  fltk::Widget::default_style->selection_color = fltk::GRAY60;
  fltk::Widget::default_style->selection_textcolor = 0;

  fltk::Style* s;
  if ((s = fltk::Style::find("button"))) {
    s->box = &classic_up_box;
  }
  if ((s = fltk::Style::find("menu"))) {
    s->selection_color = fltk::WHITE;
    s->selection_textcolor = 0;
    s->color = fltk::GRAY75;
  }
  if ((s = fltk::Style::find("item"))) {
    s->textcolor = fltk::BLACK;
  }
  if ((s = fltk::Style::find("menu_bar"))) {
    s->buttonbox = &menu_title_box;
    s->selection_color = fltk::GRAY75;
  }
  if ((s = fltk::Style::find("check_button"))) {
    s->textcolor = fltk::BLACK;
    //s->buttoncolor = fltk::GRAY;
  }
  if ((s = fltk::Style::find("output"))) {
    s->color = fltk::GRAY75;
  }
  if ((s = fltk::Style::find("choice"))) {
    s->glyph = choice_glyph;
  }
  return true;
}

//
// End of "$Id: classic.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
