//
// "$Id: MenuType.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Menu item code for the Fast Light Tool Kit (FLTK).
//
// Vastly simplified from the 1.0 version, since 2.0 uses real child
// widgets to represent menu items.
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

#include "Enumeration.h"
#include "Widget_Types.h"
#include <fltk/events.h>
#include <fltk/ask.h>
#include <fltk/Menu.h>
#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/Divider.h>
#include <fltk/Button.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const Enumeration item_type_menu[] = {
  {"Normal", "NORMAL", (void*)0},
  {"Toggle", "TOGGLE", (void*)fltk::Item::TOGGLE},
  {"Radio",  "RADIO",  (void*)fltk::Item::RADIO},
  {0}};

extern int reading_file;

using namespace fltk;

Widget *ItemType::widget(int,int,int,int) {
  return new fltk::Item(reading_file ? 0 : "item");
}

Widget *SubmenuType::widget(int,int,int,int) {
  fltk::ItemGroup *g = new fltk::ItemGroup(reading_file ? 0 : "submenu");
  fltk::Group::current(0);
  return g;
}

////////////////////////////////////////////////////////////////

// This is the base class for widgets that contain a menu (ie
// subclasses of fltk::Menu).

FluidType* MenuType::click_test(int, int) {
  if (selected()) return 0; // let user move the widget
  fltk::Menu* w = (fltk::Menu*)o;
  if (!w->size()) return 0;
  Widget* save = w->item();
  w->item(0);
  // disable menu items callbacks so we can do selection
  w->when(WHEN_NEVER);
  fltk::pushed(w);
  w->handle(fltk::PUSH);
  // enable them again
  w->when(WHEN_RELEASE);
  const Widget* m = w->item();
  if (m) return (FluidType*)(m->user_data());
  w->item(save);
  return this;
}

////////////////////////////////////////////////////////////////

#include <fltk/PopupMenu.h>
const Enumeration button_type_menu[] = {
  {"normal", 0,		(void*)0},
  {"popup1", "POPUP1",	(void*)fltk::PopupMenu::POPUP1},
  {"popup2", "POPUP2",	(void*)fltk::PopupMenu::POPUP2},
  {"popup3", "POPUP3",	(void*)fltk::PopupMenu::POPUP3},
  {"popup12","POPUP12", (void*)fltk::PopupMenu::POPUP12},
  {"popup23","POPUP23", (void*)fltk::PopupMenu::POPUP23},
  {"popup13","POPUP13", (void*)fltk::PopupMenu::POPUP13},
  {"popup123","POPUP123",(void*)fltk::PopupMenu::POPUP123},
  {0}};

////////////////////////////////////////////////////////////////

const Enumeration input_browser_type_menu[] = {
  {"Normal",		0,	(void*)fltk::InputBrowser::NORMAL},
  {"Non-Editable","NONEDITABLE",(void*)fltk::InputBrowser::NONEDITABLE},
  {"Indented",	"INDENTED",	(void*)fltk::InputBrowser::INDENTED},
  {"Non-Editable Indented","NONEDITABLE_INDENTED",(void*)fltk::InputBrowser::NONEDITABLE_INDENTED},
  {0}};

#include <fltk/Browser.h>
const Enumeration browser_type_menu[] = {
  {"Single",	0,	(void*)fltk::Browser::NORMAL},
  {"Multi",	"MULTI",	(void*)fltk::Browser::MULTI, "fltk::MultiBrowser"},
  {0}};

#include <fltk/FileBrowser.h>

////////////////////////////////////////////////////////////////
// Shortcut entry item in panel:

#include <fltk/Output.h>
#include "Shortcut_Button.h"
#include <fltk/draw.h>

void Shortcut_Button::draw() {
  label(fltk::key_name(svalue));
#ifdef _WIN32
  Button::draw();
#else
  fltk::Button::draw();
#endif
}

int Shortcut_Button::handle(int e) {
  when(0); type(TOGGLE);
  if (e == fltk::KEY) {
    if (!value()) return 0;
    unsigned v = fltk::event_text()[0];
    if ((v > 32 && v < 0x7f) || (v > 0xa0 && v <= 0xff)) {
      v = v | (fltk::event_state()&(fltk::META|fltk::ALT|fltk::CTRL));
    } else {
      v = (fltk::event_state()&(fltk::META|fltk::ALT|fltk::CTRL|fltk::SHIFT)) | fltk::event_key();
      if (v == fltk::BackSpaceKey && svalue) v = 0;
    }
    if (v != svalue) {svalue = v; do_callback(); redraw();}
    return 1;
  } else if (e == fltk::UNFOCUS) {
    int c = changed(); value(0); if (c) set_changed();
    return 1;
  } else if (e == fltk::FOCUS) {
    return value();
  } else {
#ifdef _WIN32
    int r = Button::handle(e);
#else
    int r = fltk::Button::handle(e);
#endif
    if (e == fltk::RELEASE && value() && fltk::focus() != this) take_focus();
    return r;
  }
}
  
#define NOT_DEFAULT(W, what) (W->o->what() != ((WidgetType*)(W->factory))->o->what())

void shortcut_in_cb(Shortcut_Button* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_button() &&
	!((WidgetType*)(current_widget->factory))->o->shortcut()) {i->hide(); return;}
    i->show();
    i->svalue = ((current_widget->o))->shortcut();
    i->redraw();
  } else {
    for (FluidType *o = FluidType::first; o; o = o->walk())
      if (o->selected() && o->is_widget()) {
	Widget* b = ((WidgetType*)o)->o;
	if (b->shortcut() != i->svalue) modflag = 1;
	b->shortcut(i->svalue);
	if (o->is_menu_item()) ((WidgetType*)o)->redraw();
      }
  }
  fltk::Color tc = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, shortcut)) tc = fltk::RED;
  if (i->labelcolor() != tc)
    { i->labelcolor(tc); i->redraw_label(); }
}

//
// End of "$Id: MenuType.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
