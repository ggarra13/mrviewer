//
// "$Id: event_key_state.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/*! \fn bool fltk::event_key_state(unsigned key);

  Returns true if the given key was held down (or pressed) <i>during the
  last event</i>. This is constant until the next event is read from the
  server. The possible values for the key are listed under fltk::SpaceKey.

  On Win32 event_key_state(KeypadEnter) does not work.
*/

/*! \fn bool fltk::get_key_state(unsigned key);

  Returns true if the given key is held down \e now. This is different
  than event_key_state() as that returns how the key was during the
  last event. This can also be slower as it requires a round-trip
  query to the window server. The values to pass are described
  under fltk::SpaceKey.

  On Win32 fltk::get_key_state(fltk::KeypadEnter) does not work. 
*/

#include <config.h>
#if USE_X11
# include "x11/event_key_state.cxx"
#elif defined(_WIN32)
# include "win32/event_key_state.cxx"
#elif defined(__APPLE__)
# include "osx/event_key_state.cxx"
#else
# error
#endif

//
// End of "$Id: event_key_state.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
