//
// "$Id: menubar.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Menubar test program for the Fast Light Tool Kit (FLTK).
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

// Use compat header for Fl_Menu_Item
#include <FL/Fl_Menu_Item.H>

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/Output.h>
#include <fltk/Box.h>
#include <fltk/Window.h>
#include <fltk/MenuBar.h>
#include <fltk/ToggleButton.h>
#include <fltk/PopupMenu.h>
#include <fltk/Choice.h>
#include <fltk/Tooltip.h>
#include <fltk/draw.h>

#include <stdio.h>
#include <stdlib.h>
#include <fltk/string.h>

#ifdef __APPLE__
#include <fltk/SystemMenuBar.h>
#endif

fltk::Window *window;

fltk::Menu* menus[4];

void test_cb(fltk::Widget* w, void*)
{
  fltk::Menu* mw = (fltk::Menu*)w;
  fltk::Widget* m = mw->item();
  if (!m)
    printf("NULL\n");
  else if (m->shortcut())
    printf("%s - %s\n", m->label(), fltk::key_name(m->shortcut()));
  else
    printf("%s\n", m->label());

  if (!strcmp("item 77", m->label())) {
    if (mw->find("button")) mw->replace("button", "Spitzak");
    else mw->replace("Spitzak", "button");
    menus[0]->redraw();
  }

  m->do_callback();
}

void quit_cb(fltk::Widget*, void*) { exit(0); }

Fl_Menu_Item hugemenu[100];

Fl_Menu_Item menutable[] = {
  {"foo",0,0,0,FL_MENU_INACTIVE},
  {"&File",0,0,0,FL_SUBMENU},
    {"&Open",	fltk::COMMAND+'O', 0, 0, FL_MENU_INACTIVE},
    {"&Close",	0,	0},
    {"&Quit",	fltk::COMMAND+'Q', quit_cb, 0, fltk::MENU_DIVIDER},
    {"shortcut",'A'},
    {"shortcut",fltk::SHIFT+'A'},
    {"shortcut",fltk::COMMAND+'A'},
    {"shortcut",fltk::COMMAND+fltk::SHIFT+'A'},
    {"shortcut",fltk::ACCELERATOR+'A'},
    {"shortcut",fltk::ACCELERATOR+fltk::SHIFT+'A'},
    {"shortcut",fltk::ACCELERATOR+fltk::COMMAND+'A'},
    {"shortcut",fltk::ACCELERATOR+fltk::SHIFT+fltk::COMMAND+'A', 0,0, fltk::MENU_DIVIDER},
    {"shortcut",fltk::ReturnKey},
    {"shortcut",fltk::COMMAND+fltk::ReturnKey, 0,0, fltk::MENU_DIVIDER},
    {"shortcut",fltk::F1Key},
    {"shortcut",fltk::SHIFT+fltk::F1Key},
    {"shortcut",fltk::COMMAND+fltk::F1Key},
    {"shortcut",fltk::SHIFT+fltk::COMMAND+fltk::F1Key},
    {"shortcut",fltk::ACCELERATOR+fltk::F1Key},
    {"shortcut",fltk::ACCELERATOR+fltk::SHIFT+fltk::F1Key},
    {"shortcut",fltk::ACCELERATOR+fltk::COMMAND+fltk::F1Key},
    {"shortcut",fltk::ACCELERATOR+fltk::SHIFT+fltk::COMMAND+fltk::F1Key, 0,0, fltk::MENU_DIVIDER},
    {"&Submenus", fltk::ACCELERATOR+'S',	0, (void*)"Submenu1", FL_SUBMENU},
      {"A very long menu item"},
      {"&submenu",fltk::COMMAND+'S',	0, (void*)"submenu2", FL_SUBMENU},
	{"item 1"},
	{"item 2"},
	{"item 3"},
	{"item 4"},
	{0},
      {"after submenu"},
      {0},
    {0},
  {"&Edit",0,0,0,FL_SUBMENU},
    {"Undo",	fltk::COMMAND+'Z',	0},
    {"Redo",	fltk::COMMAND+'Y',	0, 0, fltk::MENU_DIVIDER},
    {"Cut",	fltk::COMMAND+'X',	0},
    {"Copy",	fltk::COMMAND+'C',	0},
    {"Paste",	fltk::COMMAND+'V',	0},
    {"Inactive",fltk::COMMAND+'D',	0, 0, FL_MENU_INACTIVE},
    {"Clear",	0,	0, 0, fltk::MENU_DIVIDER},
    {"Invisible",fltk::COMMAND+'E',	0, 0, FL_MENU_INVISIBLE},
    {"Preferences",0,	0},
    {"Larger", '+', 0, 0},
    {"Smaller", '-', 0, 0},
    {0},
  {"&Checkbox",0,0,0,FL_SUBMENU},
    {"&Alpha",	0,	0, (void *)1, fltk::MENU_TOGGLE|FL_MENU_VALUE},
    {"&Beta",	0,	0, (void *)2, fltk::MENU_TOGGLE},
    {"&Gamma",	0,	0, (void *)3, fltk::MENU_TOGGLE},
    {"&Delta",	0,	0, (void *)4, fltk::MENU_TOGGLE|FL_MENU_VALUE},
    {"&Epsilon",0,	0, (void *)5, fltk::MENU_TOGGLE},
    {"&Pi",	0,	0, (void *)6, fltk::MENU_TOGGLE},
    {"&Mu",	0,	0, (void *)7, fltk::MENU_TOGGLE|fltk::MENU_DIVIDER},
    {"Red",	0,	0, (void *)1, fltk::MENU_TOGGLE},
    {"Black",	0,	0, (void *)1, fltk::MENU_TOGGLE|fltk::MENU_DIVIDER},
    {"00",	0,	0, (void *)1, fltk::MENU_TOGGLE},
    {"000",	0,	0, (void *)1, fltk::MENU_TOGGLE},
    {0},
  {"&Radio",0,0,0,FL_SUBMENU},
    {"&Alpha",	0,	0, (void *)1, fltk::MENU_RADIO},
    {"&Beta",	0,	0, (void *)2, fltk::MENU_RADIO},
    {"&Gamma",	0,	0, (void *)3, fltk::MENU_RADIO},
    {"&Delta",	0,	0, (void *)4, fltk::MENU_RADIO|FL_MENU_VALUE},
    {"&Epsilon",0,	0, (void *)5, fltk::MENU_RADIO},
    {"&Pi",	0,	0, (void *)6, fltk::MENU_RADIO},
    {"&Mu",	0,	0, (void *)7, fltk::MENU_RADIO|fltk::MENU_DIVIDER},
    {"Red",	0,	0, (void *)1, fltk::MENU_RADIO},
    {"Black",	0,	0, (void *)1, fltk::MENU_RADIO|fltk::MENU_DIVIDER},
    {"00",	0,	0, (void *)1, fltk::MENU_RADIO},
    {"000",	0,	0, (void *)1, fltk::MENU_RADIO},
    {0},
  {"&Font",0,0,0,FL_SUBMENU},
    {"Normal",	0, 0},
    {"Bold",	0, 0},
    {"Italic",	0, 0},
    {"BoldItalic",0,0},
    {"Small",	0, 0},
    {"Large",	0, 0},
    {"Emboss",	0, 0},
    {"Engrave",	0, 0},
    {"Shadow",	0, 0},
    {"@->",	0, 0},
    {0},
  {"E&mpty",0,0,0,FL_SUBMENU},
    {0},
  {"&Inactive", 0,	0, 0, FL_MENU_INACTIVE|FL_SUBMENU},
    {"A very long menu item"},
    {"A very long menu item"},
    {0},
  {"Invisible",0,	0, 0, FL_MENU_INVISIBLE|FL_SUBMENU},
    {"A very long menu item"},
    {"A very long menu item"},
    {0},
  {"&Huge", 0, 0, (void*)hugemenu, FL_SUBMENU_POINTER},
  // these buttons demonstrates that the menubar can be used as a "toolbar"
  {"@[]"}, {"@<->"}, {"@+"},
  // it would be nice if checkmarks worked, but they don't:
  //{"toggle",0, 0, 0, fltk::MENU_TOGGLE},
  {0}
};

Fl_Menu_Item pulldown[] = {
  {"Red",	fltk::ACCELERATOR+'r'},
  {"Green",	fltk::ACCELERATOR+'g'},
  {"Blue",	fltk::ACCELERATOR+'b'},
  {"Strange",	fltk::ACCELERATOR+'s'},
  {"&Charm",	fltk::ACCELERATOR+'c'},
  {"Truth",	fltk::ACCELERATOR+'t'},
  {"Beauty",	fltk::ACCELERATOR+'b'},
  {0}
};

#define WIDTH 600
#define HEIGHT 22 //30 // use 25 for better Windoze look

int main(int argc, char **argv)
{
  for (int i=0; i<99; i++) {
    char buf[100];
    sprintf(buf,"item %d",i);
    hugemenu[i].text = newstring(buf);
  }

  fltk::Window window(WIDTH,400);
  window.color(fltk::WHITE);
  window.tooltip("Press right button\nfor a pop-up menu");
  window.begin();

  fltk::MenuBar menubar(0,0,WIDTH,HEIGHT); menubar.menu(menutable);
  menubar.find("&Font/Normal")->labelfont(fltk::HELVETICA);
  menubar.find("&Font/Bold")->labelfont(fltk::HELVETICA_BOLD);
  menubar.find("&Font/Italic")->labelfont(fltk::HELVETICA_ITALIC);
  menubar.find("&Font/BoldItalic")->labelfont(fltk::HELVETICA_BOLD_ITALIC);
  menubar.find("&Font/Small")->labelsize(10);
  menubar.find("&Font/Large")->labelsize(24);
  menubar.find("&Font/Emboss")->labeltype(fltk::EMBOSSED_LABEL);
  menubar.find("&Font/Engrave")->labeltype(fltk::ENGRAVED_LABEL);
  menubar.find("&Font/Shadow")->labeltype(fltk::SHADOW_LABEL);
  menubar.find("&Font/@->")->labeltype(fltk::SYMBOL_LABEL);
  menubar.find("&Checkbox/Red")->labelcolor(fltk::RED); // label text red
  menubar.find("&Checkbox/Red")->selection_textcolor(fltk::RED); // label text red when selected
  menubar.find("&Checkbox/Red")->textcolor(fltk::RED); // check mark red
  menubar.find("&Checkbox/Black")->labelcolor(fltk::BLACK);
  menubar.find("&Checkbox/Black")->selection_textcolor(fltk::BLACK);
  menubar.find("&Checkbox/Black")->textcolor(fltk::BLACK);
  menubar.find("&Radio/Red")->labelcolor(fltk::RED);
  menubar.find("&Radio/Red")->selection_textcolor(fltk::RED);
  menubar.find("&Radio/Red")->textcolor(fltk::RED);
  menubar.find("&Radio/Black")->labelcolor(fltk::BLACK);
  menubar.find("&Radio/Black")->selection_textcolor(fltk::BLACK);
  menubar.find("&Radio/Black")->textcolor(fltk::BLACK);
  //menubar.find("&Huge/item 69")->deactivate();
  menubar.callback(test_cb);
  menubar.tooltip("This is a menu bar");
  menus[0] = &menubar;

  fltk::PopupMenu mb1(100,100,120,25,"&menubutton"); mb1.menu(pulldown);
  mb1.callback(test_cb);
  mb1.tooltip("This is a menu button");
  menus[1] = &mb1;

  fltk::Choice ch(300,100,90,25,"&choice:"); ch.menu(pulldown);
  ch.callback(test_cb);
  ch.tooltip("This is a choice");
  menus[2] = &ch;

  fltk::PopupMenu mb(0,25,WIDTH,400-HEIGHT/*,"&popup"*/);
  mb.type(fltk::PopupMenu::POPUP3);
  mb.menu(menutable);
  mb.callback(test_cb);
  menus[3] = &mb;

  window.resizable(&mb);
  window.size_range(300,20);
  window.end();
  window.show(argc, argv);

#ifdef __APPLE__
  fltk::SystemMenuBar sysmb(0, 0, 1, 1);
  sysmb.menu(menutable);
  sysmb.find("&Font/Normal")->labelfont(fltk::HELVETICA);
  sysmb.find("&Font/Bold")->labelfont(fltk::HELVETICA_BOLD);
  sysmb.find("&Font/Italic")->labelfont(fltk::HELVETICA_ITALIC);
  sysmb.find("&Font/BoldItalic")->labelfont(fltk::HELVETICA_BOLD_ITALIC);
  sysmb.find("&Font/Small")->labelsize(10);
  sysmb.find("&Font/Large")->labelsize(24);
  sysmb.find("&Font/Emboss")->labeltype(fltk::EMBOSSED_LABEL);
  sysmb.find("&Font/Engrave")->labeltype(fltk::ENGRAVED_LABEL);
  sysmb.find("&Font/Shadow")->labeltype(fltk::SHADOW_LABEL);
  sysmb.find("&Font/@->")->labeltype(fltk::SYMBOL_LABEL);
  sysmb.find("&Checkbox/Red")->labelcolor(fltk::RED); // label text red
  sysmb.find("&Checkbox/Red")->selection_textcolor(fltk::RED); // label text red when selected
  sysmb.find("&Checkbox/Red")->textcolor(fltk::RED); // check mark red
  sysmb.find("&Checkbox/Black")->labelcolor(fltk::BLACK);
  sysmb.find("&Checkbox/Black")->selection_textcolor(fltk::BLACK);
  sysmb.find("&Checkbox/Black")->textcolor(fltk::BLACK);
  sysmb.find("&Radio/Red")->labelcolor(fltk::RED);
  sysmb.find("&Radio/Red")->selection_textcolor(fltk::RED);
  sysmb.find("&Radio/Red")->textcolor(fltk::RED);
  sysmb.find("&Radio/Black")->labelcolor(fltk::BLACK);
  sysmb.find("&Radio/Black")->selection_textcolor(fltk::BLACK);
  sysmb.find("&Radio/Black")->textcolor(fltk::BLACK);
  sysmb.callback(test_cb);
  sysmb.layout();
#endif

  return fltk::run();
}

//
// End of "$Id: menubar.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
