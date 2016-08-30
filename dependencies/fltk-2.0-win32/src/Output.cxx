//
// "$Id: Output.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/events.h>
#include <fltk/Output.h>
#include <fltk/draw.h>

using namespace fltk;

/*! \class fltk::Output

  \image html text.gif

  This widget displays a piece of text. When you set the value(), it
  does a strcpy() to it's own storage, which is useful for
  program-generated values. You can also call static_value() if
  you know the original text will not be deleted.
  The text may contain any characters except nul. Any control characters
  less than 32 will display in ^X notation. Other characters are
  drawn without any changes.

  The user may select portions of the text
  using the mouse and paste the contents into other fields or
  programs.

*/

/*! Always returns false, thus disallowing all editing. */
bool Output::replace(int, int, const char*, int) {
  return false;
}

int Output::handle(int event) {
  // The Input may grab the focus if sent a shortcut, prevent this:
  if (event == SHORTCUT) return 0;
  // You can't drag&drop on it either:
  if (event == DND_ENTER) return 0;
  // Don't return 3 for FOCUS, return 1 instead:
  if (Input::handle(event)) {
    // clicking or selecting text does the callback:
    if (event == RELEASE) do_callback();
    return 1;
  }
  return 0;
}

static NamedStyle style("Output", 0, &Output::default_style);
/*! Output has it's own style so the color can be set to gray like
  Motif-style programs want. */
NamedStyle* Output::default_style = &::style;

Output::Output(int x, int y, int w, int h, const char *l)
  : Input(x, y, w, h, l)
{
  style(default_style);
}

/*! \class fltk::MultiLineOutput

  \image html text.gif

  Displays multiple lines of text. It will interpret ^J for line
  breaks and ^I for tabs.

  There are no scrollbars, so this is for short pieces of text only.
*/

/*! \class fltk::WordwrapOutput

  \image html text.gif

  Similar to MultiLineOutput this displays multiple lines of text. It
  will interpret ^J for line breaks and ^I for tabs. It will also wrap
  lines at spaces so long lines fit in the widget.

  There are no scrollbars, so this is for short pieces of text only.
*/

//
// End of "$Id: Output.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
