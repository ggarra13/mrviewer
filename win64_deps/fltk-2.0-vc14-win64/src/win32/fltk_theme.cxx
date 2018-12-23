//
// "$Id: fltk_theme.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// This is the default fltk_theme() function on Windows. It reads
// colors and font sizes from standard Windows95 interfaces.

#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <fltk/Monitor.h>
#include <fltk/Font.h>
#include <fltk/events.h>
#include <stdio.h>
#include <fltk/string.h>
#include <fltk/utf.h>
#include <fltk/x.h>
#include <limits.h>
#include <wchar.h>

using namespace fltk;

extern int has_unicode();

////////////////////////////////////////////////////////////////

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

#ifndef WHEEL_PAGESCROLL
#define WHEEL_PAGESCROLL	(UINT_MAX) /* Scroll one page */
#endif

static Color win_color(int wincol) {
  int R = wincol&0xff;
  int G = (wincol >> 8)&0xff;
  int B = (wincol >> 16)&0xff;
  Color col = color(R, G, B);
  if (col) return col;
  return BLACK;
}

static int win_fontsize(int winsize) {
  if (winsize < 0) return -winsize; // -charsize: which is what FLTK uses
  if (winsize == 0) return 12; // pick any good size.  12 is good!
  return winsize*3/4; // cellsize: convert to charsize
}

extern "C" bool fltk_theme() {

  Color background = win_color(GetSysColor(COLOR_BTNFACE));
  Color foreground = win_color(GetSysColor(COLOR_BTNTEXT));
  Color select_background = win_color(GetSysColor(COLOR_HIGHLIGHT));
  Color select_foreground = win_color(GetSysColor(COLOR_HIGHLIGHTTEXT));
  Color text_background = win_color(GetSysColor(COLOR_WINDOW));
  Color text_foreground = win_color(GetSysColor(COLOR_WINDOWTEXT));
// These colors are incorrect on XP with it's default scheme:
//   Color menuitem_background = win_color(GetSysColor(COLOR_MENU));
//   Color menuitem_foreground = win_color(GetSysColor(COLOR_MENUTEXT));
  Color tooltip_background = win_color(GetSysColor(COLOR_INFOBK));
  Color tooltip_foreground = win_color(GetSysColor(COLOR_INFOTEXT));
// Windows doesn't seem to honor this one
// Color slider_background = win_color(GetSysColor(COLOR_SCROLLBAR));

  fltk::set_background(background);
  Widget::default_style->labelcolor_ = foreground;
  Widget::default_style->highlight_textcolor_ = foreground;
  Widget::default_style->color_ = text_background;
  Widget::default_style->textcolor_ = text_foreground;
  Widget::default_style->selection_color_ = select_background;
  Widget::default_style->selection_textcolor_ = select_foreground;

  Style* style;

  if ((style = Style::find("ScrollBar"))) {
//    style->color = lerp(slider_background, text_background, .5);
    style->color_ = lerp(background, text_background, .5);
  }

//   if (menuitem_background != background || menuitem_foreground != foreground) {
//     if ((style = Style::find("Menu"))) {
//       style->color_ = menuitem_background;
//       style->textcolor_ = menuitem_foreground;
// //    style->selection_color_ = select_background;
// //    style->selection_textcolor_ = select_foreground;
//     }
//   }

/* This is the same as the defaults:
  if ((style = Style::find("menu title"))) {
    style->highlight_color_ = GRAY75;
    style->highlight_textcolor_ = foreground;
    style->selection_color_ = GRAY75;
    style->selection_textcolor_ = foreground;
  }
*/

  if ((style = Style::find("Tooltip"))) {
    style->color_ = tooltip_background;
    style->labelcolor_ = tooltip_foreground;
  }

  /*
     Windows font stuff

     It looks Windows has just three separate fonts that it actually
     uses for stuff replaced by FLTK.  But the "Display Properties"
     dialog has a lot more fonts that you can set?  Wrong, look again.
     Some of the fonts are duplicates and another doesn't do anything!
     It has fonts for the titlebar and icons which we don't have to worry
     about, a menu font which is used for menubars and menu items, a
     status font which is for status bars and tooltips, and a message
     box font which is used for everything else.  Except that it's not
     used by everything else;  almost all non-menu widgets in every
     application I tested did not respond to font changes.  The fonts
     are apparently hard coded by the applications which seems to me to
     bad programming considering that Windows has an adequate system for
     allowing the user to specify font preferences.  This is especially
     true of Microsoft applications and Windows itself!  We will allow
     FLTK applications to automatically use the fonts specified by the
     user.

     CET
  */
#ifndef _WIN32_WCE

  if (has_unicode()) {
    NONCLIENTMETRICSW ncm;
    int sncm = sizeof(ncm);
    ncm.cbSize = sncm;
    if (false == SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sncm, &ncm, SPIF_SENDCHANGE))
      return false;
    Font* font; float size;
    wchar_t *name;
    const int BUFLEN = 1024;
    char buffer[BUFLEN];

    // get font info for regular widgets from LOGFONT structure
    name = ncm.lfMessageFont.lfFaceName;
    utf8fromwc(buffer, BUFLEN, name, wcslen(name));
    font = fltk::font(buffer,
		     (ncm.lfMessageFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMessageFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMessageFont.lfHeight));

    Widget::default_style->labelfont_ = font;
    Widget::default_style->textfont_ = font;
    Widget::default_style->labelsize_ = size;
    Widget::default_style->textsize_ = size;

    // get font info for menu items from LOGFONT structure
    name = ncm.lfMenuFont.lfFaceName;
    utf8fromwc(buffer, BUFLEN, name, wcslen(name));
    font = fltk::font(buffer,
		     (ncm.lfMenuFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMenuFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMenuFont.lfHeight));
    if ((style = Style::find("Menu"))) {
      style->textfont_ = font;
      style->textsize_ = size;
    }

    if ((style = Style::find("Tooltip"))) {
      name = ncm.lfStatusFont.lfFaceName;
      utf8fromwc(buffer, BUFLEN, name, wcslen(name));
      // get font info for tooltips from LOGFONT structure
      font = fltk::font(buffer,
		       (ncm.lfStatusFont.lfWeight >= 600 ? BOLD : 0) +
		       (ncm.lfStatusFont.lfItalic ? ITALIC : 0));
      size = float(win_fontsize(ncm.lfStatusFont.lfHeight));

      style->labelfont_ = font;
      style->labelsize_ = size;
    }

  } else {

    NONCLIENTMETRICSA ncm;
    int sncm = sizeof(ncm);
    ncm.cbSize = sncm;
    if (false == SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sncm, &ncm, SPIF_SENDCHANGE))
      return false;

    Font* font; float size;
    char *name;

    // get font info for regular widgets from LOGFONT structure
    name = ncm.lfMessageFont.lfFaceName;
    font = fltk::font(name,
		     (ncm.lfMessageFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMessageFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMessageFont.lfHeight));

    Widget::default_style->labelfont_ = font;
    Widget::default_style->textfont_ = font;
    Widget::default_style->labelsize_ = size;
    Widget::default_style->textsize_ = size;

    // get font info for menu items from LOGFONT structure
    name = ncm.lfMenuFont.lfFaceName;
    font = fltk::font(name,
		     (ncm.lfMenuFont.lfWeight >= 600 ? BOLD : 0) +
		     (ncm.lfMenuFont.lfItalic ? ITALIC : 0));
    size = float(win_fontsize(ncm.lfMenuFont.lfHeight));
    if ((style = Style::find("Menu"))) {
      style->textfont_ = font;
      style->textsize_ = size;
    }

    if ((style = Style::find("Tooltip"))) {
      name = ncm.lfStatusFont.lfFaceName;
      // get font info for tooltips from LOGFONT structure
      font = fltk::font(name,
		       (ncm.lfStatusFont.lfWeight >= 600 ? BOLD : 0) +
		       (ncm.lfStatusFont.lfItalic ? ITALIC : 0));
      size = float(win_fontsize(ncm.lfStatusFont.lfHeight));

      style->labelfont_ = font;
      style->labelsize_ = size;
    }
  }
#endif
  // grab mousewheel stuff from Windows
  UINT delta;
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (PVOID)&delta, 0);
  if (delta == WHEEL_PAGESCROLL) Style::wheel_scroll_lines_ = 10000;
  else Style::wheel_scroll_lines_ = (int)delta;

  // CET - FIXME - do encoding stuff
  return true;
}

//
// End of "$Id: fltk_theme.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
