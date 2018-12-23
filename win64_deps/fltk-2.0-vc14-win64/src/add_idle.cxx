//
// "$Id: add_idle.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// Allows you to manage an arbitrary set of idle() callbacks.
// Replaces the older set_idle() call (which is used to implement this)

#include <fltk/run.h>
using namespace fltk;

struct IdleCb {
  void (*cb)(void*);
  void* data;
  IdleCb *next;
};

// the callbacks are stored linked in a ring.  last points at the one
// just called, first at the next to call.  last->next == first.

static IdleCb* first;
static IdleCb* last;
static IdleCb* freelist;

static void call_idle() {
  IdleCb* p = first;
  last = p; first = p->next;
  p->cb(p->data); // this may call add_idle() or remove_idle()!
}

/*!
  Adds a callback function that is called every time by fltk::wait()
  and also makes it act as though the timeout is zero (this makes
  fltk::wait() return immediately, so if it is in a loop it is called
  repeatedly, and thus the idle fucntion is called repeatedly). The
  idle function can be used to get background processing done.

  You can have multiple idle callbacks. They are called one after
  another in a round-robin fashion, checking for events between each.

  fltk::wait() and fltk::check() call idle callbacks, but
  fltk::ready() does not.

  The idle callback can call any FLTK functions, including
  fltk::wait(), fltk::check(), and fltk::ready(). In this case fltk
  will not recursively call the idle callback.
*/
void fltk::add_idle(TimeoutHandler cb, void* data) {
  IdleCb* p = freelist;
  if (p) freelist = p->next;
  else p = new IdleCb;
  p->cb = cb;
  p->data = data;
  if (first) {
    last->next = p;
    last = p;
    p->next = first;
  } else {
    first = last = p;
    p->next = p;
    set_idle(call_idle);
  }
}

/*! Returns true if the specified idle callback is currently installed. */
bool fltk::has_idle(TimeoutHandler cb, void* data) {
  for (IdleCb* p = first; p != last; p = p->next)
    if (p->cb == cb && p->data == data) return true;
  return false;
}

/*! Removes the specified idle callback, if it is installed. */
void fltk::remove_idle(TimeoutHandler cb, void* data) {
  IdleCb* p = first;
  if (!p) return;
  IdleCb* l = last;
  for (;; p = p->next) {
    if (p->cb == cb && p->data == data) break;
    if (p==last) return; // not found
    l = p;
  }
  if (l == p) { // only one
    first = last = 0;
    set_idle(0);
  } else {
    last = l;
    first = l->next = p->next;
  }
  p->next = freelist;
  freelist = p;
}

//
// End of "$Id: add_idle.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
