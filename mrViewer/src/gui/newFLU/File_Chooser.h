// generated by Fast Light User Interface Designer (fluid) version 1,0400

#ifndef File_Chooser_h
#define File_Chooser_h
#include <string>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>

#include <gui/MyPack.h>

extern Fl_Group *uiPaths;
extern MyPack *uiThumbnails;
Fl_Double_Window* make_window();
typedef std::vector< std::string > FluStringVector;
const char* _newflu_file_chooser( const char *message, const char *pattern, const char *filename, int type, FluStringVector& filelist, const bool compact_files = true );
#endif
