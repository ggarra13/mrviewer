//
// "$Id: file.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Fluid file routines for the Fast Light Tool Kit (FLTK).
//
// You may find the basic read_* and write_* routines to
// be useful for other programs.  I have used them many times.
// They are somewhat similar to tcl, using matching { and }
// to quote strings.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fltk/FL_VERSION.h>

#include "alignment_panel.h"
#include "Fluid_Image.h"
#include "WidgetType.h"
#include "PrefsData.h"

////////////////////////////////////////////////////////////////
// BASIC FILE WRITING:

static FILE *fout;

#include <fltk/ask.h>
int open_write(const char *s) {
  if (!s) {fout = stdout; return 1;}
  FILE *f = fopen(s,"w");
  if (!f) return 0;
  fout = f;
  return 1;
}

int close_write() {
  if (fout != stdout) {
    int x = fclose(fout);
    fout = stdout;
    return x >= 0;
  }
  return 1;
}

static int needspace;
int is_id(char); // in code.C

// write a string, quoting characters if necessary:
void write_word(const char *w) {
  if (needspace) putc(' ', fout);
  needspace = 1;
  if (!w || !*w) {fprintf(fout,"{}"); return;}
  const char *p;
  // see if it is a single word:
  for (p = w; is_id(*p); p++) ;
  if (!*p) {fprintf(fout,"%s",w); return;}
  // see if there are matching braces:
  int n = 0;
  for (p = w; *p; p++) {
    if (*p == '{') n++;
    else if (*p == '}') {n--; if (n<0) break;}
  }
  int mismatched = (n != 0);
  // write out brace-quoted string:
  putc('{', fout);
  for (; *w; w++) {
    switch (*w) {
    case '{':
    case '}':
      if (!mismatched) break;
    case '\\':
    case '#':
      putc('\\',fout);
      break;
    }
    putc(*w,fout);
  }
  putc('}', fout);
}

// write an arbitrary formatted word, or a comment, etc:
void write_string(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (needspace) fputc(' ',fout);
  vfprintf(fout, format, args);
  va_end(args);
  needspace = !isspace(format[strlen(format)-1]);
}

// start a new line and indent it for a given nesting level:
void write_indent(int n) {
  fputc('\n',fout);
  while (n--) {fputc(' ',fout); fputc(' ',fout);}
  needspace = 0;
}

// write a '{' at the given indenting level:
void write_open(int) {
  if (needspace) fputc(' ',fout);
  fputc('{',fout);
  needspace = 0;
}

// write a '}' at the given indenting level:
void write_close(int n) {
  if (needspace) write_indent(n);
  fputc('}',fout);
  needspace = 1;
}

////////////////////////////////////////////////////////////////
// BASIC FILE READING:

static FILE *fin;
static int lineno;
static const char *fname;

int open_read(const char *s) {
  lineno = 1;
  if (!s) {fin = stdin; fname = "stdin"; return 1;}
  FILE *f = fopen(s,"r");
  if (!f) return 0;
  fin = f;
  fname = s;
  return 1;
}

int close_read() {
  if (fin != stdin) {
    int x = fclose(fin);
    fin = 0;
    return x >= 0;
  }
  return 1;
}

#include <fltk/ask.h>

void read_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (!fin) {
    char buffer[1024];
    vsprintf(buffer, format, args);
    fltk::message(buffer);
  } else {
    fprintf(stderr, "%s:%d: ", fname, lineno);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
  }
  va_end(args);
}

static int hexdigit(int x) {
  if (isdigit(x)) return x-'0';
  if (isupper(x)) return x-'A'+10;
  if (islower(x)) return x-'a'+10;
  return 20;
}


static int read_quoted() {	// read whatever character is after a \ .
  int c,d,x;
  switch(c = fgetc(fin)) {
  case '\n': lineno++; return -1;
  case 'a' : return('\a');
  case 'b' : return('\b');
  case 'f' : return('\f');
  case 'n' : return('\n');
  case 'r' : return('\r');
  case 't' : return('\t');
  case 'v' : return('\v');
  case 'x' :	/* read hex */
    for (c=x=0; x<3; x++) {
      int ch = fgetc(fin);
      d = hexdigit(ch);
      if (d > 15) {ungetc(ch,fin); break;}
      c = (c<<4)+d;
    }
    break;
  default:		/* read octal */
    if (c<'0' || c>'7') break;
    c -= '0';
    for (x=0; x<2; x++) {
      int ch = fgetc(fin);
      d = hexdigit(ch);
      if (d>7) {ungetc(ch,fin); break;}
      c = (c<<3)+d;
    }
    break;
  }
  return(c);
}

// return a word read from the file, or NULL at the EOF:
// This will skip all comments (# to end of line), and evaluate
// all \xxx sequences and use \ at the end of line to remove the newline.
// A word is any one of:
//	a continuous string of non-space chars except { and } and #
//	everything between matching {...} (unless wantbrace != 0)
//	the characters '{' and '}'

static char *buffer;
static int buflen;
static void expand_buffer(int length) {
  if (length >= buflen) {
    if (!buflen) {
      buflen = length+1;
      buffer = (char*)malloc(buflen);
    } else {
      buflen = 2*buflen;
      if (length >= buflen) buflen = length+1;
      buffer = (char *)realloc((void *)buffer,buflen);
    }
  }
}

const char *read_word(int wantbrace) {
  int x;

  // skip all the whitespace before it:
  for (;;) {
    x = getc(fin);
    if (x < 0) {	// eof
      return 0;
    } else if (x == '#') {	// comment
      do x = getc(fin); while (x >= 0 && x != '\n');
      lineno++;
      continue;
    } else if (x == '\n') {
      lineno++;
    } else if (!isspace(x)) {
      break;
    }
  }

  expand_buffer(100);

  if (x == '{' && !wantbrace) {

    // read in whatever is between braces
    int length = 0;
    int nesting = 0;
    for (;;) {
      x = getc(fin);
      if (x<0) {read_error("Missing '}'"); break;}
      else if (x == '#') { // embedded comment
	do x = getc(fin); while (x >= 0 && x != '\n');
	lineno++;
	continue;
      } else if (x == '\n') lineno++;
      else if (x == '\\') {x = read_quoted(); if (x<0) continue;}
      else if (x == '{') nesting++;
      else if (x == '}') {if (!nesting--) break;}
      buffer[length++] = x;
      expand_buffer(length);
    }
    buffer[length] = 0;
    return buffer;

  } else if (x == '{' || x == '}') {
    // all the punctuation is a word:
    buffer[0] = x;
    buffer[1] = 0;
    return buffer;

  } else {

    // read in an unquoted word:
    int length = 0;
    for (;;) {
      if (x == '\\') {x = read_quoted(); if (x<0) continue;}
      else if (x<0 || isspace(x) || x=='{' || x=='}' || x=='#') break;
      buffer[length++] = x;
      expand_buffer(length);
      x = getc(fin);
    }
    ungetc(x, fin);
    buffer[length] = 0;
    return buffer;

  }
}

////////////////////////////////////////////////////////////////

// global int variables:
static struct {const char* name; int* value;} inttable[] = {
  {"gridx", prefs.gridxPtr()},
  {"gridy", prefs.gridyPtr()},
  {"snap", prefs.snapPtr()}
};

extern int header_file_set;
extern int code_file_set;
extern const char* header_file_name;
extern const char* code_file_name;
extern char* theme;

int write_file(const char *filename, int selected_only) {
  if (!open_write(filename)) return 0;
  write_string("# data file for the FLTK User Interface Designer (FLUID)\n"
	       "version %.4f",FL_VERSION);
  if (images_dir) write_string("\nimages_dir %s", images_dir);
  if(!include_H_from_C)
    write_string("\ndo_not_include_H_from_C");
  if (!selected_only) {
    write_string("\nheader_name"); write_word(header_file_name);
    write_string("\ncode_name"); write_word(code_file_name);
    write_string("\n%s %d","gridx", prefs.gridx());
    write_string("\n%s %d","gridy", prefs.gridy());
    write_string("\n%s %d","snap", prefs.snap());
  }
  if (theme && *theme) { write_string("\ntheme "); write_word(theme); }
  for (FluidType *p = FluidType::first; p;) {
    if (!selected_only || p->selected()) {
      p->write();
      write_string("\n");
      p = p->next_brother;
    } else {
      p = p->walk();
    }
  }
  return close_write();
}

////////////////////////////////////////////////////////////////
// read all the objects out of the input file:

void read_fdesign();

double read_version;

extern void set_theme(const char* s);

static void read_children(FluidType *p, int paste) {
  FluidType::current = p;
  for (;;) {
    unsigned int i;
    const char *c = read_word();
  REUSE_C:
    if (!c) {
      if (p && !paste) read_error("Missing '}'");
      break;
    }

    if (!strcmp(c,"}")) {
      if (!p) read_error("Unexpected '}'");
      break;
    }

    // this is the first word in a .fd file:
    if (!strcmp(c,"Magic:")) {
      read_fdesign();
      return;
    }

    if (!strcmp(c,"version")) {
      c = read_word();
      read_version = strtod(c,0);
      if (read_version<=0 || read_version>FL_VERSION)
	read_error("unknown version '%s'",c);
      continue;
    }

    // back compatability with Vincent Penne's original class code:
    if (!p && !strcmp(c,"define_in_struct")) {
	FluidType *t = FluidType::make("class");
      t->name(read_word());
      FluidType::current = p = t;
      paste = 1; // stops "missing }" error
      continue;
    }

    if (!strcmp(c,"do_not_include_H_from_C"))
    {
      include_H_from_C=0;
      goto CONTINUE;
    }
    if (!strcmp(c,"images_dir"))
    {
      images_dir = strdup(read_word()); // This will never get deleted ...
      if (!*images_dir) images_dir = 0;
      goto CONTINUE;
    }
    if (!strcmp(c,"header_name")) {
      if (!header_file_set) header_file_name = strdup(read_word());
      else read_word();
      goto CONTINUE;
    }

    if (!strcmp(c,"code_name")) {
      if (!code_file_set) code_file_name = strdup(read_word());
      else read_word();
      goto CONTINUE;
    }

    if (!strcmp(c, "theme") || !strcmp(c, "scheme")) {
      set_theme(read_word());
      goto CONTINUE;
    }

    for (i=0; i<sizeof(inttable)/sizeof(*inttable); i++) {
      if (!strcmp(c,inttable[i].name)) {
	c = read_word();
	*inttable[i].value = int(strtol(c,0,0));
	goto CONTINUE;
      }
    }

    {FluidType *t = FluidType::make(c);
    if (!t) {
      read_error("Unknown word \"%s\"", c);
      continue;
    }
    t->name(read_word());

    c = read_word(1);
    
	if (strcmp(c,"{") && t->is_class()) {   // <prefix> <name>
      t->prefix(t->name());
      t->name(c);
      c = read_word(1);
    }

    if (strcmp(c,"{")) {
      read_error("Missing property list for %s\n",t->title());
      goto REUSE_C;
    }

    t->open_ = false;
    for (;;) {
      const char *c = read_word();
      if (!c || !strcmp(c,"}")) break;
      t->read_property(c);
    }

    if (!t->is_parent()) continue;
    c = read_word(1);
    if (strcmp(c,"{")) {
      read_error("Missing child list for %s\n",t->title());
      goto REUSE_C;
    }
    read_children(t, 0);}
    FluidType::current = p;
  CONTINUE:;
  }
}

extern void deselect();

int read_file(const char *filename, int merge) {
  FluidType* parent = merge ? FluidType::current : 0;
  read_version = 0.0;
  if (!open_read(filename)) return 0;
  if (merge) deselect();
  else delete_all();
  read_children(parent, merge);
  FluidType::current = parent;
  for (FluidType *o = FluidType::first; o; o = o->walk())
    if (o->selected()) {
      FluidType::current = o; break;
    }
  return close_read();
}

////////////////////////////////////////////////////////////////
// Read Forms and XForms fdesign files:

int read_fdesign_line(const char*& name, const char*& value) {

  int length = 0;
  int x;
  // find a colon:
  for (;;) {
    x = getc(fin);
    if (x < 0) return 0;
    if (x == '\n') {length = 0; continue;} // no colon this line...
    if (!isspace(x)) {
      buffer[length++] = x;
      expand_buffer(length);
    }
    if (x == ':') break;
  }
  int valueoffset = length;
  buffer[length-1] = 0;

  // skip to start of value:
  for (;;) {
    x = getc(fin);
    if (x < 0 || x == '\n' || !isspace(x)) break;
  }

  // read the value:
  for (;;) {
    if (x == '\\') {x = read_quoted(); if (x<0) continue;}
    else if (x == '\n') break;
    buffer[length++] = x;
    expand_buffer(length);
    x = getc(fin);
  }
  buffer[length] = 0;
  name = buffer;
  value = buffer+valueoffset;
  return 1;
}

int fdesign_flip;
int fdesign_magic;
#include <fltk/Group.h>

static const char *class_matcher[] = {
"fltk::CHECKBUTTON", "fltk::CheckButton",
"fltk::ROUNDBUTTON", "fltk::RadioButton",
"fltk::ROUND3DBUTTON", "fltk::RadioButton",
"fltk::LIGHTBUTTON", "fltk::LightButton",
"fltk::FRAME", "fltk::Box",
"fltk::LABELFRAME", "fltk::Box",
"fltk::TEXT", "fltk::Box",
"fltk::VALSLIDER", "fltk::ValueSlider",
"fltk::MENU", "fltk::PopupMenu",
"3", "fltk::BITMAP",
"1", "fltk::BOX",
"71","fltk::BROWSER",
"11","fltk::BUTTON",
"4", "fltk::CHART",
"42","fltk::CHOICE",
"61","fltk::CLOCK",
"25","fltk::COUNTER",
"22","fltk::DIAL",
"101","fltk::FREE",
"31","fltk::INPUT",
"12","fltk::LightButton",
"41","fltk::MENU",
"23","fltk::POSITIONER",
"13","fltk::RadioButton",
"21","fltk::SLIDER",
"2", "fltk::BOX", // was fltk::TEXT
"62","fltk::TIMER",
"24","fltk::ValueSlider",
0};

// This is copied from forms_compatability.cxx:

static void fl_end_group() {
  fltk::Group* g = fltk::Group::current();
  // set the dimensions of a group to surround contents
  if (g->children() && !g->w()) {
    fltk::Widget* o = g->child(0);
    int rx = o->x();
    int ry = o->y();
    int rw = rx+o->w();
    int rh = ry+o->h();
    for (int i=g->children()-1; i>0; i--) {
      o = g->child(i);
      if (o->x() < rx) rx = o->x();
      if (o->y() < ry) ry = o->y();
      if (o->x()+o->w() > rw) rw = o->x()+o->w();
      if (o->y()+o->h() > rh) rh = o->y()+o->h();
    }
    g->x(rx);
    g->y(ry);
    g->w(rw-rx);
    g->h(rh-ry);
  }
  // flip all the children's coordinate systems:
  //if (fltk::flip) {
    fltk::Widget* o = g->is_window() ? g : g->window();
    int Y = o->h();
    for (int i=g->children(); i--;) {
      fltk::Widget* o = g->child(i);
//      o->y(Y-o->y()-o->h());
      // I think this is equivalent?
      o->position(o->x(), Y-o->y()-o->h());
    }
    //g->oy_ = Y-g->oy_-g->h();
    //}
  g->fix_old_positions();
  g->end();
}

void read_fdesign() {
  fdesign_magic = atoi(read_word());
  fdesign_flip = (fdesign_magic < 13000);
  WidgetType *window = 0;
  WidgetType *group = 0;
  WidgetType *widget = 0;
  if (!FluidType::current) {
    FluidType *t = FluidType::make("Function");
    t->name("create_the_forms()");
    FluidType::current = t;
  }
  for (;;) {
    const char *name;
    const char *value;
    if (!read_fdesign_line(name, value)) break;

    if (!strcmp(name,"Name")) {

      window = (WidgetType*)FluidType::make("fltk::Window");
      window->name(value);
      window->label(value);
      FluidType::current = widget = window;

    } else if (!strcmp(name,"class")) {

      if (!strcmp(value,"fltk::BEGIN_GROUP")) {
	  group = widget = (WidgetType*)FluidType::make("fltk::Group");
	FluidType::current = group;
      } else if (!strcmp(value,"fltk::END_GROUP")) {
	if (group) {
	  fltk::Group* g = (fltk::Group*)(group->o);
	  g->begin();
	  // g->forms_end();
	  fl_end_group(); // how 'bout this instead?
	  fltk::Group::current(0);
	}
	group = widget = 0;
	FluidType::current = window;
      } else {
	for (int i = 0; class_matcher[i]; i += 2)
	  if (!strcmp(value,class_matcher[i])) {
	    value = class_matcher[i+1]; break;}
	  widget = (WidgetType*)FluidType::make(value);
	if (!widget) {
	  printf("class %s not found, using fltk::Button\n", value);
	  widget = (WidgetType*)FluidType::make("fltk::Button");
	}
      }

    } else if (widget) {
      if (!widget->read_fdesign(name, value))
	printf("Ignoring \"%s: %s\"\n", name, value);
    }
  }
}

//
// End of "$Id: file.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
