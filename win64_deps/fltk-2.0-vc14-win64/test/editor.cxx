//
// "$Id: editor.cxx 8662 2011-05-15 06:04:24Z bgbnbigben $"
//
// A simple text editor program for the Fast Light Tool Kit (FLTK).
//
// This program is described in Chapter 4 of the FLTK Programmer's Guide.
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
//     http://www.fltk.org/str.php
//

//
// Include necessary headers...
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/Group.h>
#include <fltk/Window.h>
#include <fltk/ask.h>
#include <fltk/file_chooser.h>
#include <fltk/Input.h>
#include <fltk/Button.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextBuffer.h>
#include <fltk/TextEditor.h>
#include <fltk/MenuBuild.h>

int                changed = 0;
char               filename[256] = "";
char               title[256];
fltk::TextBuffer     *textbuf = 0;


// Syntax highlighting stuff...
fltk::TextBuffer     *stylebuf = 0;
fltk::TextDisplay::StyleTableEntry
                   styletable[] = {	// Style table
		     { fltk::BLACK,           fltk::COURIER,        12 }, // A - Plain
		     { fltk::DARK_GREEN,      fltk::COURIER_ITALIC, 12 }, // B - Line comments
		     { fltk::DARK_GREEN,      fltk::COURIER_ITALIC, 12 }, // C - Block comments
		     { fltk::BLUE,            fltk::COURIER,        12 }, // D - Strings
		     { fltk::DARK_RED,        fltk::COURIER,        12 }, // E - Directives
		     { fltk::DARK_RED,        fltk::COURIER_BOLD,   12 }, // F - Types
		     { fltk::BLUE,            fltk::COURIER_BOLD,   12/*, fltk::TextDisplay::ATTR_UNDERLINE*/ }  // G - Keywords
		   };
const char         *code_keywords[] = {	// List of known C/C++ keywords...
		     "and",
		     "and_eq",
		     "asm",
		     "bitand",
		     "bitor",
		     "break",
		     "case",
		     "catch",
		     "compl",
		     "continue",
		     "default",
		     "delete",
		     "do",
		     "else",
		     "false",
		     "for",
		     "goto",
		     "if",
		     "new",
		     "not",
		     "not_eq",
		     "operator",
		     "or",
		     "or_eq",
		     "return",
		     "switch",
		     "template",
		     "this",
		     "throw",
		     "true",
		     "try",
		     "using",
		     "while",
		     "xor",
		     "xor_eq"
		   };
const char         *code_types[] = {	// List of known C/C++ types...
		     "auto",
		     "bool",
		     "char",
		     "class",
		     "const",
		     "const_cast",
		     "double",
		     "dynamic_cast",
		     "enum",
		     "explicit",
		     "extern",
		     "float",
		     "friend",
		     "inline",
		     "int",
		     "long",
		     "mutable",
		     "namespace",
		     "private",
		     "protected",
		     "public",
		     "register",
		     "short",
		     "signed",
		     "sizeof",
		     "static",
		     "static_cast",
		     "struct",
		     "template",
		     "typedef",
		     "typename",
		     "union",
		     "unsigned",
		     "virtual",
		     "void",
		     "volatile"
		   };


//
// 'compare_keywords()' - Compare two keywords...
//

extern "C" {
  int
  compare_keywords(const void *a,
                   const void *b) {
    return (strcmp(*((const char **)a), *((const char **)b)));
  }
}

//
// 'style_parse()' - Parse text and produce style data.
//

void
style_parse(const char *text,
            char       *style,
	    int        length) {
  char	     current;
  int	     col;
  int	     last;
  char	     buf[255],
             *bufptr;
  const char *temp;

  // Style letters:
  //
  // A - Plain
  // B - Line comments
  // C - Block comments
  // D - Strings
  // E - Directives
  // F - Types
  // G - Keywords

  for (current = *style, col = 0, last = 0; length > 0; length --, text ++) {
    if (current == 'B' || current == 'F' || current == 'G') current = 'A';
    if (current == 'A') {
      // Check for directives, comments, strings, and keywords...
      if (col == 0 && *text == '#') {
        // Set style to directive
        current = 'E';
      } else if (strncmp(text, "//", 2) == 0) {
        current = 'B';
	for (; length > 0 && *text != '\n'; length --, text ++) *style++ = 'B';

        if (length == 0) break;
      } else if (strncmp(text, "/*", 2) == 0) {
        current = 'C';
      } else if (strncmp(text, "\\\"", 2) == 0) {
        // Quoted quote...
	*style++ = current;
	*style++ = current;
	text ++;
	length --;
	col += 2;
	continue;
      } else if (*text == '\"') {
        current = 'D';
      } else if (!last && (islower(*text) || *text == '_')) {
        // Might be a keyword...
	for (temp = text, bufptr = buf;
	     (islower(*temp) || *temp == '_') && bufptr < (buf + sizeof(buf) - 1);
	     *bufptr++ = *temp++);

        if (!islower(*temp) && *temp != '_') {
	  *bufptr = '\0';

          bufptr = buf;

	  if (bsearch(&bufptr, code_types,
	              sizeof(code_types) / sizeof(code_types[0]),
		      sizeof(code_types[0]), compare_keywords)) {
	    while (text < temp) {
	      *style++ = 'F';
	      text ++;
	      length --;
	      col ++;
	    }

	    text --;
	    length ++;
	    last = 1;
	    continue;
	  } else if (bsearch(&bufptr, code_keywords,
	                     sizeof(code_keywords) / sizeof(code_keywords[0]),
		             sizeof(code_keywords[0]), compare_keywords)) {
	    while (text < temp) {
	      *style++ = 'G';
	      text ++;
	      length --;
	      col ++;
	    }

	    text --;
	    length ++;
	    last = 1;
	    continue;
	  }
	}
      }
    } else if (current == 'C' && strncmp(text, "*/", 2) == 0) {
      // Close a C comment...
      *style++ = current;
      *style++ = current;
      text ++;
      length --;
      current = 'A';
      col += 2;
      continue;
    } else if (current == 'D') {
      // Continuing in string...
      if (strncmp(text, "\\\"", 2) == 0) {
        // Quoted end quote...
	*style++ = current;
	*style++ = current;
	text ++;
	length --;
	col += 2;
	continue;
      } else if (*text == '\"') {
        // End quote...
	*style++ = current;
	col ++;
	current = 'A';
	continue;
      }
    }

    // Copy style info...
    if (current == 'A' && (*text == '{' || *text == '}')) *style++ = 'G';
    else *style++ = current;
    col ++;

    last = isalnum(*text) || *text == '_' || *text == '.';

    if (*text == '\n') {
      // Reset column and possibly reset the style
      col = 0;
      if (current == 'B' || current == 'E') current = 'A';
    }
  }
}


//
// 'style_init()' - Initialize the style buffer...
//

void
style_init(void) {
  char *style = new char[textbuf->length() + 1];
  const char *text = textbuf->text();

  memset(style, 'A', textbuf->length());
  style[textbuf->length()] = '\0';

  if (!stylebuf) stylebuf = new fltk::TextBuffer(textbuf->length());

  style_parse(text, style, textbuf->length());

  stylebuf->text(style);
  delete[] style;
}


//
// 'style_unfinished_cb()' - Update unfinished styles.
//

void
style_unfinished_cb(int, void*) {
}


//
// 'style_update()' - Update the style buffer...
//

void
style_update(int        pos,		// I - Position of update
             int        nInserted,	// I - Number of inserted chars
	     int        nDeleted,	// I - Number of deleted chars
             int        /*nRestyled*/,	// I - Number of restyled chars
	     const char * /*deletedText*/,// I - Text that was deleted
             void       *cbArg) {	// I - Callback data
  int	start,				// Start of text
	end;				// End of text
  char	last,				// Last style on line
	*style,				// Style data
	*text;				// Text data


  // If this is just a selection change, just unselect the style buffer...
  if (nInserted == 0 && nDeleted == 0) {
    stylebuf->unselect();
    return;
  }

  // Track changes in the text buffer...
  if (nInserted > 0) {
    // Insert characters into the style buffer...
    style = new char[nInserted + 1];
    memset(style, 'A', nInserted);
    style[nInserted] = '\0';

    stylebuf->replace(pos, pos + nDeleted, style);
    delete[] style;
  } else {
    // Just delete characters in the style buffer...
    stylebuf->remove(pos, pos + nDeleted);
  }

  // Select the area that was just updated to avoid unnecessary
  // callbacks...
  stylebuf->select(pos, pos + nInserted - nDeleted);

  // Re-parse the changed region; we do this by parsing from the
  // beginning of the previous line of the changed region to the end of
  // the line of the changed region...  Then we check the last
  // style character and keep updating if we have a multi-line
  // comment character...
  start = textbuf->line_start(pos);
//  if (start > 0) start = textbuf->line_start(start - 1);
  end   = textbuf->line_end(pos + nInserted);
  text  = textbuf->text_range(start, end);
  style = stylebuf->text_range(start, end);
  if (start==end)
    last = 0;
  else
    last  = style[end - start - 1];

//  printf("start = %d, end = %d, text = \"%s\", style = \"%s\", last='%c'...\n",
//         start, end, text, style, last);

  style_parse(text, style, end - start);

//  printf("new style = \"%s\", new last='%c'...\n",
//         style, style[end - start - 1]);

  stylebuf->replace(start, end, style);
  ((fltk::TextEditor *)cbArg)->redisplay_range(start, end);

  if (start==end || last != style[end - start - 1]) {
//    printf("Recalculate the rest of the buffer style\n");
    // Either the user deleted some text, or the last character
    // on the line changed styles, so reparse the
    // remainder of the buffer...
    free(text);
    free(style);

    end   = textbuf->length();
    text  = textbuf->text_range(start, end);
    style = stylebuf->text_range(start, end);

    style_parse(text, style, end - start);

    stylebuf->replace(start, end, style);
    ((fltk::TextEditor *)cbArg)->redisplay_range(start, end);
  }

  free(text);
  free(style);
}


// Editor window functions and class...
void save_cb();
void saveas_cb();
void find2_cb(fltk::Widget*, void*);
void replall_cb(fltk::Widget*, void*);
void replace2_cb(fltk::Widget*, void*);
void replcan_cb(fltk::Widget*, void*);

class EditorWindow : public fltk::Window {
  public:
    EditorWindow(int w, int h, const char* t);
    ~EditorWindow();

    fltk::Window          *replace_dlg;
    fltk::Input           *replace_find;
    fltk::Input           *replace_with;
    fltk::Button          *replace_all;
    fltk::ReturnButton   *replace_next;
    fltk::Button          *replace_cancel;

    fltk::TextEditor     *editor;
    char               search[256];
};

EditorWindow::EditorWindow(int w, int h, const char* t) : fltk::Window(w, h, t) {
  replace_dlg = new fltk::Window(300, 105, "Replace");
  replace_dlg->begin();
    replace_find = new fltk::Input(80, 10, 210, 25, "Find:");
    replace_find->align(fltk::ALIGN_LEFT);

    replace_with = new fltk::Input(80, 40, 210, 25, "Replace:");
    replace_with->align(fltk::ALIGN_LEFT);

    replace_all = new fltk::Button(10, 70, 90, 25, "Replace All");
    replace_all->callback((fltk::Callback *)replall_cb, this);

    replace_next = new fltk::ReturnButton(105, 70, 120, 25, "Replace Next");
    replace_next->callback((fltk::Callback *)replace2_cb, this);

    replace_cancel = new fltk::Button(230, 70, 60, 25, "Cancel");
    replace_cancel->callback((fltk::Callback *)replcan_cb, this);
  replace_dlg->end();
  replace_dlg->set_non_modal();
  editor = 0;
  *search = (char)0;
}

EditorWindow::~EditorWindow() {
  delete replace_dlg;
}

int check_save(void) {
  if (!changed) return 1;

  int r = fltk::choice("The current file has not been saved.\n"
                    "Would you like to save it now?",
                    "Don't Save", "Save", "Cancel");

  if (r == 1) {
    save_cb(); // Save the file...
    return !changed;
  }

  return (r == 2) ? 1 : 0;
}

int loading = 0;
void load_file(const char *newfile, int ipos) {
  loading = 1;
  int insert = (ipos != -1);
  changed = insert;
  if (!insert) strcpy(filename, "");
  int r;
  if (!insert) r = textbuf->loadfile(newfile);
  else r = textbuf->insertfile(newfile, ipos);
  if (r) {
    if (fltk::ask("File '%s' does not exit. Do you want to create one?", newfile))
      strcpy(filename, newfile);
    else
      strcpy(filename, "");
  } // if
  else
    if (!insert) strcpy(filename, newfile);
  loading = 0;
  textbuf->call_modify_callbacks();
}

void save_file(const char *newfile) {
  if (textbuf->savefile(newfile))
    fltk::alert("Error writing to file \'%s\':\n%s.", newfile, strerror(errno));
  else
    strcpy(filename, newfile);
  changed = 0;
  textbuf->call_modify_callbacks();
}

void copy_cb(fltk::Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  fltk::TextEditor::kf_copy(0, e->editor);
}

void cut_cb(fltk::Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  fltk::TextEditor::kf_cut(0, e->editor);
}

void delete_cb(fltk::Widget*, void*) {
  textbuf->remove_selection();
}

void find_cb(fltk::Widget* w, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  const char *val;

  val = fltk::input("Search String:", e->search);
  if (val != NULL) {
    // User entered a string - go find it!
    strcpy(e->search, val);
    find2_cb(w, v);
  }
}

void find2_cb(fltk::Widget* w, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  if (e->search[0] == '\0') {
    // Search string is blank; get a new one...
    find_cb(w, v);
    return;
  }

  int pos = e->editor->insert_position();
  int found = textbuf->search_forward(pos, e->search, &pos);
  if (found) {
    // Found a match; select and update the position...
    textbuf->select(pos, pos+strlen(e->search));
    e->editor->insert_position(pos+strlen(e->search));
    e->editor->show_insert_position();
  }
  else fltk::alert("No occurrences of \'%s\' found!", e->search);
}

void set_title(fltk::Window* w) {
  if (filename[0] == '\0') strcpy(title, "Untitled.txt");
  else {
    char *slash;
    slash = strrchr(filename, '/');
#ifdef WIN32
    if (slash == NULL) slash = strrchr(filename, '\\');
#endif
    if (slash != NULL) strcpy(title, slash + 1);
    else strcpy(title, filename);
  }

  if (changed) strcat(title, " (modified)");

  w->label(title);
}

void changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v) {
  if ((nInserted || nDeleted) && !loading) changed = 1;
  EditorWindow *w = (EditorWindow *)v;
  set_title(w);
  if (loading) w->editor->show_insert_position();
}

void new_cb(fltk::Widget*, void*) {
  if (!check_save()) return;

  filename[0] = '\0';
  textbuf->select(0, textbuf->length());
  textbuf->remove_selection();
  changed = 0;
  textbuf->call_modify_callbacks();
}

void open_cb(fltk::Widget*, void*) {
  if (!check_save()) return;

  const char *newfile = fltk::file_chooser("Open File?", "*", filename);
  if (newfile != NULL) load_file(newfile, -1);
}

void insert_cb(fltk::Widget*, void *v) {
  const char *newfile = fltk::file_chooser("Insert File?", "*", filename);
  EditorWindow *w = (EditorWindow *)v;
  if (newfile != NULL) load_file(newfile, w->editor->insert_position());
}

void paste_cb(fltk::Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  fltk::TextEditor::kf_paste(0, e->editor);
}

int num_windows = 0;

void close_cb(fltk::Widget*, void* v) {
  fltk::Window* w = (fltk::Window*)v;
  if (num_windows == 1 && !check_save()) {
    return;
  }

  w->hide();
  textbuf->remove_modify_callback(changed_cb, w);
  delete w;
  num_windows--;
  if (!num_windows) exit(0);
}

void quit_cb(fltk::Widget*, void*) {
  if (changed && !check_save())
    return;

  exit(0);
}

void replace_cb(fltk::Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  e->replace_dlg->show();
}

void replace2_cb(fltk::Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  const char *find = e->replace_find->text();
  const char *replace = e->replace_with->text();

  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    e->replace_dlg->show();
    return;
  }

  e->replace_dlg->hide();

  int pos = e->editor->insert_position();
  int found = textbuf->search_forward(pos, find, &pos);

  if (found) {
    // Found a match; update the position and replace text...
    textbuf->select(pos, pos+strlen(find));
    textbuf->remove_selection();
    textbuf->insert(pos, replace);
    textbuf->select(pos, pos+strlen(replace));
    e->editor->insert_position(pos+strlen(replace));
    e->editor->show_insert_position();
  }
  else fltk::alert("No occurrences of \'%s\' found!", find);
}

void replall_cb(fltk::Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  const char *find = e->replace_find->text();
  const char *replace = e->replace_with->text();

  find = e->replace_find->text();
  if (find[0] == '\0') {
    // Search string is blank; get a new one...
    e->replace_dlg->show();
    return;
  }

  e->replace_dlg->hide();

  e->editor->insert_position(0);
  int times = 0;

  // Loop through the whole string
  for (int found = 1; found;) {
    int pos = e->editor->insert_position();
    found = textbuf->search_forward(pos, find, &pos);

    if (found) {
      // Found a match; update the position and replace text...
      textbuf->select(pos, pos+strlen(find));
      textbuf->remove_selection();
      textbuf->insert(pos, replace);
      e->editor->insert_position(pos+strlen(replace));
      e->editor->show_insert_position();
      times++;
    }
  }

  if (times) fltk::message("Replaced %d occurrences.", times);
  else fltk::alert("No occurrences of \'%s\' found!", find);
}

void replcan_cb(fltk::Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  e->replace_dlg->hide();
}

void save_cb() {
  if (filename[0] == '\0') {
    // No filename - get one!
    saveas_cb();
    return;
  }
  else save_file(filename);
}

void saveas_cb() {
  const char *newfile = fltk::file_chooser("Save File As?", "*", filename);
  if (newfile != NULL) save_file(newfile);
}

fltk::Window* new_view();

void view_cb(fltk::Widget*, void*) {
  fltk::Window* w = new_view();
  w->show();
}

static void build_menus(fltk::MenuBar * menu, fltk::Widget *w) {
    fltk::ItemGroup * g;
    menu->user_data(w);
    menu->begin();
      g = new fltk::ItemGroup( "&File" );
      g->begin();
	new fltk::Item( "&New File",        0, (fltk::Callback *)new_cb );
	new fltk::Item( "&Open File...",    fltk::COMMAND + 'O', (fltk::Callback *)open_cb );
	new fltk::Item( "&Insert File...",  fltk::COMMAND + 'I', (fltk::Callback *)insert_cb);
	new fltk::Divider();
	new fltk::Item( "&Save File",       fltk::COMMAND + 'S', (fltk::Callback *)save_cb );
	new fltk::Item( "Save File &As...", fltk::COMMAND + fltk::SHIFT + 'S', (fltk::Callback *)saveas_cb);
	new fltk::Divider();
	new fltk::Item( "New &View", fltk::ACCELERATOR + 'V', (fltk::Callback *)view_cb, 0 );
	new fltk::Item( "&Close View", fltk::COMMAND + 'W', (fltk::Callback *)close_cb);
	new fltk::Divider();
	new fltk::Item( "E&xit", fltk::COMMAND + 'Q', (fltk::Callback *)quit_cb, 0 );
      g->end();
      g = new fltk::ItemGroup( "&Edit" );
      g->begin();
	new fltk::Item( "Cu&t",        fltk::COMMAND + 'X', (fltk::Callback *)cut_cb );
	new fltk::Item( "&Copy",       fltk::COMMAND + 'C', (fltk::Callback *)copy_cb );
	new fltk::Item( "&Paste",      fltk::COMMAND + 'V', (fltk::Callback *)paste_cb );
	new fltk::Item( "&Delete",     0, (fltk::Callback *)delete_cb );
      g->end();
      g = new fltk::ItemGroup( "&Search" );
      g->begin();
	new fltk::Item( "&Find...",       fltk::COMMAND + 'F', (fltk::Callback *)find_cb );
	new fltk::Item( "F&ind Again",    fltk::COMMAND + 'G', find2_cb );
	new fltk::Item( "&Replace...",    fltk::COMMAND + 'R', replace_cb );
	new fltk::Item( "Re&place Again", fltk::COMMAND + 'T', replace2_cb );
      g->end();
    menu->end();
}

fltk::Window* new_view() {
  EditorWindow* w = new EditorWindow(660, 400, title);
  w->begin();
    fltk::MenuBar* m = new fltk::MenuBar(0, 0, 660, 21);
    build_menus(m,w);
    w->editor = new fltk::TextEditor(0, 21, 660, 379);
    w->editor->buffer(textbuf);
    w->editor->highlight_data(stylebuf, styletable,
      sizeof(styletable) / sizeof(styletable[0]),
     'A', style_unfinished_cb, 0);
    w->editor->textfont(fltk::COURIER);
  w->end();
  w->resizable(w->editor);
  w->callback((fltk::Callback *)close_cb, w);

  w->editor->linenumber_width(60);
  w->editor->wrap_mode(true, 0);
  w->editor->cursor_style(fltk::TextDisplay::BLOCK_CURSOR);
  // w->editor->insert_mode(false);

  textbuf->add_modify_callback(style_update, w->editor);
  textbuf->add_modify_callback(changed_cb, w);
  textbuf->call_modify_callbacks();
  num_windows++;
  return w;
}

int main(int argc, char **argv) {

  textbuf = new fltk::TextBuffer(0);
  style_init();

  fltk::Window* window = new_view();

  window->show(1, argv);

  if (argc > 1) {
    window->label(" "); // Prevent from displaying "Untitled.txt" before its time...
    load_file(argv[1], -1);
  }

  return fltk::run();
}

//
// End of "$Id: editor.cxx 8662 2011-05-15 06:04:24Z bgbnbigben $".
//
