// CodeEditor.h
//
// $Id:$
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

#ifndef Fluid_CodeEditor_h
# define Fluid_CodeEditor_h

//
// Include necessary headers...
//

#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <ctype.h>
#  include <fltk/run.h>
#  include <fltk/events.h>
#  include <fltk/TextBuffer.h>
#  include <fltk/TextDisplay.h>
#  include <fltk/TextEditor.h>

// WAS: I removed the namespace as fluid tries to include the fltk
// header for any classes with fltk in their name. However it is
// quite likely that this class sould be made part of fltk...
//namespace fltk {

  class CodeEditor : public fltk::TextEditor {
    static TextDisplay::StyleTableEntry styletable[];
    static const char * const code_keywords[];
    static const char * const code_types[];


    // 'compare_keywords()' - Compare two keywords...
    static int compare_keywords(const void *a, const void *b);

    // 'style_parse()' - Parse text and produce style data.
    static void style_parse(const char *text, char *style, int length);

    // 'style_unfinished_cb()' - Update unfinished styles.
    static void style_unfinished_cb(int, void*);

    // 'style_update()' - Update the style buffer...
    static void style_update(int pos, int nInserted, int nDeleted,
                             int /*nRestyled*/, const char * /*deletedText*/,
                             void *cbArg);

    static int auto_indent(int, CodeEditor* e);

  public:

    CodeEditor(int X, int Y, int W, int H, const char *L=0);
    ~CodeEditor();
    int top_line() { return get_absolute_top_line_number(); }
  };

  class CodeViewer : public CodeEditor {
  public:

    CodeViewer(int X, int Y, int W, int H, const char *L=0);

  protected:
    int handle(int ev) { return TextDisplay::handle(ev); }
  };

  //} // namespace fltk

#endif // !CodeEditor_h

// End of $Id:$
