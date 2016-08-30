#ifndef Fl_Tile_H
#define Fl_Tile_H

#include "Fl_Widget.H"
#include <fltk/TiledGroup.h>

class Fl_Tile : public fltk::TiledGroup {

public:
  Fl_Tile(int x, int y, int w, int h, const char*l = 0)
    : fltk::TiledGroup(x,y,w,h,l) {begin();}
};

#endif

