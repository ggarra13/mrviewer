//
// "$Id: tile.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Tile test program for the Fast Light Tool Kit (FLTK).
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

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/TiledGroup.h>
#include <fltk/Button.h>

int main(int argc, char** argv) {
  fltk::Window window(320,320);
  window.begin();
  window.resizable(window);

  fltk::TiledGroup tile(10,10,300,300);
  tile.begin();

  fltk::Group box0(0,0,150,150,"0");
  box0.box(fltk::DOWN_BOX);
  box0.color(9);
  box0.labelsize(36);
  box0.set_flag(fltk::ALIGN_CLIP);

  box0.begin();
  fltk::Button but(20, 20, 100, 30, "Button");
  but.set_flag(fltk::ALIGN_CLIP);
  box0.end();

  fltk::Window w1(150,0,150,150,"1");
  w1.begin();
  fltk::Widget box1(0,0,150,150,"1\nThis is a\nchild\nX window");
  box1.box(fltk::DOWN_BOX);
  box1.color(19);
  box1.labelsize(18);
  box1.set_flag(fltk::ALIGN_CLIP);
  w1.resizable(box1);
  w1.end();

  //  fltk::Tile tile2(0,150,150,150);
  fltk::Widget box2a(0,150,70,150,"2a");
  box2a.box(fltk::DOWN_BOX);
  box2a.color(12);
  box2a.labelsize(36);
  box2a.set_flag(fltk::ALIGN_CLIP);
  fltk::Widget box2b(70,150,80,150,"2b");
  box2b.box(fltk::DOWN_BOX);
  box2b.color(13);
  box2b.labelsize(36);
  box2b.set_flag(fltk::ALIGN_CLIP);
  //tile2.end();

  //Tile tile3(150,150,150,150);
  fltk::Widget box3a(150,150,150,70,"3a");
  box3a.box(fltk::DOWN_BOX);
  box3a.color(12);
  box3a.labelsize(36);
  box3a.set_flag(fltk::ALIGN_CLIP);
  fltk::Widget box3b(150,150+70,150,80,"3b");
  box3b.box(fltk::DOWN_BOX);
  box3b.color(13);
  box3b.labelsize(36);
  box3b.set_flag(fltk::ALIGN_CLIP);
  //tile3.end();
  
  fltk::Widget r(10,0,300-10,300-10);
  //r.box(fltk::BORDER_FRAME);
  r.hide();
  tile.resizable(r);

  tile.end();
  window.end();
  window.show(argc,argv);
  w1.show();
  return fltk::run();
}

//
// End of "$Id: tile.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
