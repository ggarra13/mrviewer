#ifndef Fl_File_Chooser_H
#define Fl_File_Chooser_H

#include "Fl_Widget.H"

#include <fltk/FileChooser.h>
typedef fltk::FileChooser Fl_File_Chooser;

#include <fltk/file_chooser.h>

//char *fl_dir_chooser(const char *message,const char *fname,int relative=0);
inline const char *fl_file_chooser(const char *message,const char *pat,const char *fname,int /*relative*/ = 0) {
  return fltk::file_chooser(message, pat, fname);
}

#endif
