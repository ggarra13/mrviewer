//
// "$Id: windows.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// Theme plugin for fltk to more accurately emulate the Windows 98 GUI
// than the fltk default does.  Due to popular demand (ugh) some of this
// may be moved to the default...

#include <fltk/Widget.h>
#include <fltk/Style.h>

////////////////////////////////////////////////////////////////
// Different box type used by win98 sometimes:

// More accurate copy of the colors on the edges of boxes, from Win98
// Fltk by default uses colors picked by Bill for aesthetic reasons:
// newer versions of Windows & KDE look closer to FLTK default
//static const fltk::Frame_Box win98_menu_window_box(0, "2AARRMMUU", fltk::DOWN_BOX);
//extern const fltk::Frame_Box win98_down_box;
//static const fltk::Frame_Box win98_up_box(0, "2AAXXIIUU", &win98_down_box);
//       const fltk::Frame_Box win98_down_box(0, "2XXIIUUAA", &win98_up_box);

////////////////////////////////////////////////////////////////

extern "C" bool fltk_theme()
{
//  fltk::get_system_colors();


// newer versions of Windows & KDE look closer to FLTK default
//  fltk::Widget::default_style->buttonbox = &win98_up_box;
// this may be needed if fltk's default is the thin box:
//  fltk::Widget::default_style->box = &win98_down_box;

  fltk::Style* s;

  s->draw_boxes_inactive(0);

  if ((s = fltk::Style::find("menu"))) {
//    s->buttonbox = &win98_menu_window_box;
    s->leading(6);
  }

  if ((s = fltk::Style::find("item"))) {
    s->buttonbox(fltk::NO_BOX); // no box around checkmarks
  }

  if ((s = fltk::Style::find("menu bar"))) {
    s->highlight_color(fltk::GRAY75); // needed for title highlighting
  }

  if ((s = fltk::Style::find("scrollbar"))) {
//    s->box = &win98_menu_window_box;
    s->color(52);
  }

  if ((s = fltk::Style::find("highlight button"))) {
    s->highlight_color(fltk::GRAY75);
  }

  return true;
}

//
// End of "$Id: windows.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
