//
// "$Id: ask.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Copyright 2008 by Bill Spitzak and others.
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

#ifndef fltk_ask_h
#define fltk_ask_h

#include "FL_API.h"
#include "Style.h"

namespace fltk {

class Widget;

/// \name fltk/ask.h
//@{

enum {
  BEEP_DEFAULT = 0,
  BEEP_MESSAGE,
  BEEP_ERROR,
  BEEP_QUESTION,
  BEEP_PASSWORD,
  BEEP_NOTIFICATION
};

FL_API void message(const char *, ...);
FL_API void alert(const char *, ...);
FL_API int  ask(const char *, ...);
FL_API int  choice(const char *q,
		  const char *b0, const char *b1, const char *b2, ...);
FL_API int  choice_alert(const char *q,
		  const char *b0, const char *b1, const char *b2, ...);
FL_API const char *input(const char *label, const char *deflt = 0, ...);
FL_API const char *password(const char *label, const char *deflt = 0, ...);
FL_API void beep(int type = BEEP_DEFAULT);
FL_API void beep_on_dialog(bool);
FL_API bool beep_on_dialog();

extern FL_API NamedStyle* icon_style;
extern FL_API NamedStyle* message_style;

extern FL_API const char* message_window_label;
extern FL_API float message_window_timeout;

extern FL_API bool message_window_scrollable;

// pointers you can use to change FLTK to a foreign language:
extern FL_API const char* no;
extern FL_API const char* yes;
extern FL_API const char* ok;
extern FL_API const char* cancel;

}

//@}

#endif

//
// End of "$Id: ask.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
