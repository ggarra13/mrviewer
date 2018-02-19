//
// "$Id: TextBuffer.h 5432 2006-09-16 02:03:04Z spitzak $"
//
// Header file for TextBuffer class.
//
// Copyright 2001-2006 by Bill Spitzak and others.
// Original code Copyright Mark Edel.  Permission to distribute under
// the LGPL for the FLTK library granted by Mark Edel.
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

#ifndef _fltk_TextBuffer_h_
#define _fltk_TextBuffer_h_

#include "FL_API.h"

namespace fltk {

/* Maximum length in characters of a tab or control character expansion
   of a single buffer character */
#define TEXT_MAX_EXP_CHAR_LEN 20

class FL_API TextSelection {
public:
  TextSelection();

  void set(int start, int end);
  void set_rectangular(int start, int end, int rectstart, int rectend);
  void update(int pos, int ndeleted, int ninserted);
  bool rectangular() const { return rectangular_; }
  int start() const	    { return start_; }
  int end() const	      { return end_; }
  int rectstart() const { return rectstart_; }
  int rectend() const   { return rectend_; }
  bool selected() const { return selected_; }
  void selected(bool b) { selected_ = b; }
  bool zerowidth() const { return zerowidth_; }
  void zerowidth(bool b) { zerowidth_ = b; }
  bool includes(int pos, int lineStartPos, int dispIndex);
  int position(int* start, int* end);
  int position(int* start, int* end, int* isrect, int* rectstart, int* rectend);

protected:
  bool selected_;	/*!< True if the selection is active */
  bool rectangular_;	/*!< True if the selection is rectangular */
  bool zerowidth_;	/*!< Width 0 selections aren't "real" selections, but
                             they can be useful when creating rectangular
                             selections from the keyboard. */
  int start_;		/*!< Pos. of start of selection, or if rectangular
                             start of line containing it. */
  int end_;		/*!< Pos. of end of selection, or if rectangular
                             end of line containing it. */
  int rectstart_;	/*!< Indent of left edge of rect. selection */
  int rectend_;		/*!< Indent of right edge of rect. selection */
};


typedef void (*Text_Modify_Cb)(	int pos, int nInserted, int nDeleted,
				int nRestyled, const char* deletedText,
				void* cbArg);

typedef void (*Text_Predelete_Cb)(int pos, int nDeleted, void* cbArg);

/** TextBuffer */
class FL_API TextBuffer {
public:
  TextBuffer(int requestedsize = 0);
  ~TextBuffer();

  int length() const { return length_; }

  const char *text();
  void text(const char* text);

  char character(int pos);
  char *text_range(int start, int end);
  char *text_in_rectangle(int start, int end, int rectStart, int rectEnd);

  void insert(int pos, const char *text);
  void append(const char *t) { insert(length(), t); }
  void remove(int start, int end);
  void replace(int start, int end, const char *text);
  void copy(TextBuffer *from_buf, int from_start, int from_end, int to_pos);

  int undo(int *cp = 0);
  void canUndo(char flag = 1);

  int insertfile(const char *file, int pos, int buflen = 128*1024);
  int appendfile(const char *file, int buflen = 128*1024)
        { return insertfile(file, length(), buflen); }
  int loadfile(const char *file, int buflen = 128*1024)
        { select(0, length()); remove_selection(); return appendfile(file, buflen); }
  int outputfile(const char *file, int start, int end, int buflen = 128*1024);
  int savefile(const char *file, int buflen = 128*1024)
        { return outputfile(file, 0, length(), buflen); }

  void insert_column(int column, int startpos, const char *text,
                     int *chars_inserted, int *chars_deleted);

  void replace_rectangular(int start, int end, int rectstart, int rectend,
                           const char *text);

  void overlay_rectangular(int startpos, int rectStart, int rectEnd,
                           const char* text, int* charsInserted,
                           int* charsDeleted);

  void remove_rectangular(int start, int end, int rectStart, int rectEnd);
  void clear_rectangular(int start, int end, int rectStart, int rectEnd);
  int tab_distance() const { return tabdist_; }
  void tab_distance(int tabDist);
  
  void select(int start, int end);
  bool selected() const { return primary_.selected(); }
  void unselect();
  
  void select_rectangular(int start, int end, int rectStart, int rectEnd);
  int selection_position(int* start, int* end);

  int selection_position(int* start, int* end, int* isRect, int* rectStart,
                         int* rectEnd);

  char *selection_text();
  void remove_selection();
  void replace_selection(const char* text);
  void secondary_select(int start, int end);
  void secondary_unselect();

  void secondary_select_rectangular(int start, int end, int rectStart,
                                    int rectEnd);

  int secondary_selection_position(int* start, int* end, int* isRect,
                                   int* rectStart, int* rectEnd);

  char *secondary_selection_text();
  void remove_secondary_selection();
  void replace_secondary_selection(const char* text);
  void highlight(int start, int end);
  void unhighlight();
  void highlight_rectangular(int start, int end, int rectStart, int rectEnd);

  int highlight_position(int* start, int* end, int* isRect, int* rectStart,
                         int* rectEnd);

  char *highlight_text();
  void add_modify_callback(Text_Modify_Cb bufModifiedCB, void* cbArg);
  void remove_modify_callback(Text_Modify_Cb bufModifiedCB, void* cbArg);

  void call_modify_callbacks() { call_modify_callbacks(0, 0, 0, 0, 0); }

  void add_predelete_callback(Text_Predelete_Cb bufPredelCB, void* cbArg);
  void remove_predelete_callback(Text_Predelete_Cb predelCB, void* cbArg);

  void call_predelete_callbacks() { call_predelete_callbacks(0, 0); }

  char* line_text(int pos);
  int line_start(int pos);
  int line_end(int pos);
  int word_start(int pos);
  int word_end(int pos);
  int expand_character(int pos, int indent, char *outStr);

  static int expand_character(char c, int indent, char* outStr, int tabDist,
                              char nullSubsChar);

  static int character_width(char c, int indent, int tabDist, char nullSubsChar);
  int count_displayed_characters(int lineStartPos, int targetPos);
  int count_displayed_characters_utf(int lineStartPos, int targetPos);
  int skip_displayed_characters(int lineStartPos, int nChars);
  int skip_displayed_characters_utf(int lineStartPos, int nChars);
  int count_lines(int startPos, int endPos);
  int skip_lines(int startPos, int nLines);
  int rewind_lines(int startPos, int nLines);
  
  bool findchar_forward(int startPos, char searchChar, int* foundPos);
  bool findchar_backward(int startPos, char searchChar, int* foundPos);

  bool findchars_forward(int startpos, const char *searchChars, int *foundPos);
  bool findchars_backward(int startpos, const char *searchChars, int *foundPos);

  bool search_forward(int startPos, const char* searchString, int* foundPos,
                      bool matchCase = false);

  bool search_backward(int startPos, const char* searchString, int* foundPos,
                       bool matchCase = false);

  char null_substitution_character() { return nullsubschar_; }
  TextSelection* primary_selection() { return &primary_; }
  TextSelection* secondary_selection() { return &secondary_; }
  TextSelection* highlight_selection() { return &highlight_; }

protected:
  void call_modify_callbacks(int pos, int nDeleted, int nInserted,
                             int nRestyled, const char* deletedText);
  void call_predelete_callbacks(int pos, int nDeleted);

  int insert_(int pos, const char* text);
  void remove_(int start, int end);

  void remove_rectangular_(int start, int end, int rectStart, int rectEnd,
                           int* replaceLen, int* endPos);

  void insert_column_(int column, int startPos, const char* insText,
                      int* nDeleted, int* nInserted, int* endPos);

  void overlay_rectangular_(int startPos, int rectStart, int rectEnd,
                            const char* insText, int* nDeleted,
                            int* nInserted, int* endPos);

  void redisplay_selection(TextSelection* oldSelection,
                           TextSelection* newSelection);

  void move_gap(int pos);
  void reallocate_with_gap(int newGapStart, int newGapLen);
  char *selection_text_(TextSelection *sel);
  void remove_selection_(TextSelection *sel);
  void replace_selection_(TextSelection *sel, const char* text);

  void rectangular_selection_boundaries(int lineStartPos, int rectStart,
                                        int rectEnd, int* selStart,
                                        int* selEnd);

  void update_selections(int pos, int nDeleted, int nInserted);

  TextSelection primary_;		/* highlighted areas */
  TextSelection secondary_;
  TextSelection highlight_;

  int length_;    /*!< length of the text in the buffer (the length
                       of the buffer itself must be calculated:
                       gapend - gapstart + length) */
  char *buf_;     /*!< allocated memory where the text is stored */
  int gapstart_;  /*!< points to the first character of the gap */
  int gapend_;    /*!< points to the first char after the gap */
  
  int tabdist_;		/*!< equiv. number of characters in a tab */
  bool usetabs_;	/*!< True if buffer routines are allowed to use
    				           tabs for padding in rectangular operations */
  
  int nmodifyprocs_;            /*!< number of modify-redisplay procs attached */
  Text_Modify_Cb *modifyprocs_;	/*   modified to redisplay contents */
  void **modifycbargs_;		  /*!< caller arguments for modifyprocs_ above */

  int npredeleteprocs_;	              /*!< number of pre-delete procs attached */
  Text_Predelete_Cb	*predeleteprocs_; /*   procedure to call before text is deleted */
 	                                    /*   from the buffer; at most one is supported. */
  void **prepeletecbargs_;	          /*!< caller argument for pre-delete proc above */
  
  int cursorposhint_; /*!< hint for reasonable cursor position after
    				               a buffer modification operation */
  char nullsubschar_;	/*!< TextBuffer is based on C null-terminated strings,
    	    	    	    	   so ascii-nul characters must be substituted
				                   with something else.  This is the else, but
				                   of course, things get quite messy when you
				                   use it */

  char mCanUndo;		  /*!< if this buffer is used for attributes, it must
				                   not do any undo calls */
};

} /* namespace fltk */

#endif

//
// End of "$Id: TextBuffer.h 5432 2006-09-16 02:03:04Z spitzak $".
//

