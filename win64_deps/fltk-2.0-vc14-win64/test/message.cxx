//
// "$Id: message.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Message test program for the Fast Light Tool Kit (FLTK).
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

#include <fltk/ask.h>
#include <fltk/run.h>
#include <stdio.h>

int main(int argc, char **argv) {

  fltk::args(argc, argv);

  if(!fltk::ask("Do you want to disable beep sounds ?", "continue"))
      fltk::beep_on_dialog(true);
  fltk::message("Spelling check sucessfull, %d errors found with %g%% confidence",
	     1002, 100*(15/77.0));

  fltk::alert("Quantum fluctuations in the space-time continuum detected, "
	   "you have %g seconds to comply.", 10.0);

  printf("fltk::ask returned %d\n",
    fltk::ask("Do you really want to %s?", "continue"));

  printf("fltk::choice returned %d\n",
    fltk::choice("Choose one of the following:","choice0","choice1","choice2"));
  const char *r;

  r = fltk::input("Please enter a string for '%s':", "this is the default value",
	       "testing");
  printf("fltk::input returned \"%s\"\n", r ? r : "NULL");

  r = fltk::password("Enter %s's password:", 0, "somebody");
  printf("fltk::password returned \"%s\"\n", r ? r : "NULL");

  return 0;
}

//
// End of "$Id: message.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
