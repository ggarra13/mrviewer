//
// "$Id: essai.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Theme plugin file for FLTK
//
// Copyright 1999 Bill Spitzak and others.
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

// essai.cxx

// Demonstration of a plugin that replaces the boxes such that a pixmap
// is used to draw everything.

#include <fltk/Widget.h>
#include <fltk/Window.h>
#include <fltk/draw.h>
#include <fltk/Style.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fltk/Box.h>
#include <fltk/SharedImage.h>
#include <fltk/TiledImage.h>

class fltk::Image_Box : public fltk::Boxtype_ {
  fltk::Flags mask;
public:
  void draw(const int,int,int,int, fltk::Color, fltk::Flags) const;
  fltk::Tiled_Image* normal_img;
  fltk::Tiled_Image* down_img;
  fltk::Tiled_Image* highlight_img;
  fltk::Image_Box(const char*, const char*, const char*, fltk::Flags = 0);
};

void fltk::Image_Box::draw(int x, int y, int w, int h,
			fltk::Color color, fltk::Flags flags) const
{
  fltk::Tiled_Image* img;

  if (flags&fltk::VALUE) img = down_img;
  else if (flags&fltk::HIGHLIGHT) img = highlight_img;
  else img = normal_img;

  fltk::up_box.draw(x,y,w,h,0,(flags|fltk::INACTIVE)&(~mask));
  if (!(flags&fltk::INACTIVE)) {
    fltk::up_box.inset(x,y,w,h);
    img->draw(x,y,w,h);
  }
}

fltk::Image_Box::fltk::Image_Box(const char* normal_b, const char* down_b, const char* highlight_b, fltk::Flags m) :
fltk::Boxtype_(0), mask(m) {
  dx_ = fltk::up_box.dx();
  dy_ = fltk::up_box.dy();
  dw_ = fltk::up_box.dw();
  dh_ = fltk::up_box.dh();
  fills_rectangle_ = true;
  char buffer[256];
  normal_img =
    new fltk::Tiled_Image(fltk::JPEG_Image::get(fltk::find_config_file(buffer,256,normal_b)));
  down_img =
    new fltk::Tiled_Image(fltk::JPEG_Image::get(fltk::find_config_file(buffer,256,down_b)));
  highlight_img =
    new fltk::Tiled_Image(fltk::JPEG_Image::get(fltk::find_config_file(buffer,256,highlight_b)));
}

class fltk::Image_NoBorderBox : public fltk::Image_Box {
  void draw(int,int,int,int, fltk::Color, fltk::Flags) const;
public:
  fltk::Image_NoBorderBox(const char*a, const char*b, const char*c, fltk::Flags m = 0) : fltk::Image_Box(a,b,c,m) {dx_=dy_=dw_=dh_=0;}
};

void fltk::Image_NoBorderBox::draw(int x, int y, int w, int h,
                                fltk::Color color, fltk::Flags flags) const
{
  if (flags&(fltk::SELECTED|fltk::HIGHLIGHT)) {
    fltk::Image_Box::draw(x,y,w,h, color, flags);
    return;
  }
  if (!(flags&fltk::INACTIVE)) {
    normal_img->draw(x, y, w, h);
  }
}

extern "C" bool fltk_theme()
{

  //  fltk::background(0xD0D0E000); // it would be nice to figure out color from image
  fltk::Boxtype flat1 = new fltk::Image_NoBorderBox("bg.jpeg", "bg2.jpeg", "bg3.jpeg");
  fltk::Boxtype flat2 = new fltk::Image_NoBorderBox("bg2.jpeg", "bg3.jpeg", "bg3.jpeg", fltk::VALUE);
  fltk::Boxtype box1 = new fltk::Image_Box("bg2.jpeg", "bg3.jpeg", "bg3.jpeg");
  //fltk::Boxtype box2 = new fltk::Image_Box("bg.jpeg", "bg.jpeg", "bg.jpeg");
  fltk::Boxtype box3 = new fltk::Image_Box("bg2.jpeg", "bg3.jpeg", "bg3.jpeg", fltk::VALUE);
  fltk::Widget::default_style->box = box1;
  fltk::Widget::default_style->button_box = box3;
  fltk::Widget::default_style->highlight_color = fltk::LIGHT2;
  fltk::Style* s;
  if ((s = fltk::Style::find("window"))) {
    s->box = flat1;
  }
  if ((s = fltk::Style::find("group"))) {
    s->box = fltk::NO_BOX;
  }
  if ((s = fltk::Style::find("menu"))) {
    s->selection_text_color = fltk::BLACK;
    s->box = flat1;
    s->button_box = flat2;
  }
  if ((s = fltk::Style::find("menu bar"))) {
    s->highlight_color = fltk::GRAY;
    s->highlight_label_color = fltk::BLACK;
    s->box = flat2;
    s->selection_text_color = fltk::BLACK;
    s->button_box = flat2;
  }
  if ((s = fltk::Style::find("highlight button"))) {
    s->box = flat2;
  }
  if ((s = fltk::Style::find("button"))) {
    s->selection_text_color = fltk::BLACK;
    s->box = box3;
  }
  if ((s = fltk::Style::find("tabs"))) {
    s->box = box1;
  }
  if ((s = fltk::Style::find("light button"))) {
    s->box = box3;
  }
  return true;
}

//
// End of "$Id: essai.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
