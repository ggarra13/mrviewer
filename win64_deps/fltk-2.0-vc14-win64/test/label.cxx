//
// "$Id: label.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Label test program for the Fast Light Tool Kit (FLTK).
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
#include <fltk/Box.h>
#include <fltk/ValueSlider.h>
#include <fltk/ToggleButton.h>
#include <fltk/Input.h>
#include <fltk/MenuBuild.h>
#include <fltk/Choice.h>
#include <fltk/draw.h>
#include <fltk/Font.h>
#include <fltk/xpmImage.h>

using namespace fltk;

ToggleButton *leftb,*rightb,*topb,*bottomb,*insideb,*centerb,*clipb,*wrapb;
Widget *textbox;
Input *input;
ValueSlider *fontslider;
ValueSlider *sizes;
Window *window;

void button_cb(Widget *,void *) {
  int i = 0;
  if (leftb->value()) i |= ALIGN_LEFT;
  if (rightb->value()) i |= ALIGN_RIGHT;
  if (topb->value()) i |= ALIGN_TOP;
  if (bottomb->value()) i |= ALIGN_BOTTOM;
  if (insideb->value()) i |= ALIGN_INSIDE;
  if (centerb->value()) i |= ALIGN_CENTER;
  if (clipb->value()) i |= ALIGN_CLIP;
  if (wrapb->value()) i |= ALIGN_WRAP;
  textbox->align(i);
  window->redraw();
}

void font_cb(Widget *,void *) {
  //Widget::default_style->leading_ = int(fontslider->value());
  textbox->labelfont(font(int(fontslider->value())));
  window->redraw();
}

void size_cb(Widget *,void *) {
  textbox->labelsize((float)sizes->value());
  window->redraw();
}

void input_cb(Widget *,void *) {
  textbox->label(input->value());
  window->redraw();
}

void normal_cb(Widget *,void *) {
  textbox->labeltype(NORMAL_LABEL);
  window->redraw();
}

void symbol_cb(Widget *,void *) {
  textbox->labeltype(SYMBOL_LABEL);
  if (input->value()[0] != '@') {
    input->static_value("@->");
    textbox->label("@->");
  }
  window->redraw();
}

void shadow_cb(Widget *,void *) {
  textbox->labeltype(SHADOW_LABEL);
  window->redraw();
}

void embossed_cb(Widget *,void *) {
  textbox->labeltype(EMBOSSED_LABEL);
  window->redraw();
}

void engraved_cb(Widget *,void *) {
  textbox->labeltype(ENGRAVED_LABEL);
  window->redraw();
}

static void load_menu(Choice* c) {
  c->begin();
      new fltk::Item("NORMAL_LABEL",0,normal_cb);
      new fltk::Item("SYMBOL_LABEL",0,symbol_cb);
      new fltk::Item("SHADOW_LABEL",0,shadow_cb);
      new fltk::Item("ENGRAVED_LABEL",0,engraved_cb);
      new fltk::Item("EMBOSSED_LABEL",0,embossed_cb);
  c->end();
}

#include "porsche.xpm"
xpmImage theimage(porsche_xpm);
void image_cb(Widget* w, void*) {
  textbox->image(((Button*)w)->value() ? &theimage : 0);
  window->redraw();
}

void inactive_cb(Widget* w, void*) {
  if (((Button*)w)->value()) textbox->deactivate(); else textbox->activate();
  window->redraw();
}

const char* const initial = "The quick brown fox jumps over the lazy dog.";

int main(int argc, char **argv) {
  window = new Window(400,400);
  window->set_double_buffer();
  window->begin();

  textbox= new Widget(100,75,200,100,initial);
  textbox->box(ENGRAVED_BOX);

  Choice *c = new Choice(50,275,200,25);
  load_menu(c);

  ToggleButton* b = new ToggleButton(250,275,50,25,"image");
  b->callback(image_cb);

  b = new ToggleButton(300,275,50,25,"inactive");
  b->callback(inactive_cb);

  Group *g = new Group(0,300,400,25);
  g->resizable(g);
  g->begin();
  leftb = new ToggleButton(0,0,50,25,"left");
  leftb->callback(button_cb);
  rightb = new ToggleButton(50,0,50,25,"right");
  rightb->callback(button_cb);
  topb = new ToggleButton(100,0,50,25,"top");
  topb->callback(button_cb);
  bottomb = new ToggleButton(150,0,50,25,"bottom");
  bottomb->callback(button_cb);
  insideb = new ToggleButton(200,0,50,25,"inside");
  insideb->callback(button_cb);
  centerb = new ToggleButton(250,0,50,25,"center");
  centerb->callback(button_cb);
  wrapb = new ToggleButton(300,0,50,25,"wrap");
  wrapb->callback(button_cb);
  clipb = new ToggleButton(350,0,50,25,"clip");
  clipb->callback(button_cb);
  g->end();

  fontslider=new ValueSlider(50,325,350,25,"Font:");
  fontslider->type(Slider::TICK_ABOVE);
  fontslider->clear_flag(ALIGN_MASK);
  fontslider->set_flag(ALIGN_LEFT);
  fontslider->range(0,15);
  fontslider->step(1);
  fontslider->value(0);
  fontslider->callback(font_cb);

  sizes= new ValueSlider(50,350,350,25,"Size:");
  sizes->type(Slider::TICK_ABOVE|Slider::LOG);
  sizes->clear_flag(ALIGN_MASK);
  sizes->set_flag(ALIGN_LEFT);
  sizes->range(1,64);
  //sizes->step(1);
  sizes->value(14);
  sizes->callback(size_cb);

  input = new Input(50,375,350,25);
  input->static_value(initial);
  input->when(WHEN_CHANGED);
  input->callback(input_cb);

  window->resizable(textbox);
  window->end();
  window->show(argc,argv);
  return run();
}

//
// End of "$Id: label.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
