//
// " $Id: file_chooser.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// File chooser test program for the Fast Light Tool Kit (FLTK).
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
//
// Contents:
//
//   main()           - Create a file chooser and wait for a selection to
//                      be made.
//   close_callback() - Close the main window...
//   fc_callback()    - Handle choices in the file chooser...
//   pdf_check()      - Check for and load the first page of a PDF file.
//   ps_check()       - Check for and load the first page of a PostScript
//                      file.
//   show_callback()  - Show the file chooser...
//

//
// Include necessary headers...
//

#include <stdio.h>
#include <stdlib.h>
#include <fltk/FileChooser.h>
#include <fltk/file_chooser.h>
#include <fltk/FileIcon.h>
#include <fltk/SharedImage.h>
#include <fltk/pnmImage.h>
//#include <fltk/PNMImage.h>
#include <fltk/LightButton.h>
#include <fltk/run.h>
#include <string.h>
#include <fltk/ask.h>

using namespace fltk;

//
// Globals...
//

Input		*filter;
FileBrowser		*files;
FileChooser		*fc;
SharedImage		*image = 0;

//
// Functions...
//

void		close_callback(void);
void		create_callback(void);
void		dir_callback(void);
void		fc_callback(FileChooser *, void *);
void		multi_callback(void);
SharedImage	*pdf_check(const char *, uchar *, int);
SharedImage	*ps_check(const char *, uchar *, int);
void		show_callback(void);

//
// 'close_callback()' - Close the main window...
//

void
close_callback(void)
{
  exit(0);
}


//
// 'create_callback()' - Handle clicks on the create button.
//

void
create_callback(void)
{
  fc->type(fc->type() ^ FileChooser::CREATE);
}


//
// 'dir_callback()' - Handle clicks on the directory button.
//

void
dir_callback(void)
{
  fc->type(fc->type() ^ FileChooser::DIRECTORY);
}


//
// 'fc_callback()' - Handle choices in the file chooser...
//

void
fc_callback(FileChooser *fc,	// I - File chooser
            void            *data)	// I - Data
{
  const char		*filename;	// Current filename


  printf("fc_callback(fc = %p, data = %p)\n", fc, data);

  filename = fc->value();

  printf("    filename = \"%s\"\n", filename ? filename : "(null)");
}


//
// 'multi_callback()' - Handle clicks on the multi button.
//

void
multi_callback(void)
{
  fc->type(fc->type() ^ FileChooser::MULTI);
}


//
// 'pdf_check()' - Check for and load the first page of a PDF file.
//

SharedImage *			// O - Page image or NULL
pdf_check(const char *name,	// I - Name of file
          uchar      *header,	// I - Header data
	  int        headerlen)	// I - Length of header data
{
  const char	*home;		// Home directory
  char		preview[1024],	// Preview filename
		command[1024];	// Command


  if (memcmp(header, "%PDF", 4) != 0)
    return 0;

  home = getenv("HOME");
  sprintf(preview, "%s/.preview.ppm", home ? home : "");

  sprintf(command,
          "gs -r100 -dFIXED -sDEVICE=ppmraw -dQUIET -dNOPAUSE -dBATCH "
	  "-sstdout=\"%%stderr\" -sOUTPUTFILE=\'%s\' "
	  "-dFirstPage=1 -dLastPage=1 \'%s\' 2>/dev/null", preview, name);

  if (system(command)) return 0;

  return new pnmImage(preview);
}


//
// 'ps_check()' - Check for and load the first page of a PostScript file.
//

SharedImage *			// O - Page image or NULL
ps_check(const char *name,	// I - Name of file
         uchar      *header,	// I - Header data
	 int        headerlen)	// I - Length of header data
{
  const char	*home;		// Home directory
  char		preview[1024],	// Preview filename
		outname[1024],	// Preview PS file
		command[1024];	// Command
  FILE		*in,		// Input file
		*out;		// Output file
  int		page;		// Current page
  char		line[256];	// Line from file


  if (memcmp(header, "%!", 2) != 0)
    return 0;

  home = getenv("HOME");
  sprintf(preview, "%s/.preview.ppm", home ? home : "");

  if (memcmp(header, "%!PS", 4) == 0) {
    // PS file has DSC comments; extract the first page...
    sprintf(outname, "%s/.preview.ps", home ? home : "");

    if (strcmp(name, outname) != 0) {
      in   = fopen(name, "rb");
      out  = fopen(outname, "wb");
      page = 0;

      while (fgets(line, sizeof(line), in) != NULL) {
	if (strncmp(line, "%%Page:", 7) == 0) {
          page ++;
	  if (page > 1) break;
	}

	fputs(line, out);
      }

      fclose(in);
      fclose(out);
    }
  } else {
    // PS file doesn't have DSC comments; do the whole file...
    strncpy(outname, name, sizeof(outname) - 1);
    outname[sizeof(outname) - 1] = '\0';
  }

  sprintf(command,
          "gs -r100 -dFIXED -sDEVICE=ppmraw -dQUIET -dNOPAUSE -dBATCH "
	  "-sstdout=\"%%stderr\" -sOUTPUTFILE=\'%s\' \'%s\' 2>/dev/null",
	  preview, outname);

  if (system(command)) return 0;

  return new pnmImage(preview);
}


//
// 'show_callback()' - Show the file chooser...
//

void
show_callback(void)
{
  int	i;			// Looping var
  int	count;			// Number of files selected
  char	relative[1024];		// Relative filename


  if (filter->value()[0])
    fc->filter(filter->value());

  fc->show(240,200);

  while (fc->visible())
    fltk::wait();

  count = fc->count();
  if (count > 0)
  {
    files->clear();

    for (i = 1; i <= count; i ++)
    {
      if (!fc->value(i))
        break;

      fltk::filename_relative(relative, sizeof(relative), fc->value(i));

      files->add(relative,
                 FileIcon::find(fc->value(i), FileIcon::PLAIN));
    }

    files->redraw();
  }
}

//
// 'main()' - Create a file chooser and wait for a selection to be made.
//
#define SIMPLE_IMPL 0
int			// O - Exit status
main(int  argc,		// I - Number of command-line arguments
     char *argv[])	// I - Command-line arguments
{
  // Make the file chooser...
  //fltk::scheme(NULL);

  // if you decide not to link with images lib you must comment this:
  FileIcon::load_system_icons();

#if !SIMPLE_IMPL
  Window	*window;// Main window
  Button	*button;// Buttons
  FileIcon	*icon;	// New file icon

  fc = new FileChooser(".", "*", FileChooser::SINGLE, "FileChooser Test");
  fc->callback(fc_callback);

  // Register the PS and PDF image types...
  SharedImage::add_handler(pdf_check);
  SharedImage::add_handler(ps_check);

  // Make the main window...
  window = new Window(400, 200, "File Chooser Test");
  window->begin();

  filter = new Input(50, 10, 315, 25, "Filter:");
  if (argc > 1)
    filter->value(argv[1]);
  else
    filter->value(
		  "Image Files (*.{bmp,gif,jpg,png})\t"
		  "PDF Files (*.pdf)\t"
                  "PostScript Files (*.ps)\t"
		  "C/C++ Source Files (*.{c,C,cc,cpp,cxx,h,H})"
		  );

  button = new Button(365, 10, 25, 25);
  button->labelcolor(fltk::YELLOW);
  button->callback((Callback *)show_callback);

#define TEST_FETCH_PNG 0
#if TEST_FETCH_PNG
  icon = new FileIcon("butt", FileIcon::DIRECTORY);
  icon->load_image("./images/folder.png");
#else
  icon   = new FileIcon(*FileIcon::find(".", FileIcon::DIRECTORY));
#endif
  icon->value(button);

  button = new LightButton(50, 45, 80, 25, "MULTI");
  button->callback((Callback*)multi_callback);

  button = new LightButton(140, 45, 90, 25, "CREATE");
  button->callback((Callback*)create_callback);

  button = new LightButton(240, 45, 115, 25, "DIRECTORY");
  button->callback((Callback*)dir_callback);

  files = new FileBrowser(50, 80, 340, 75, "Files:");
  files->align(fltk::ALIGN_LEFT);

  button = new Button(340, 165, 50, 25, "Close");
  button->callback((Callback*)close_callback);
  window->resizable(files);
  window->end();
  window->show(1, argv);
  fltk::run();

#else
  const char *defaultname = "images/testimg.jpg";
  fltk::use_system_file_chooser(true);
  const char * name = fltk::file_chooser("Open","Image Files (*.{bmp,gif,jpg,png})\t", defaultname);
  fltk::message("File selected is : %s", name && strlen(name) ? name : "<no_selection>");
#endif

  return (0);
}

//
// End of "$Id: file_chooser.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
