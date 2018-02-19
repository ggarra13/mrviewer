#ifndef fl_filename_H
#define fl_filename_H

#include <fltk/filename.h>
#include <string.h>

#define fl_filename_name	fltk::filename_name
#define fl_filename_ext		fltk::filename_ext
#define fl_filename_match	fltk::filename_match
#define fl_filename_isdir	fltk::filename_isdir
#define fl_filename_list	fltk::filename_list

inline char *fl_filename_setext(char * name, const char *ext) {
  strcpy(fltk::filename_ext(name), ext ? ext : "");
  return name;
}
#define FL_PATH_MAX PATH_MAX // all buffers are assummed to be at least this long
FL_API bool fl_filename_expand(char *, const char *from);
FL_API bool fl_filename_absolute(char *, const char *from, const char* pwd=0);

#endif
