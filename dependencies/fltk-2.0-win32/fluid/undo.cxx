//
// "$Id$"
//
// FLUID undo support for the Fast Light Tool Kit (FLTK).
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

//#include <fltk/run.h>
#include <fltk/Preferences.h>
#include <fltk/string.h>
#include <fltk/MenuBar.h>
#include <fltk/Item.h>

#include "PrefsData.h"
#include "FluidType.h"
#include "undo.h"

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <io.h>
#  include <windows.h>
#  define getpid (int)GetCurrentProcessId
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#  define unlink _unlink
#else
#  include <unistd.h>
#endif // WIN32 && !__CYGWIN__


extern fltk::Item*  undo_item[2];
const int UNDO_ITEM=0;
const int REDO_ITEM=1;

//
// This file implements an undo system using temporary files; ideally
// we'd like to do this in memory, however the current data structures
// and design aren't well-suited...  Instead, we save and restore
// checkpoint files.
//
int Undo::current = 0;
int Undo::last = 0;
int Undo::max = 0;
int Undo::save = -1;			// Last undo level that was saved
char Undo::path[1024]="";
bool Undo::paused=false;

// Return the undo filename
const char *Undo::filename(int level, char *buf, int bufsize) {

  if (!path[0]) prefs.getUserdataPath(path, sizeof(path));
  snprintf(buf, bufsize, "%sundo_%d_%d.fl", path, getpid(), level);
  return buf;
}

void Undo::update_saved() {
  if (modflag && current <= save) save = -1;
  else if (!modflag) save = current;
}

// Redo menu callback
void Undo::redo_cb(fltk::Widget *, void *) {
  char	fname[1024];			// Undo checkpoint file
  if (current >= last) return;

  suspend();
  if (!read_file(filename(current + 1, fname, sizeof(fname)), 0)) {
    // Unable to read checkpoint file, don't redo...
    resume();
    return;
  }

  current ++;

  // Update modified flag...
  modflag = current != save;

  // Update undo/redo menu items...
  if (current >= last) undo_item[REDO_ITEM]->deactivate();
  undo_item[UNDO_ITEM]->activate();
}

// Undo menu callback
void Undo::undo_cb(fltk::Widget *, void *) {
  char	fname[1024];			// Undo checkpoint file
  
  if (current <= 0) return;

  if (current == last) {
    write_file(filename(current, fname, sizeof(fname)));
  }

  suspend();
  if (!read_file(filename(current - 1, fname, sizeof(fname)), 0)) {
    // Unable to read checkpoint file, don't undo...
    resume();
    return;
  }

  current--;

  // Update modified flag...
  modflag = (current != save);

  // Update undo/redo menu items...
  if (current <= 0) undo_item[UNDO_ITEM]->deactivate();
  undo_item[REDO_ITEM]->activate();
  resume();
}

// Save current file to undo buffer
void Undo::checkpoint() {
  char	fname[1024];			// Undo checkpoint filename

//  printf("undo_checkpoint(): current=%d, undo_paused=%d, modflag=%d\n",
//         current, undo_paused, modflag);

  // Don't checkpoint if undo_suspend() has been called...
  if (paused) return;

  // Save the current UI to a checkpoint file...
  if (!write_file(filename (current, fname, sizeof(fname)))) {
    // Don't attempt to do undo stuff if we can't write a checkpoint file...
    perror(fname);
    return;
  }

  // Update the saved level...
  if (modflag && current <= save) save = -1;
  else if (!modflag) save = current;

  // Update the current undo level...
  current ++;
  last = current;
  if (current > max) max = current;

  // Enable the Undo and disable the Redo menu items...
  undo_item[UNDO_ITEM]->activate();
  undo_item[REDO_ITEM]->deactivate();
}

// Clear undo buffer
void Undo::clear() {
  char	fname[1024];			// Undo checkpoint filename

  // Remove old checkpoint files...
  for (int i = 0; i <= max; i ++) {
    unlink(filename (i, fname, sizeof(fname)));
  }

  // Reset current, last, and save indices...
  current = last = max = 0;
  if (modflag) save = -1;
  else save = 0;
}

// remove the last set checkpoint
void Undo::remove_last() {
    current --;    
    last --;
    if (current <=0)   
	undo_item[UNDO_ITEM]->deactivate();

}

//
// End of "$Id$".
//
