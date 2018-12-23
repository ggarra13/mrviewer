//
// "$Id: factory.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Widget factory code for the Fast Light Tool Kit (FLTK).
//
// Type classes for most of the fltk widgets.  Most of the work
// is done by code in WidgetType.C.  Also a factory instance
// of each of these type classes.
//
// This file also contains the "new" menu, which has a pointer
// to a factory instance for every class (both the ones defined
// here and ones in other files)
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
#include <fltk/Group.h>
#include <fltk/MenuBuild.h>
#include <fltk/MenuBar.h>
#include <fltk/string.h>

#include <string.h>
#include <stdio.h>

#include "Enumeration.h"
#include "Widget_Types.h"
#include "Fluid_Plugins.h"
#include "fluid_menus.h"
#include "undo.h"

using namespace fltk;

////////////////////////////////////////////////////////////////
const Enumeration buttontype_menu[] = {
  {"Normal", 0,		(void*)0},
  {"Toggle", "TOGGLE",	(void*)fltk::Button::TOGGLE},
  {"Radio",  "RADIO",	(void*)fltk::Button::RADIO},
  {0}};

const Enumeration input_type_menu[] = {
  {"Normal",	0,	(void*)Input::NORMAL},
  {"Numeric",	0,	(void*)6,			"fltk::NumericInput"},
  {"Float",	0,	(void*)1,			"fltk::FloatInput"},
  {"Int",	0,	(void*)2,			"fltk::IntInput"},
  {"Secret",	0,	(void*)Input::SECRET,		"fltk::SecretInput"},
  {"Multiline",	0,	(void*)Input::MULTILINE,	"fltk::MultiLineInput"},
  {"Wordwrap",	0,	(void*)Input::WORDWRAP,		"fltk::WordwrapInput"},
  {0}};

const Enumeration dial_type_menu[] = {
  {"Dot", 	0,	(void*)Dial::NORMAL},
  {"Line",	"LINE",	(void*)Dial::LINE},
  {"Fill",	"FILL",	(void*)Dial::FILL},
  {0}};

const Enumeration slider_type_menu[] = {
  {"Linear No ticks",	"LINEAR",	(void*)(fltk::Slider::LINEAR)},
  {"Linear Top/left ticks","TICK_ABOVE",(void*)(fltk::Slider::TICK_ABOVE)},
  {"Linear Bottom/right ticks","TICK_BELOW",(void*)(fltk::Slider::TICK_BELOW)},
  {"Linear Both ticks","TICK_BOTH", (void*)(fltk::Slider::TICK_BOTH)},

  {"Log No ticks",	"LOG",	(void*)(fltk::Slider::LOG)},
  {"Log Top/left ticks","LOG|fltk::Slider::TICK_ABOVE",(void*)(fltk::Slider::LOG|fltk::Slider::TICK_ABOVE)},
  {"Log Bottom/right ticks","LOG|fltk::Slider::TICK_BELOW",(void*)(fltk::Slider::LOG|fltk::Slider::TICK_BELOW)},
  {"Log Both ticks","LOG|fltk::Slider::TICK_BOTH", (void*)(fltk::Slider::LOG|fltk::Slider::TICK_BOTH)},
  {0}};

const Enumeration output_type_menu[] = {
  {"Normal",	0,	(void*)fltk::Output::NORMAL},
  {"Multiline",	0,	(void*)fltk::Output::MULTILINE, "fltk::MultiLineOutput"},
  {"Wordwrap",	0,	(void*)fltk::Output::WORDWRAP, "fltk::WordwrapOutput"},
  {0}};

////////////////////////////////////////////////////////////////
int TextEditorType::textstuff(int w, fltk::Font* f, int& s, fltk::Color c) {
    fltk::TextEditor *myo = (fltk::TextEditor*)(w==4 ? ((WidgetType*)factory)->o : o);
    switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = (int) myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize((float)s); break;
    case 3: myo->textcolor(c); break;
    }
    return 1;
}

int TextDisplayType::textstuff(int w, fltk::Font* f, int& s, fltk::Color c) {
  fltk::TextDisplay *myo = (fltk::TextDisplay*)(w==4 ? ((WidgetType*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = (int) myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize((float)s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

int FileInputType::textstuff(int w, fltk::Font* f, int& s, fltk::Color c) {
    fltk::FileInput *myo = (fltk::FileInput*)(w==4 ? ((WidgetType*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = (int) myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize((float)s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

extern void select(FluidType *,int);

using namespace fltk;


//////////////////////////////////////////////////////////////////////

int reading_file;

// Recursive function for searching submenus:
static FluidType *FluidType_make(const char *tn, fltk::ItemGroup * menu) {
  FluidType *r = 0;
  fltk::Item * m=0;
  char menuName[128];
  int n;

  reading_file = 1; // makes labels be null

  if (!tn || strlen(tn)==0) 
      return 0;

  for (n = menu->children(); n--;) {
    fltk::Widget* w = menu->child(n);
    if (w->label() && w->is_group() && tn && *tn) {
	snprintf(menuName,sizeof(menuName),"%s/%s",w->label(),tn);
	m = (fltk::Item *) menu->find(menuName);
	if (m)  break;
      }
  }
  if (m && m->user_data())
    r = ((FluidType*)(m->user_data()))->make();

  reading_file = 0; // makes labels be null

  return r;
}

static struct {const char* oldname; const char* newname;} ntable[] = {
  {"submenu",		"fltk::ItemGroup"},
  {"menuitem",		"fltk::Item"},
  {"Fl_Counter",	"fltk::ValueInput"},
  {"Fl_Spinner",	"fltk::ValueInput"},
  {"Fl_Tabs",		"fltk::TabGroup"},
  {"Fl_Return_Button",	"fltk::ReturnButton"},
  {"fltk::EnterButton",	"fltk::ReturnButton"},
  {"Fl_Menu_Button",	"fltk::PopupMenu"},
  {"Fl_Box",		"fltk::InvisibleBox"},
  {"Fl_Boxtype",	"fltk::InvisibleBoxType"},
  {"Fl_Round_Button",	"fltk::RadioButton"},
  {"Fl_Pack",		"fltk::PackedGroup"},
  {"Fl_Tabs",		"fltk::TabGroup"},
  {"Fl_Scroll",		"fltk::ScrollGroup"},
  {"Fl_Bar",		"fltk::BarGroup"},
  {"Fl_Roller",		"fltk::ThumbWheel"},
  {"Fl_File_Browser",	"fltk::FileBrowser"},
  {"Fl_Text_Display",	"fltk::TextDisplay"},
  {"Fl_Text_Editor",	"fltk::TextEditor"},
  {"Fl_Tile",		"fltk::TiledGroup"}
};

// Create a new type by name.
FluidType* FluidType::make(const char *tn) {
  // Translate fltk1.0 names into fltk2.0 ones:
  const char* p = 0;
  // First try any of our direct translations:
  for (unsigned n = 0; n < sizeof(ntable)/sizeof(*ntable); n++) {
    if (!strcasecmp(ntable[n].oldname, tn)) {p = ntable[n].newname; break;}
  }
  char buffer[128];
  // Now try cooking old names into new ones:
  if (!p && !strncmp(tn, "Fl_", 3)) {
    p = buffer;
    strcpy(buffer, "fltk::");
    char* q = buffer+6;
    for (const char* r = tn+3; *r; r++) if (*r != '_') *q++ = *r;
    *q = 0;
  } else if (!p) {
    p = tn;
  }
  // Now search the "new" menu for them
  return ::FluidType_make(p, newMenu);
}

////////////////////////////////////////////////////////////////

// Since I have included all the .h files, do this table here:

// This table is only used for back compatability with XForms fdesign files,
// there should be no need to add to it in future versions of fluid.

struct symbol {const char *name; int value;};

static symbol table[] = {
  {"BLACK",	fltk::BLACK},
  {"RED",	fltk::RED},
  {"GREEN",	fltk::GREEN},
  {"YELLOW",	fltk::YELLOW},
  {"BLUE",	fltk::BLUE},
  {"MAGENTA",	fltk::MAGENTA},
  {"CYAN",	fltk::CYAN},
  {"WHITE",	fltk::WHITE},

  {"LCOL",		fltk::BLACK},
  {"COL1",		fltk::GRAY75},
  {"MCOL",		51},
  {"LEFT_BCOL",		55},
  {"TOP_BCOL",		53},
  {"BOTTOM_BCOL",	45},
  {"RIGHT_BCOL",	39},
  {"INACTIVE",		fltk::GRAY66},
  {"INACTIVE_COL",	fltk::GRAY66},
  {"FREE_COL1",		16},
  {"FREE_COL2",		17},
  {"FREE_COL3",		18},
  {"FREE_COL4",		19},
  {"FREE_COL5",		20},
  {"FREE_COL6",		21},
  {"FREE_COL7",		22},
  {"FREE_COL8",		23},
  {"FREE_COL9",		24},
  {"FREE_COL10",	25},
  {"FREE_COL11",	26},
  {"FREE_COL12",	27},
  {"FREE_COL13",	28},
  {"FREE_COL14",	29},
  {"FREE_COL15",	30},
  {"FREE_COL16",	31},
  {"TOMATO",		131},
  {"INDIANRED",		164},
  {"SLATEBLUE",		195},
  {"DARKGOLD",		84},
  {"PALEGREEN",		157},
  {"ORCHID",		203},
  {"DARKCYAN",		189},
  {"DARKTOMATO",	113},
  {"WHEAT",		174},

  {"ALIGN_CENTER",	fltk::ALIGN_CENTER},
  {"ALIGN_TOP",		fltk::ALIGN_TOP},
  {"ALIGN_BOTTOM",	fltk::ALIGN_BOTTOM},
  {"ALIGN_LEFT",	fltk::ALIGN_LEFT},
  {"ALIGN_RIGHT",	fltk::ALIGN_RIGHT},
  {"ALIGN_INSIDE",	fltk::ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT",	 fltk::ALIGN_TOP | fltk::ALIGN_LEFT},
  {"ALIGN_TOP_RIGHT",	 fltk::ALIGN_TOP | fltk::ALIGN_RIGHT},
  {"ALIGN_BOTTOM_LEFT",	 fltk::ALIGN_BOTTOM | fltk::ALIGN_LEFT},
  {"ALIGN_BOTTOM_RIGHT", fltk::ALIGN_BOTTOM | fltk::ALIGN_RIGHT},
  {"ALIGN_CENTER|FL_ALIGN_INSIDE",	fltk::ALIGN_CENTER|fltk::ALIGN_INSIDE},
  {"ALIGN_TOP|FL_ALIGN_INSIDE",		fltk::ALIGN_TOP|fltk::ALIGN_INSIDE},
  {"ALIGN_BOTTOM|FL_ALIGN_INSIDE",	fltk::ALIGN_BOTTOM|fltk::ALIGN_INSIDE},
  {"ALIGN_LEFT|FL_ALIGN_INSIDE",	fltk::ALIGN_LEFT|fltk::ALIGN_INSIDE},
  {"ALIGN_RIGHT|FL_ALIGN_INSIDE",	fltk::ALIGN_RIGHT|fltk::ALIGN_INSIDE},
  {"ALIGN_INSIDE|FL_ALIGN_INSIDE",	fltk::ALIGN_INSIDE|fltk::ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT|FL_ALIGN_INSIDE",	fltk::ALIGN_TOP|fltk::ALIGN_LEFT|fltk::ALIGN_INSIDE},
  {"ALIGN_TOP_RIGHT|FL_ALIGN_INSIDE",	fltk::ALIGN_TOP|fltk::ALIGN_RIGHT|fltk::ALIGN_INSIDE},
  {"ALIGN_BOTTOM_LEFT|FL_ALIGN_INSIDE",	fltk::ALIGN_BOTTOM|fltk::ALIGN_LEFT|fltk::ALIGN_INSIDE},
  {"ALIGN_BOTTOM_RIGHT|FL_ALIGN_INSIDE",fltk::ALIGN_BOTTOM|fltk::ALIGN_RIGHT|fltk::ALIGN_INSIDE},

  {"ALIGN_LEFT_TOP",	 fltk::ALIGN_TOP | fltk::ALIGN_LEFT},
  {"ALIGN_RIGHT_TOP",	 fltk::ALIGN_TOP | fltk::ALIGN_RIGHT},
  {"ALIGN_LEFT_BOTTOM",	 fltk::ALIGN_BOTTOM | fltk::ALIGN_LEFT},
  {"ALIGN_RIGHT_BOTTOM", fltk::ALIGN_BOTTOM | fltk::ALIGN_RIGHT},
  {"INVALID_STYLE",	 255},
  {"NORMAL_STYLE",	 0},
  {"BOLD_STYLE",	 1},
  {"ITALIC_STYLE",	 2},
  {"BOLDITALIC_STYLE",	 3},
  {"FIXED_STYLE",	 4},
  {"FIXEDBOLD_STYLE",	 5},
  {"FIXEDITALIC_STYLE",	 6},
  {"FIXEDBOLDITALIC_STYLE",  7},
  {"TIMES_STYLE",	 8},
  {"TIMESBOLD_STYLE",	 9},
  {"TIMESITALIC_STYLE",	 10},
  {"TIMESBOLDITALIC_STYLE",  11},
//   {"SHADOW_STYLE",	0x100
//   {"ENGRAVED_STYLE",	0x200
//   {"EMBOSSED_STYLE",	0x300
  {"TINY_SIZE",		8},
  {"SMALL_SIZE",	11},
  {"NORMAL_SIZE",	12},
  {"MEDIUM_SIZE",	18},
  {"LARGE_SIZE",	24},
  {"HUGE_SIZE",		32},
  {"DEFAULT_SIZE",	12},
  {"TINY_FONT",		8},
  {"SMALL_FONT",	11},
  {"NORMAL_FONT",	12},
  {"MEDIUM_FONT",	18},
  {"LARGE_FONT",	24},
  {"HUGE_FONT",		32},
  {"NORMAL_FONT1",	11},
  {"NORMAL_FONT2",	12},
  {"DEFAULT_FONT",	11},
  {"RETURN_END_CHANGED",0},
  {"RETURN_CHANGED",	1},
  {"RETURN_END",	2},
  {"RETURN_ALWAYS",	3},
  {"PUSH_BUTTON",	fltk::Button::TOGGLE},
  {"RADIO_BUTTON",	fltk::Button::RADIO},
  {"HIDDEN_BUTTON",	fltk::Button::HIDDEN},
  {"SELECT_BROWSER",	fltk::Browser::NORMAL},
  {"HOLD_BROWSER",	fltk::Browser::NORMAL},
  {"MULTI_BROWSER",	fltk::Browser::MULTI},
  //  {"SIMPLE_COUNTER",	fltk::Counter::SIMPLE},
  {"LINE_DIAL",		fltk::Dial::LINE},
  {"FILL_DIAL",		fltk::Dial::FILL},
  {"VERT_SLIDER",	fltk::Slider::LINEAR},
  {"HOR_SLIDER",	fltk::Slider::LINEAR},
  {"VERT_FILL_SLIDER",	fltk::Slider::LINEAR},
  {"HOR_FILL_SLIDER",	fltk::Slider::LINEAR},
  {"VERT_NICE_SLIDER",	fltk::Slider::LINEAR},
  {"HOR_NICE_SLIDER",	fltk::Slider::LINEAR},
};

#include <stdlib.h>

int lookup_fdesign(const char *name, int &v, int numberok) {
  if (name[0]=='F' && name[1]=='L' && name[2]=='_') name += 3;
  for (int i=0; i < int(sizeof(table)/sizeof(*table)); i++)
    if (!strcasecmp(name,table[i].name)) {v = table[i].value; return 1;}
  if (numberok) {
    char* p;
    v = strtol(name, &p, 0);
    if (!*p && p > name) return 1;
  }
  return 0;
}

//
// End of "$Id: factory.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
