//
// "$Id: output.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Output test program for the Fast Light Tool Kit (FLTK).
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
#include <fltk/ValueInput.h> // necessary for bug in mingw32?
#include <fltk/Window.h>
#include <fltk/Box.h>
#include <fltk/ToggleButton.h>
#include <fltk/Input.h>
#include <fltk/draw.h>
#include <fltk/Output.h>
#include <fltk/MultiLineOutput.h>
#include <fltk/ValueSlider.h>
#include <fltk/Font.h>

using namespace fltk;

Output *text;
MultiLineOutput *text2;
Input *input;
ValueSlider *fonts;
ValueSlider *sizes;
Window *window;

void font_cb(Widget *,void *) {
  text->textfont(font(int(fonts->value())));
  text->redraw();
  text2->textfont(font(int(fonts->value())));
  text2->redraw();
}

void size_cb(Widget *,void *) {
  text->textsize(int(sizes->value()));
  text->redraw();
  text2->textsize(int(sizes->value()));
  text2->redraw();
}

void input_cb(Widget *,void *) {
  text->value(input->value());
  text2->value(input->value());
}

static const char* const initial_value =
"The quick brown fox\njumps over\nthe lazy dog.";

int main(int argc, char **argv) {
  window = new Window(400,400);
  window->begin();
  text = new Output(100,90,200,30,"Output");
  text->value(initial_value);
  text->clear_flag(fltk::ALIGN_MASK);
  text->set_flag(fltk::ALIGN_BOTTOM);

  text2 = new MultiLineOutput(100,150,200,100,"MultiLineOutput");
  text2->value(initial_value);
  text2->clear_flag(fltk::ALIGN_MASK);
  text2->set_flag(fltk::ALIGN_BOTTOM);
  window->resizable(text2);

  fonts = new ValueSlider(50,325,350,25,"Font");
  fonts->clear_flag(fltk::ALIGN_MASK);
  fonts->set_flag(fltk::ALIGN_LEFT);
  fonts->range(0,15);
  fonts->step(1);
  fonts->value(0);
  fonts->callback(font_cb);

  sizes = new ValueSlider(50,350,350,25,"Size");
  sizes->clear_flag(fltk::ALIGN_MASK);
  sizes->set_flag(fltk::ALIGN_LEFT);
  sizes->range(1,64);
  sizes->step(1);
  sizes->value(14);
  sizes->callback(size_cb);

  input = new Input(50,375,350,25);
  input->static_value(initial_value);
  input->when(fltk::WHEN_CHANGED);
  input->callback(input_cb);

  window->end();
  window->show(argc,argv);
  return fltk::run();
}

//
// End of "$Id: output.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
