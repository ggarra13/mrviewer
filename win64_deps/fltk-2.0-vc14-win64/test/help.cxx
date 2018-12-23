//
// "$Id: help.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Fl_Help_Dialog test program.
//
// Copyright 1999-2003 by Easy Software Products.
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
// Contents:
//
//   main() - Display the help GUI...
//

//
// Include necessary headers...
//

#include <fltk/run.h>
#include <fltk/HelpDialog.h>

//
// 'main()' - Display the help GUI...
//

int				// O - Exit status
main(int  argc,			// I - Number of command-line arguments
     char *argv[])		// I - Command-line arguments
{
  fltk::HelpDialog	*help;		// Help dialog


  help = new fltk::HelpDialog;

  if (argc < 2)
    help->load("../documentation/index.html");
  else
    help->load(argv[1]);

  help->show();

  fltk::run();

  delete help;

  return (0);
}


//
// End of "$Id: help.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
