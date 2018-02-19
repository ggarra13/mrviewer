// "$Id: symbols.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Symbol test program for the Fast Light Tool Kit (FLTK).
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

#include <fltk/Window.h>
#include <fltk/Browser.h>
#include <fltk/Symbol.h>
#include <fltk/run.h>
#include <fltk/string.h>
#include <fltk/Slider.h>
#include <stdlib.h>

using namespace fltk;

int sort(const void* a, const void* b) {
  const Symbol* A = *(const Symbol**)a;
  const Symbol* B = *(const Symbol**)b;
  return strcasecmp(A->name(), B->name());
}

struct fl_char_table {const char* name; const char* text;} specialtable[] = {
  {"b", "@t@@b;\t@n@b;bold"},
  {"B", "@t@@B#;\t@n@B11;bg color"},
  {"C", "@t@@C#;\t@n@C11;color"},
  {"f", "@t@@f;\t@n@f;fixed"},
  {"i", "@t@@i;\t@n@i;italic"},
  {"n", "@t@@n;\t@n;reset font & color"},
  {"t", "@t@@t;\t@t;typewriter"},
  {"s", "@t@@s+#;\t@n@s+4;bigger"},
  {"S", "@t@@s-#;\t@n@S-4;smaller"},
  {"x", "@t@@x#;\t@n@x+10;t@x+3;e@x-4;x@x+5;t (displace x)"},
  {"y", "@t@@y#;\t@n@y-1;t@y+1;e@y+3;x@y+5;t@n; (displace y)"},
  {".", "@t@@.;\t@n@.;raw mode: @&"},
  {"l",	"@t@@l;\t@n@l;left"},
  {"c",	"@t@@c;\t@n@c;center"},
  {"r",	"@t@@r;\t@n@r;right"}
};

void slider_cb(Widget* w, void* v) {
  Browser* browser = (Browser*)v;
  browser->textsize(((Slider*)w)->value());
  // make it remeasure all the child items (not sure if this is a bug
  // in the Browser or not:
  for (int i = 0; i < browser->children(); i++)
    browser->child(i)->h(0);
  browser->relayout();
  browser->redraw();
}

static const char* labels[] = {"command", "result", 0};
static int widths[] = {170,-1,0};

int main(int argc, char** argv) {
  Window window(250,700);
  window.begin();
  const int sliderh = 25;
  Browser browser(0,0,window.w(),window.h()-sliderh);
  browser.column_widths(widths);
  browser.column_labels(labels);
  browser.color(GRAY85);
  window.resizable(browser);
  Slider slider(0, browser.h(), window.w(), sliderh);
  slider.type(Slider::TICK_ABOVE|Slider::LOG);
  slider.range(1,60);
  slider.step(1);
  slider.value(browser.textsize());
  slider.callback(slider_cb, &browser);
  window.end();
  const Symbol* table[1000];
  int n = 0;
  int i; for (i=0;;) {
    const Symbol* symbol = Symbol::iterate(i);
    if (!symbol) break;
    table[n++] = symbol;
  }
  qsort(table, n, sizeof(*table), sort);
  for (i=0; i<n; i++) {
    const Symbol* symbol = table[i];
    char buffer[256];
    for (unsigned j = 0; j < sizeof(specialtable)/sizeof(*specialtable); j++) {
      if (!strcmp(specialtable[j].name, symbol->name())) {
        browser.add(specialtable[j].text);
        goto CONTINUE;
      }
    }
    snprintf(buffer, 256, "@t@@%s;\t@n@%s;", symbol->name(), symbol->name());
    browser.add(buffer);
  CONTINUE:;
  }
  window.show(argc, argv);
  return fltk::run();
}

/*
 * End of "$Id: symbols.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
 */
