//
// "$Id: KDE.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Copyright 2004 Bill Spitzak and others.
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

// Read the "INI" style files written by KDE 1,2,3 and extract some
// appearance information for fltk from it. Older KDE also indicated
// if windows or Motif style is wanted, but this code is commented out
// because newer KDE ignores it.

// Original KDE theme code by Carl Thompson (clip@home.net)
// Rewritten to single file by Bill Spitzak

#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <fltk/Monitor.h>
#include <fltk/events.h>
#include <stdio.h>
#include <fltk/string.h>
#ifndef _WIN32
# include <fltk/x.h>
# include <unistd.h>
#endif
#ifndef PATH_MAX
# define PATH_MAX 1024
#endif
#include <stdlib.h>           // atoi()
#include <ctype.h>            // isspace()

using namespace fltk;

//kdewin_menu_text_box("kde windows menu window", "2AAUUIIXX", &fltk::downBox);

////////////////////////////////////////////////////////////////
#ifndef _WIN32
// this function handles KDE style change messages:

// I think these are for KDE1:
static Atom ChangeGeneral, ChangeStyle, ChangePalette;

// For KDE2:
static Atom KIPC;
enum KIPCMessage {
  PaletteChanged=0, FontChanged, StyleChanged, BackgroundChanged,
  SettingsChanged, IconChanged, UserMessage=32
};

static int x_event_handler(int,Window*) {
  if (fltk::xevent.type != ClientMessage) return 0; // not a Client message
  XClientMessageEvent* cm = (XClientMessageEvent*)&fltk::xevent;
  if (cm->message_type == KIPC) {
    if (cm->data.l[0] == PaletteChanged) ;
    else if (cm->data.l[0] == FontChanged) ;
    //else if (cm->data.l[0] == StyleChanged) ; // only if Motif is used
    else return 0;
  } else if (cm->message_type == ChangeGeneral) ;
  else if (cm->message_type == ChangePalette) ;
  //else if (cm->message_type == ChangeStyle) ;
  else return 0;
  fltk::reload_theme();

  return 1;
}

static void add_event_handler() {
  static bool done;
  if (!done) {
    done = 1;
    open_display();
    Atom kde_atom = XInternAtom(xdisplay, "KDE_DESKTOP_WINDOW", False);
    long data = 1;
    XChangeProperty(xdisplay, message_window, kde_atom, kde_atom, 32,
                    PropModeReplace, (unsigned char *)&data, 1);

    ChangeGeneral = XInternAtom(xdisplay, "KDEChangeGeneral", False);
    ChangeStyle = XInternAtom(xdisplay, "KDEChangeStyle", False);
    ChangePalette = XInternAtom(xdisplay, "KDEChangePalette", False);
    KIPC = XInternAtom(xdisplay, "KIPC_COMM_ATOM", False);
    fltk::add_event_handler(x_event_handler);
  }
}

#endif

#ifndef R_OK
#define R_OK 4
#endif
////////////////////////////////////////////////////////////////

// Fonts have been seen defined several ways in the .kderc files:
//
// name,ptsize,pxlsize?,encoding?,weight,slant,<up to 4 mystery numbers>
//
// pxlsize?: some files have ptsize==-1, in this case this appears to be
// a size in pixels, but I am unsure. When ptsize is greater than 0 this
// seems to be some mystery number.
//
// encoding? early versions had the iso8859-1 type name here as text. Now
// there is a number that may be the Windows number, or x from iso8859-x?
// Sometimes it is zero. Currently I ignore this.

static fltk::Font* grok_font(char* s, float& fontsize, char* fontencoding)
{
  char* sv; // to save strtok_r() state
  const char* p;

  // get the name:
  char fontname[64] = "";
  if ( (p = strtok_r(s, ",", &sv)) ) {
    // Turn "adobe-foobar" into just "foobar":
    char* q = strchr(p, '-');
    if (q) p = q+1;
    strncpy(fontname, p, sizeof(fontname));
  }

  // Read point size field:
  // as far as I can tell, any dpi other than 96 is a mistake by XFree86:
  fontsize = 12;
  if ( (p = strtok_r(0, ",", &sv)) )
    //fontsize = atoi(p) * Monitor::all().dpi()/72;
    fontsize = atoi(p) * 96/72;

  // If it is bad, guess that the next is pixelsize:
  p = strtok_r(0, ",", &sv);
  if (fontsize < 5 || fontsize > 64) {
    if (p) fontsize = atoi(p);
    // go back to default if it looks completely messed up:
    if (fontsize < 5 || fontsize > 64) fontsize = 12;
  }

  // next field may be the encoding:
  fontencoding[0] = 0;
  if ( (p = strtok_r(0, ",", &sv)) ) {
    strncpy(fontencoding, p, sizeof(fontencoding));
    // remove dash between iso and rest of text:
    if (!strncasecmp(fontencoding, "iso-", 4))
      memmove(fontencoding+3,fontencoding+4, strlen(fontencoding+4)+1);
  }
  // next two fields are the weight and slant:
  int attrib = 0;
  if ( (p = strtok_r(0, ",", &sv)) && atoi(p) >= 75 ) attrib = BOLD;
  if ( (p = strtok_r(0, ",", &sv)) && atoi(p) > 0 ) attrib |= ITALIC;

  return fltk::font(fontname, attrib);
}

////////////////////////////////////////////////////////////////
// Mini INI file reader:
// You can load several files. The first occurance of a name takes
// precedence, so load the files "backwards", with the user's local
// one first.

class INIFile {
  typedef struct {
    const char* section;
    const char* name;
    const char* value;
    int serial;
  } Entry;
  Entry* entries;
  unsigned count;
  unsigned array_size;
  static int compare(const void*, const void*);
public:
  INIFile() : entries(0), count(0), array_size(0) {}
  bool load(const char* filename);
  void clear();
  ~INIFile() {clear();}
  const char* get(const char* section, const char* name);
};

int INIFile::compare(const void* ap, const void* bp) {
  Entry* a = *(Entry**)ap;
  Entry* b = *(Entry**)bp;
  int n = strcasecmp(a->section,b->section);
  if (!n) n = strcasecmp(a->value,b->value);
  if (!n) n = a->serial-b->serial;
  return n;
}

bool INIFile::load(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file) return false;
  char buffer[1024];
  char section[128];
  for (;;) {
    if (!fgets(buffer, 1024, file)) break;
    char* p = buffer;
    while (isspace(*p)) p++;
    if (*p == '[') {
      // it's a section title
      char* q = p+1;
      while (*q && *q != ']') q++;
      if (*q == ']' && q-p < 128) {
	*q = 0;
	strcpy(section, p);
      }
    } else if (isalnum(*p)) {
      // it's a name or name=value
      char* q = p+1;
      while (*q && *q != '=' && *q != '\n') q++;
      char* r = q; // point at end
      while (r>p && isspace(*(r-1))) r--;
      if (*q=='=') q++; else q = 0; // remember if = was found
      *r = 0;
      const char* name = newstring(p);
      const char* value = "";
      if (q) {
	while (isspace(*q)) q++;
	for (r = q; *r && *r != '\n'; r++);
	while (r>q && isspace(*(r-1))) r--;
	if (r > q) {*r = 0; value = newstring(q);}
      }
      printf("Creating %s,%s,%s\n", section,name,value);
      //Entry* entry = new Entry();
      Entry entry;
      entry.section = newstring(section);
      entry.name = name;
      entry.value = value;
      entry.serial = count;
      if (count >= array_size) {
	array_size = array_size ? 2*array_size : 100;
	Entry* newarray = new Entry[array_size];
	if (count) memcpy(newarray, entries, count*sizeof(*entries));
	delete[] entries;
	entries = newarray;
      }
      entries[count++] = entry;
    }
  }
  fclose(file);
  qsort(entries, count, sizeof(*entries), compare);
  return true;
}

// Does binary search for the item:
const char* INIFile::get(const char* section, const char* name) {
  if (!section) section = "";
  if (!name) name = "";
  int a = 0;
  int b = count;
  while (a < b) {
    int c = (a+b)/2;
    Entry* e = entries+c;
    int n = strcasecmp(e->section, section);
    if (!n) n = strcasecmp(e->name, name);
    if (n < 0) a = c+1;
    else if (n > 0) b = c;
    else {
      // search backwards for earliest one read:
      while (c>0) {
	Entry* e1 = entries + --c;
	if (strcasecmp(e->section, section)) break;
	if (strcasecmp(e->name, name)) break;
	e = e1;
      }
      return e->value;
    }
  }
  return 0;
}

void INIFile::clear() {
  for (int i = count; i--;) {
    Entry* e = entries+i;
    delete[] e->section;
    delete[] e->name;
    if (e->value[0]) delete[] e->value;
  }
  delete[] entries;
  entries = 0;
  count = 0;
  array_size = 0;
}

////////////////////////////////////////////////////////////////

extern "C" bool fltk_theme() {

  fltk::reset_theme();

  const char* home = getenv("HOME");
  if (!home) return false;
  char kderc[PATH_MAX];
  int kde1 = 0;
  snprintf(kderc, sizeof(kderc), "%s/.kde/share/config/kdeglobals", home);
  if (access(kderc, R_OK)) {
    snprintf(kderc, sizeof(kderc), "%s/.kderc", home);
    kde1 = 1;
  }

  INIFile f;
  // skip it if it looks like they are not using kde:
  if (!f.load(kderc)) return false;
  // skip if it looks like kde is not installed:
  if (!f.load("/usr/share/config/kdeglobals")) return false;

  const char* s;

#if 0
  int motif_style = 0;
  s = f.get("General", "widgetStyle");
  if (!s) s = f.get("KDE","widgetStyle");
  if (s && !strcasecmp(s, "Motif")) motif_style = 1;
  if (motif_style) {
    // load the motif.theme to set it to motif appearance:
    Theme f = Style::load_theme("motif");
    if (f) f();
    else {
      fprintf(stderr,"Unable to load motif theme as part of kde\n");
      motif_style = 0;
    }
    // see below for modifications to the motif/windows themes
  }
#endif

  Color foreground = NO_COLOR;
  if ((s=f.get("General", "foreground")))
    foreground = color(s);

  Color background = NO_COLOR;
  if ((s=f.get("General", "background")))
    background = color(s);

  Color select_foreground = NO_COLOR;
  if ((s=f.get("General", "selectForeground")))
    select_foreground = color(s);

  Color select_background = NO_COLOR;
  if ((s=f.get("General", "selectBackground")))
    select_background = color(s);

  // this one seems to do absolutely nothing
  Color window_foreground = NO_COLOR;
  if ((s=f.get("General", "windowForeground")))
    window_foreground = color(s);

  Color window_background = NO_COLOR;
  if ((s=f.get("General", "windowBackground")))
    window_background = color(s);

  Color button_foreground = NO_COLOR;
  if ((s=f.get("General", "buttonForeground")))
    button_foreground = color(s);

  Color button_background = NO_COLOR;
  if ((s=f.get("General", "buttonBackground")))
    button_background = color(s);

  if ((s=f.get("General", "font"))) {
    float fontsize; static char fontencoding[32];
    fltk::Font* font = grok_font((char*)s, fontsize, fontencoding);
    if (font) {
// CET - FIXME    if (*fontencoding) fltk::encoding(fontencoding);
      Widget::default_style->labelfont(font);
      Widget::default_style->textfont(font);
    }
    Widget::default_style->labelsize(fontsize);
    Widget::default_style->textsize(fontsize);
  }

  if ((s=f.get("General", "menuFont"))) {
    float fontsize; static char fontencoding[32];
    fltk::Font* font = grok_font((char*)s, fontsize, fontencoding);
    Style* style;
    if ((style = Style::find("MenuBar"))) {
      if (font) style->textfont(font);
      style->textsize(fontsize);
    }
    if ((style = Style::find("PopupMenu"))) {
      if (font) style->textfont(font);
      style->textsize(fontsize);
    }
  }

  Style* style = Widget::default_style;

  // turn off highlighting:
//    style->highlight_color(NO_COLOR);
//    style->highlight_textcolor(NO_COLOR);

  if (background)
    fltk::set_background(background);

  if (foreground) {
    style->labelcolor(foreground);
    style->textcolor(foreground);
    style->selection_textcolor(foreground);
  }

  if (button_background && button_background != background)
    style->buttoncolor(button_background);

  if (window_background && window_background != background)
    style->color(window_background);

  if (window_foreground)
    style->textcolor(window_foreground);

  if (select_background)
    style->selection_color(select_background);

  if (select_foreground)
    style->selection_textcolor(select_foreground);

  if (button_foreground && button_foreground != foreground &&
      (style = Style::find("button"))) {
    style->labelcolor(button_foreground);
  }

// Don't bother.  KDE gets it wrong.
//  if ((style = Style::find("scrollbar"))) {
//    if (background && window_background)
//      style->color(fltk::lerp(background, window_background, 0.5));
//  }

/* looks better for dark backgrounds
  if ((style = Style::find("scrollbar"))) {
    if (foreground) style->color(48);
  }

  if ((style = Style::find("slider"))) {
    if (foreground) style->color(48);
  }

  if ((style = Style::find("value slider"))) {
    if (foreground) style->color(48);
  }
*/

#if 0
  if (motif_style) {
//  setcolor(GRAY90, GRAY85); // looks better for dark backgrounds
    if ((style = Style::find("menu"))) {
      style->leading(4);
    }
    if ((style = Style::find("check button"))) {
      style->selection_color(GRAY66);
      style->buttoncolor(GRAY75);
    }
  } else {

//  if ((style = Style::find("menu"))) {
//    style->leading(8);
//    style->box(&kdewin_menu_text_box);
//  }

//  if ((style = Style::find("scrollbar"))) {
//    style->box(&kdewin_menu_text_box);
//  }
  }
#endif

#ifndef _WIN32
  add_event_handler();
#endif
  return true;
}

//
// End of "$Id: KDE.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
