//
// "$Id$"
//
// FLUID undo definitions for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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

#ifndef fluid_undo_h
# define fluid_undo_h

namespace fltk {class Widget;}

class Undo {
  static int  current;  	// Current undo level in buffer
  static int  last;		// Last undo level in buffer
  static int  save;		// Last undo level that was saved
  static int  max;		// Maximum undo level used
  static bool paused;		// Undo checkpointing paused?
  static char path[1024];	// Undo path

 public:

  static void checkpoint();	// Save current file to undo buffer
  static void clear();		// Clear undo buffer

  static void resume()  {  paused = 0;}	// Resume undo checkpoints
  static void suspend() {  paused = 1;}	// Suspend undo checkpoints
  static void update_saved();	// Update the undo pos when saved

  static const char *filename(int level, char *buf, int bufsize);
  static void remove_last();

  static void redo_cb(fltk::Widget *, void *);  // Redo menu callback
  static void undo_cb(fltk::Widget *, void *);  // Undo menu callback
};

#endif // !undo_h

//
// End of "$Id$".
//
