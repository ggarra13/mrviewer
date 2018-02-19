//
// "$Id: boxtype.cxx 5810 2007-05-11 22:44:12Z spitzak $"
//
// Boxtype test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
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
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

#include <stdlib.h>
#include <stdio.h>
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/InvisibleBox.h>

int N = 0;
#define W 150
#define H 50
#define ROWS 6

fltk::Widget* bt(const char *name, fltk::Box* type, int square=0) {
    int x = N%4;
    int y = N/4;
    N++;
    x = x*W+10;
    y = y*H+10;
    fltk::Widget *b = new fltk::InvisibleBox(type,x,y,square ? H-20 : W-20,H-20,name);
    b->labelsize(11);
    if (square) {
	b->clear_flag(fltk::ALIGN_MASK);
	b->set_flag(fltk::ALIGN_RIGHT);
    }
    return b;
}

int main(int argc, char ** argv) {
    fltk::Window window(4*W,ROWS*H);
    window.color(12);// light blue
    window.begin();
    bt("fltk::NO_BOX",fltk::NO_BOX);
    bt("fltk::FLAT_BOX",fltk::FLAT_BOX);
    //  N += 2; // go to start of next row to line up boxes & frames
    bt("fltk::UP_BOX",fltk::UP_BOX);
    bt("fltk::DOWN_BOX",fltk::DOWN_BOX);
    //  bt("fltk::UP_FRAME",fltk::UP_FRAME);
    //  bt("fltk::DOWN_FRAME",fltk::DOWN_FRAME);
    bt("fltk::THIN_UP_BOX",fltk::THIN_UP_BOX);
    bt("fltk::THIN_DOWN_BOX",fltk::THIN_DOWN_BOX);
    //  bt("fltk::THIN_UP_FRAME",fltk::THIN_UP_FRAME);
    //  bt("fltk::THIN_DOWN_FRAME",fltk::THIN_DOWN_FRAME);
    bt("fltk::ENGRAVED_BOX",fltk::ENGRAVED_BOX);
    bt("fltk::EMBOSSED_BOX",fltk::EMBOSSED_BOX);
    //  bt("fltk::ENGRAVED_FRAME",fltk::ENGRAVED_FRAME);
    //  bt("fltk::EMBOSSED_FRAME",fltk::EMBOSSED_FRAME);
    bt("fltk::ROUND_UP_BOX",fltk::ROUND_UP_BOX);
    bt("fltk::ROUND_DOWN_BOX",fltk::ROUND_DOWN_BOX);
    bt("fltk::DIAMOND_UP_BOX",fltk::DIAMOND_UP_BOX);
    bt("fltk::DIAMOND_DOWN_BOX",fltk::DIAMOND_DOWN_BOX);
    //  bt("fltk::BORDER_FRAME",fltk::BORDER_FRAME);
    //  bt("fltk::SHADOW_FRAME",fltk::SHADOW_FRAME);
    bt("fltk::BORDER_BOX",fltk::BORDER_BOX);
    bt("fltk::ROUNDED_BOX",fltk::ROUNDED_BOX);
    bt("fltk::RSHADOW_BOX",fltk::RSHADOW_BOX);
    //  bt("fltk::ROUNDED_FRAME",fltk::ROUNDED_FRAME);
    bt("fltk::RFLAT_BOX",fltk::RFLAT_BOX);
    bt("fltk::SHADOW_BOX",fltk::SHADOW_BOX);
    bt("fltk::OVAL_BOX",fltk::OVAL_BOX);
    bt("fltk::OSHADOW_BOX",fltk::OSHADOW_BOX);
    //  bt("fltk::OVAL_FRAME",fltk::OVAL_FRAME);
    bt("fltk::OFLAT_BOX",fltk::OFLAT_BOX);
    //    bt("fltk::PLASTIC_UP_BOX", fltk::PLASTIC_UP_BOX);
    //    bt("fltk::PLASTIC_DOWN_BOX", fltk::PLASTIC_DOWN_BOX);
    //    bt("fltk::FOCUS_FRAME", fltk::FOCUS_FRAME);
    bt("fltk::BORDER_FRAME", fltk::BORDER_FRAME);
    bt("fltk::PLASTIC_UP_BOX", fltk::PLASTIC_UP_BOX)->color(12);
    bt("fltk::PLASTIC_DOWN_BOX", fltk::PLASTIC_DOWN_BOX)->color(12);
    window.resizable(window);
    window.end();
    window.show(argc,argv);
    return fltk::run();
}

//
// End of "$Id: boxtype.cxx 5810 2007-05-11 22:44:12Z spitzak $".
//
