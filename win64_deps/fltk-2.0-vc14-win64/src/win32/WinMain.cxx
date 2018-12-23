/*
 * "$Id: WinMain.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
 *
 * Copyright 1998-2006 by Bill Spitzak and others.
 *
 * fl_call_main() calls main() for you Windows people.  Needs to be done in C
 * because Borland C++ won't let you call main() from C++.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to the following page:
 *
 *    http://www.fltk.org/str.php
 */

/*
 * This WinMain() function can be overridden by an application and
 * is provided for compatibility with programs written for other
 * operating systems that conform to the ANSI standard entry point
 * "main()".  This will allow you to build a _WIN32 Application
 * without any special settings.
 *
 * Because of problems with the Microsoft Visual C++ header files
 * and/or compiler, you cannot have a WinMain function in a DLL.
 * I don't know why.  Thus, this nifty feature is only available
 * if you link to the static library. You may want to compile this
 * source file to a .obj and link it with your program.
 *
 * Currently the debug version of this library will create a
 * console window for your application so you can put printf()
 * statements for debugging or informational purposes.  Ultimately
 * we want to update this to always use the parent's console,
 * but at present we have not identified a function or API in
 * Microsoft(r) Windows(r) that allows for it.
 */

#if defined(_WIN32) && !defined(FL_LIBRARY) && !defined (__GNUC__) 

#include <windows.h>
#include <stdio.h>
// ignore deprecated freopen warning in vc2005:
#pragma warning(disable: 4996)
extern int main(int, char *[]);
#ifdef BORLAND5 // FIXME : or __BORLANDC__ ??
# define __argc _argc
# define __argv _argv
#endif
#if defined(_MSC_VER) && defined(_MSC_DLL)
	int __argc = 1;
	static char *args[2] = { "", NULL };
	char **__argv = &args[0];
#else
# include <stdlib.h>
//extern int  __argc;
//extern char **__argv;
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                             LPSTR lpCmdLine, int nCmdShow) {
#ifdef _DEBUG
  // If we are using compiling in debug mode, open a console window so
  // we can see any printf's, etc...
  //
  // While we can detect if the program was run from the command-line -
  // look at the CMDLINE environment variable, it will be "WIN" for
  // programs started from the GUI - the shell seems to run all _WIN32
  // applications in the background anyways...

  AllocConsole();
  freopen("conin$", "r", stdin);
  freopen("conout$", "w", stdout);
  freopen("conout$", "w", stderr);
#endif /* _DEBUG */

  /* Run the standard main entry point function... */
  return main(__argc, __argv);
}

#endif /* _WIN32 && !FL_LIBRARY && !__GNUC__ */

/*
 * End of "$Id: WinMain.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
 */

