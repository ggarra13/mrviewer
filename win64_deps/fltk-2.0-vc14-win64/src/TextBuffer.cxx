//
// "$Id: TextBuffer.cxx 8750 2011-05-28 11:26:33Z bgbnbigben $"
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

#include <stdio.h>
#include <stdlib.h>
#include <fltk/string.h>
#include <ctype.h>
#include <fltk/events.h>
#include <fltk/ask.h>
#include <fltk/error.h>
#include <fltk/TextBuffer.h>

// Return number of bytes that a legal UTF-8 encoding starting with cc
// will use. Returns 1 if cc cannot start an encoding.
static int utf8len(char cc) {
  unsigned char c = (unsigned char)cc;
  if (c < 0xc2) return 1;
  else if (c < 0xe0) return 2;
  else if (c < 0xf0) return 3;
  else if (c < 0xf5) return 4;
  else return 1;
}

using namespace fltk;

/* Initial size for the buffer gap (empty space
   in the buffer where text might be inserted
   if the user is typing sequential chars ) */
#define PREFERRED_GAP_SIZE 80

static void insertColInLine( const char *line, char *ins_line, int column, int inswidth,
                             int tabDist, int useTabs, char nullSubsChar, char *out_str, int *out_len,
                             int *end_offset );
static void deleteRectFromLine( const char *line, int rectstart, int rectend,
                                int tabDist, int useTabs, char nullSubsChar, char *out_str, int *out_len,
                                int *end_offset );
static void overlayRectInLine( const char *line, char *ins_line, int rectstart,
                               int rectend, int tabDist, int useTabs, char nullSubsChar, char *out_str,
                               int *out_len, int *end_offset );

static void add_padding( char *string, int startIndent, int toIndent,
                        int tabDist, int useTabs, char nullSubsChar, int *charsAdded );
static char *copy_line( const char* text, int *lineLen );
static int count_lines(const char *string);
static int text_width( const char *text, int tabDist, char nullSubsChar );
static char *realign_tabs( const char *text, int origIndent, int newIndent,
                          int tabDist, int useTabs, char nullSubsChar, int *newLength );
static char *expand_tabs( const char *text, int startIndent, int tabDist,
                         char nullSubsChar, int *newLen );
static char *unexpandTabs( char *text, int startIndent, int tabDist,
                           char nullSubsChar, int *newLen );
static int max( int i1, int i2 );
static int min( int i1, int i2 );

#if 0
static const char *ControlCodeTable[32] = {
  "nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel",
  "bs", "ht", "nl", "vt", "np", "cr", "so", "si",
  "dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb",
  "can", "em", "sub", "esc", "fs", "gs", "rs", "us"};
#endif

static char* undobuffer;
static int undobufferlength;
static TextBuffer* undowidget;
static int undoat;	            /// points after insertion
static int undocut;	            /// number of characters deleted there
static int undoinsert;	        /// number of characters inserted
static int undoyankcut;	        /// length of valid contents of buffer, even if undocut=0

static void undobuffersize(int n) {
  if (n > undobufferlength) {
    if (undobuffer) {
      do {undobufferlength *= 2;} while (undobufferlength < n);
      undobuffer = (char*)realloc(undobuffer, undobufferlength);
    } else {
      undobufferlength = n+9;
      undobuffer = (char*)malloc(undobufferlength);
    }
  }
}

/**
 * Create an empty text buffer of a pre-determined size (use this to
 * avoid unnecessary re-allocation if you know exactly how much the buffer
 * will need to hold.
 */
TextBuffer::TextBuffer(int requestedsize) {
  length_ = 0;
  buf_ = (char *)malloc(requestedsize + PREFERRED_GAP_SIZE);
  gapstart_ = 0;
  gapend_   = PREFERRED_GAP_SIZE;
  tabdist_  = 8;
  usetabs_  = true;

  modifyprocs_ = NULL;
  modifycbargs_ = NULL;
  nmodifyprocs_ = 0;

  npredeleteprocs_ = 0;
  predeleteprocs_ = NULL;
  prepeletecbargs_ = NULL;

  cursorposhint_ = 0;
  nullsubschar_ = '\0';

  mCanUndo = 1;

#ifdef PURIFY
    { int i; for (i = gapstart_; i < gapend_; i++) buf_[i] = '.'; }
#endif
}

/**
 * Free a text buffer
 */
TextBuffer::~TextBuffer() {
  free(buf_);
  if (nmodifyprocs_ != 0) {
    delete[] modifyprocs_;
    delete[] modifycbargs_;
  }
  if (npredeleteprocs_ != 0) {
    delete[] predeleteprocs_;
    delete[] prepeletecbargs_;
  }
}

/**
 * Return the entire contents of the text buffer. Returned memory is
 * temporary and will only be usable until the next time the text is
 * altered.
 *
 * Unlike previous versions of fltk, DO NOT FREE THE RETURNED RESULT!
 */
const char *TextBuffer::text() {
  if (!gapstart_ && length_) {
    buf_[length_+gapend_] = 0;
    return buf_+gapend_;
  }
  if (gapstart_ < gapend_) {
    memmove(&buf_[gapstart_], &buf_[gapend_], length_-gapstart_);
    gapstart_ = gapend_ = length_;
  }
  buf_[length_] = 0; // add null terminator, assume length < buffer size!
  return buf_;
}

/**
 * Replace the entire contents of the text buffer
 */
void TextBuffer::text(const char *t) {
  if(!t) return;

  call_predelete_callbacks(0, length_);

  /* Save information for redisplay, and get rid of the old buffer */
  const char* deleted_text = text();
  int deleted_length = length_;
  char* oldbuf = buf_; // keep this until we are done w deleted_text
  int insert_length = strlen(t);

  /* Start a new buffer with a gap of PREFERRED_GAP_SIZE at end */
  buf_ = (char*)malloc(insert_length + PREFERRED_GAP_SIZE);
  length_ = gapstart_ = gapend_ = insert_length;
  strcpy(buf_, t);

  /* Zero all of the existing selections */
  update_selections(0, deleted_length, 0);

  /* Call the saved display routine(s) to update the screen */
  call_modify_callbacks(0, deleted_length, insert_length, 0, deleted_text);

  free(oldbuf);
}

/**
 * Return a copy of the text between "start" and "end" character positions
 * from text buffer "buf".  Positions start at 0, and the range does not
 * include the character pointed to by "end"
 */
char *TextBuffer::text_range(int start, int end) {
  char *text;
  int length, part1length;
  
  /* Make sure start and end are ok, and allocate memory for returned string.
     If start is bad, return "", if end is bad, adjust it. */
  if (start < 0 || start > length_) {
    text = (char*)malloc(1);
	  text[0] = '\0';
    return text;
  }
  if (end < start) {
    int temp = start;
    start = end;
    end = temp;
  }
  if (end > length_)
    end = length_;
  
  length = end - start;
  text = (char*)malloc(length+1);
  
  /* Copy the text from the buffer to the returned string */
  if (end <= gapstart_) {
      memcpy(text, &buf_[start], length);
  } else if (start >= gapstart_) {
      memcpy(text, &buf_[start+(gapend_-gapstart_)], length);
  } else {
      part1length = gapstart_ - start;
      memcpy(text, &buf_[start], part1length);
      memcpy(&text[part1length], &buf_[gapend_], length-part1length);
  }
  text[length] = '\0';
  return text;
}

/**
 * Return the character at buffer position "pos". Positions start at 0.
 */
char TextBuffer::character(int pos) {
  if (pos < 0 || pos >= length_)
    return '\0';
  if (pos < gapstart_)
    return buf_[pos];
  
  return buf_[pos + gapend_-gapstart_];
}

/**
 * Insert null-terminated string "s" at position "pos" in "buf"
 */
void TextBuffer::insert(int pos, const char *s) {
  int ninserted;
    
  /* if pos is not contiguous to existing text, make it */
  if (pos > length_) pos = length_;
  if (pos < 0 ) pos = 0;

  /* Even if nothing is deleted, we must call these callbacks */
  call_predelete_callbacks(pos, 0);

  /* insert and redisplay */
  ninserted = insert_(pos, s);
  cursorposhint_ = pos + ninserted;
  call_modify_callbacks(pos, 0, ninserted, 0, NULL);
}

/**
 * Delete the characters between "start" and "end", and insert the
 * null-terminated string "text" in their place in in "buf"
 */
void TextBuffer::replace(int start, int end, const char *s) {
  char *deleted_text;
  int ninserted;

  // Range check...
  if (!s) return;
  if (start < 0) start = 0;
  if (end > length_) end = length_;

  call_predelete_callbacks(start, end-start);
  deleted_text = text_range(start, end);
  remove_(start, end);
  //undoyankcut = undocut;
  ninserted = insert_(start, s);
  cursorposhint_ = start + ninserted;
  call_modify_callbacks(start, end - start, ninserted, 0, deleted_text);
  free(deleted_text);
}

void TextBuffer::remove(int start, int end) {
  char *deleted_text;

  /* Make sure the arguments make sense */
  if (start > end) {
    int temp = start;
    start = end;
    end = temp;
  }
  if (start > length_) start = length_;
  if (start < 0) start = 0;
  if (end > length_) end = length_;
  if (end < 0) end = 0;

  if (start == end) return;

  call_predelete_callbacks(start, end-start);
  /* Remove and redisplay */
  deleted_text = text_range(start, end);
  remove_(start, end);
  cursorposhint_ = start;
  call_modify_callbacks(start, end - start, 0, 0, deleted_text);
  free(deleted_text);
}

void TextBuffer::copy(TextBuffer *from_buf, int from_start, int from_end, int to_pos) {
  int copy_length = from_end - from_start;
  int part1length;

  /* Prepare the buffer to receive the new text.  If the new text fits in
     the current buffer, just move the gap (if necessary) to where
     the text should be inserted.  If the new text is too large, reallocate
     the buffer with a gap large enough to accomodate the new text and a
     gap of PREFERRED_GAP_SIZE */
  if (copy_length > gapend_ - gapstart_)
    reallocate_with_gap(to_pos, copy_length + PREFERRED_GAP_SIZE);
  else if (to_pos != gapstart_)
    move_gap(to_pos);
  
  /* Insert the new text (to_pos now corresponds to the start of the gap) */
  if (from_end <= from_buf->gapstart_) {
      memcpy(&buf_[to_pos], &from_buf->buf_[from_start], copy_length);
  } else if (from_start >= from_buf->gapstart_) {
      memcpy(&buf_[to_pos],
            &from_buf->buf_[from_start+(from_buf->gapend_-from_buf->gapstart_)],
            copy_length);
  } else {
      part1length = from_buf->gapstart_ - from_start;
      memcpy(&buf_[to_pos], &from_buf->buf_[from_start], part1length);
      memcpy(&buf_[to_pos+part1length], &from_buf->buf_[from_buf->gapend_],
            copy_length-part1length);
  }
  gapstart_ += copy_length;
  length_ += copy_length;
  update_selections(to_pos, 0, copy_length);
}

/**
 * Remove text according to the undo variables or insert text 
 * from the undo buffer.
 */
int TextBuffer::undo(int *cursorPos) {
  if (undowidget != this || (!undocut && !undoinsert &&!mCanUndo)) return 0;

  int ilen = undocut;
  int xlen = undoinsert;
  int b = undoat-xlen;

  if (xlen && undoyankcut && !ilen) {
    ilen = undoyankcut;
  }

  if (xlen && ilen) {
    undobuffersize(ilen+1);
    undobuffer[ilen] = 0;
    char *tmp = newstring(undobuffer);
    replace(b, undoat, tmp);
    if (cursorPos) *cursorPos = cursorposhint_;
    delete[] tmp;
  }
  else if (xlen) {
    remove(b, undoat);
    if (cursorPos) *cursorPos = cursorposhint_;
  }
  else if (ilen) {
    undobuffersize(ilen+1);
    undobuffer[ilen] = 0;
    insert(undoat, undobuffer);
    if (cursorPos) *cursorPos = cursorposhint_;
    undoyankcut = 0;
  }

  return 1;
}

/**
 * Let the undo system know if we can undo changes.
 */
void TextBuffer::canUndo(char flag) {
  mCanUndo = flag;
}

/**
 * Insert "text" columnwise into buffer starting at displayed character
 * position "column" on the line beginning at "startpos".  Opens a rectangular
 * space the width and height of "text", by moving all text to the right of
 * "column" right.  If chars_inserted and chars_deleted are not NULL, the
 * number of characters inserted and deleted in the operation (beginning
 * at startpos) are returned in these arguments.
 */
void TextBuffer::insert_column(int column, int startpos, const char *s,
                               int *chars_inserted, int *chars_deleted ) {
  int nlines, linestartpos, ndeleted, insertdeleted, ninserted;
  char *deleted_text;

  nlines = ::count_lines(s);
  linestartpos = line_start(startpos);
  ndeleted = line_end(skip_lines(startpos, nlines)) - linestartpos;
  call_predelete_callbacks(linestartpos, ndeleted);
  deleted_text = text_range(linestartpos, linestartpos + ndeleted);
  insert_column_(column, linestartpos, s, &insertdeleted, &ninserted, &cursorposhint_);
  if (ndeleted != insertdeleted)
    fltk::warning("TextBuffer::insert_column(): internal consistency check ins1 failed");
  call_modify_callbacks(linestartpos, ndeleted, ninserted, 0, deleted_text);
  free(deleted_text);
  if (chars_inserted != NULL)
    *chars_inserted = ninserted;
  if (chars_deleted != NULL)
    *chars_deleted = ndeleted;  
}

/**
 * Overlay "text" between displayed character positions "rectstart" and
 * "rectend" on the line beginning at "startpos".  If chars_inserted and
 * chars_deleted are not NULL, the number of characters inserted and deleted
 * in the operation (beginning at startpos) are returned in these arguments.
 */
void TextBuffer::overlay_rectangular(int startpos, int rectstart,
                                     int rectend, const char *s, 
                                     int *chars_inserted, int *chars_deleted) {
  int nlines, linestartpos, ndeleted, insertdeleted, ninserted;
  char *deleted_text;

  nlines = ::count_lines(s);
  linestartpos = line_start(startpos);
  if (rectend == -1)
    rectend = rectstart + text_width(s, tabdist_, nullsubschar_);
  ndeleted = line_end(skip_lines(startpos, nlines)) - linestartpos;
  call_predelete_callbacks(linestartpos, ndeleted);
  deleted_text = text_range(linestartpos, linestartpos + ndeleted);
  overlay_rectangular_(linestartpos, rectstart, rectend, s, &insertdeleted,
                       &ninserted, &cursorposhint_ );
  if ( ndeleted != insertdeleted )
    fltk::warning("TextBuffer::overlay_rectangle(): internal consistency check ovly1 failed");
  call_modify_callbacks(linestartpos, ndeleted, ninserted, 0, deleted_text);
  free(deleted_text);
  if (chars_inserted != NULL)
    *chars_inserted = ninserted;
  if (chars_deleted != NULL)
    *chars_deleted = ndeleted;
}

/**
 * Replace a rectangular area in buf, given by "start", "end", "rectstart",
 * and "rectend", with "text".  If "text" is vertically longer than the
 * rectangle, add extra lines to make room for it.
 */
void TextBuffer::replace_rectangular(int start, int end, int rectstart,
                                     int rectend, const char *s) {
  char *deleted_text;
  char *ins_text=NULL;
  int ins_len;
  int i, ninsertedlines, ndeletedlines, hint;
  int insertdeleted, insertinserted, deleteinserted;
  int lines_padded = 0;
  
  /* Make sure start and end refer to complete lines, since the
     columnar delete and insert operations will replace whole lines */
  start = line_start(start);
  end   = line_end(end);
  
  call_predelete_callbacks(start, end-start);
  
  /* If more lines will be deleted than inserted, pad the inserted text
     with newlines to make it as long as the number of deleted lines.  This
     will indent all of the text to the right of the rectangle to the same
     column.  If more lines will be inserted than deleted, insert extra
     lines in the buffer at the end of the rectangle to make room for the
     additional lines in "text" */
  ninsertedlines = ::count_lines(s);
  ndeletedlines = count_lines(start, end);
  if (ninsertedlines < ndeletedlines) {
    char *ins_ptr;

    ins_len = strlen(s);
    ins_text = (char*)malloc(ins_len + ndeletedlines - ninsertedlines + 1);
    strcpy(ins_text, s);
    ins_ptr = ins_text + ins_len;
    for (i=0; i<ndeletedlines-ninsertedlines; i++)
    	  *ins_ptr++ = '\n';
    *ins_ptr = '\0';
  } else if (ndeletedlines < ninsertedlines) {
    lines_padded = ninsertedlines-ndeletedlines;
    for (i=0; i<lines_padded; i++)
      insert_(end, "\n");
  } else /* ndeletedlines == ninsertedlines */ {
  }
  
  /* Save a copy of the text which will be modified for the modify CBs */
  deleted_text = text_range(start, end);
    	
  /* Delete then insert */
  remove_rectangular_(start, end, rectstart, rectend, &deleteinserted, &hint);
  if (ins_text) {
    insert_column_(rectstart, start, ins_text, &insertdeleted, &insertinserted, &cursorposhint_);
    free(ins_text);
  } else
    insert_column_(rectstart, start, s, &insertdeleted, &insertinserted, &cursorposhint_);
  
  /* Figure out how many chars were inserted and call modify callbacks */
  if (insertdeleted != deleteinserted + lines_padded)
    fltk::warning("TextBuffer::replace_rectangular(): internal consistency check repl1 failed");
  call_modify_callbacks(start, end-start, insertinserted, 0, deleted_text);
  free(deleted_text);
}

/**
 * Remove a rectangular swath of characters between character positions start
 * and end and horizontal displayed-character offsets rectstart and rectend.
 */
void TextBuffer::remove_rectangular(int start, int end, int rectstart, int rectend) {
  char *deleted_text;
  int ninserted;

  start = line_start(start);
  end = line_end(end);
  call_predelete_callbacks(start, end-start);
  deleted_text = text_range(start, end);
  remove_rectangular_(start, end, rectstart, rectend, &ninserted, &cursorposhint_);
  call_modify_callbacks(start, end - start, ninserted, 0, deleted_text);
  free(deleted_text);
}

/**
 * Clear a rectangular "hole" out of the buffer between character positions
 * start and end and horizontal displayed-character offsets rectstart and
 * rectend.
 */
void TextBuffer::clear_rectangular(int start, int end, int rectstart, int rectend) {
  int i, nlines;
  char *newline_str;

  nlines = count_lines(start, end);
  newline_str = (char *)malloc(nlines + 1);
  for (i = 0; i < nlines; i++)
    newline_str[i] = '\n';
  newline_str[i] = '\0';
  overlay_rectangular(start, rectstart, rectend, newline_str, NULL, NULL);
  free(newline_str);
}

char *TextBuffer::text_in_rectangle(int start, int end, int rectstart, int rectend) {
  int linestart, selLeft, selRight, len;
  char *textOut, *textIn, *out_ptr, *retabbed_str;

  start = line_start( start );
  end = line_end( end );
  textOut = (char *)malloc((end - start) + 1);
  linestart = start;
  out_ptr = textOut;
  while (linestart <= end) {
    rectangular_selection_boundaries(linestart, rectstart, rectend, &selLeft, &selRight );
    textIn = text_range(selLeft, selRight);
    len = selRight - selLeft;
    memcpy(out_ptr, textIn, len);
    free(textIn);
    out_ptr += len;
    linestart = line_end(selRight) + 1;
    *out_ptr++ = '\n';
  }
  if (out_ptr != textOut)
    out_ptr--;    /* don't leave trailing newline */
  *out_ptr = '\0';

  /* If necessary, realign the tabs in the selection as if the text were
     positioned at the left margin */
  retabbed_str = realign_tabs(textOut, rectstart, 0, tabdist_,
                            usetabs_, nullsubschar_, &len );
  free(textOut);
  return retabbed_str;
}

/**
 * Set the hardware tab distance used by all displays for this buffer,
 * and used in computing offsets for rectangular selection operations.
 */
void TextBuffer::tab_distance(int tabDist) {

  /* First call the pre-delete callbacks with the previous tab setting 
     still active. */
  call_predelete_callbacks(0, length_);
    
  /* Change the tab setting */
  tabdist_ = tabDist;

  /* Force any display routines to redisplay everything */
  call_modify_callbacks( 0, length_, length_, 0, text() );
}

void TextBuffer::select(int start, int end) {
  TextSelection old_selection = primary_;

  primary_.set(start, end);
  redisplay_selection(&old_selection, &primary_);
}

void TextBuffer::unselect() {
  TextSelection old_selection = primary_;

  primary_.selected(false);
  primary_.zerowidth(false);
  redisplay_selection(&old_selection, &primary_);
}

void TextBuffer::select_rectangular(int start, int end, int rectstart, int rectend) {
  TextSelection old_selection = primary_;

  primary_.set_rectangular(start, end, rectstart, rectend);
  redisplay_selection(&old_selection, &primary_);
}

int TextBuffer::selection_position(int *start, int *end) {
  return primary_.position(start, end);
}

int TextBuffer::selection_position(int *start, int *end, int *isRect, int *rectstart, int *rectend) {
  return primary_.position(start, end, isRect, rectstart, rectend);
}

char *TextBuffer::selection_text() {
  return selection_text_(&primary_);
}

void TextBuffer::remove_selection() {
  remove_selection_(&primary_);
}

void TextBuffer::replace_selection(const char *s) {
  replace_selection_(&primary_, s);
}

void TextBuffer::secondary_select(int start, int end) {
  TextSelection old_selection = secondary_;

  secondary_.set(start, end);
  redisplay_selection(&old_selection, &secondary_);
}

void TextBuffer::secondary_unselect() {
  TextSelection old_selection = secondary_;

  secondary_.selected(false);
  secondary_.zerowidth(false);
  redisplay_selection(&old_selection, &secondary_);
}

void TextBuffer::secondary_select_rectangular(int start, int end, int rectstart, int rectend) {
  TextSelection old_selection = secondary_;

  secondary_.set_rectangular( start, end, rectstart, rectend );
  redisplay_selection(&old_selection, &secondary_);
}

int TextBuffer::secondary_selection_position(int *start, int *end, int *isRect, int *rectstart, int *rectend) {
  return secondary_.position(start, end, isRect, rectstart, rectend);
}

char *TextBuffer::secondary_selection_text() {
  return selection_text_(&secondary_);
}

void TextBuffer::remove_secondary_selection() {
  remove_selection_( &secondary_ );
}

void TextBuffer::replace_secondary_selection( const char *s ) {
  replace_selection_( &secondary_, s );
}

void TextBuffer::highlight(int start, int end) {
  TextSelection old_selection = highlight_;

  highlight_.set( start, end );
  redisplay_selection( &old_selection, &highlight_ );
}

void TextBuffer::unhighlight() {
  TextSelection old_selection = highlight_;

  highlight_.selected(false);
  highlight_.zerowidth(false);
  redisplay_selection(&old_selection, &highlight_);
}

void TextBuffer::highlight_rectangular( int start, int end,
    int rectstart, int rectend ) {
  TextSelection old_selection = highlight_;

  highlight_.set_rectangular( start, end, rectstart, rectend );
  redisplay_selection( &old_selection, &highlight_ );
}

int TextBuffer::highlight_position(int *start, int *end, int *isRect, int *rectstart, int *rectend) {
  return highlight_.position(start, end, isRect, rectstart, rectend);
}

char *TextBuffer::highlight_text() {
  return selection_text_(&highlight_);
}

/**
 * Add a callback routine to be called when the buffer is modified
 */
void TextBuffer::add_modify_callback(Text_Modify_Cb bufModifiedCB, void *cbArg) {
  Text_Modify_Cb *newModifyProcs;
  void **newCBArgs;
  int i;

  newModifyProcs = new Text_Modify_Cb[nmodifyprocs_ + 1];
  newCBArgs = new void * [nmodifyprocs_ + 1];
  for (i = 0; i < nmodifyprocs_; i++) {
    newModifyProcs[i + 1] = modifyprocs_[i];
    newCBArgs[i + 1] = modifycbargs_[i];
  }
  if (nmodifyprocs_ != 0) {
    delete[] modifyprocs_;
    delete[] modifycbargs_;
  }
  newModifyProcs[0] = bufModifiedCB;
  newCBArgs[0] = cbArg;
  nmodifyprocs_++;
  modifyprocs_ = newModifyProcs;
  modifycbargs_ = newCBArgs;
}

void TextBuffer::remove_modify_callback(Text_Modify_Cb bufModifiedCB, void *cbArg) {
  int i, toRemove = -1;
  Text_Modify_Cb *newModifyProcs;
  void **newCBArgs;

  /* find the matching callback to remove */
  for (i = 0; i < nmodifyprocs_; i++) {
    if (modifyprocs_[i] == bufModifiedCB && modifycbargs_[i] == cbArg) {
      toRemove = i;
      break;
    }
  }
  if (toRemove == -1) {
    fltk::warning("TextBuffer::remove_modify_callback(): Can't find modify CB to remove");
    return;
  }

  /* Allocate new lists for remaining callback procs and args (if any are left) */
  nmodifyprocs_--;
  if (nmodifyprocs_ == 0) {
    nmodifyprocs_ = 0;
    delete[] modifyprocs_;
    modifyprocs_ = NULL;
    delete[] modifycbargs_;
    modifycbargs_ = NULL;
    return;
  }
  newModifyProcs = new Text_Modify_Cb [nmodifyprocs_];
  newCBArgs = new void * [nmodifyprocs_];

  /* copy out the remaining members and free the old lists */
  for (i = 0; i < toRemove; i++) {
    newModifyProcs[i] = modifyprocs_[i];
    newCBArgs[i] = modifycbargs_[i];
  }
  for (; i < nmodifyprocs_; i++) {
    newModifyProcs[i] = modifyprocs_[i + 1];
    newCBArgs[i] = modifycbargs_[i + 1];
  }
  delete[] modifyprocs_;
  delete[] modifycbargs_;
  modifyprocs_ = newModifyProcs;
  modifycbargs_ = newCBArgs;
}

/**
 * Add a callback routine to be called before text is deleted from the buffer.
 */
void TextBuffer::add_predelete_callback(Text_Predelete_Cb bufPreDeleteCB, void *cbArg) {
  Text_Predelete_Cb *newPreDeleteProcs;
  void **newCBArgs;
  int i;
  
  newPreDeleteProcs = new Text_Predelete_Cb[npredeleteprocs_ + 1];
  newCBArgs = new void * [npredeleteprocs_ + 1];
  for (i = 0; i < npredeleteprocs_; i++) {
    newPreDeleteProcs[i + 1] = predeleteprocs_[i];
    newCBArgs[i + 1] = prepeletecbargs_[i];
  }
  if (npredeleteprocs_ != 0) {
	  delete[] predeleteprocs_;
	  delete[] prepeletecbargs_;
  }
  newPreDeleteProcs[0] = bufPreDeleteCB;
  newCBArgs[0] = cbArg;
  npredeleteprocs_++;
  predeleteprocs_ = newPreDeleteProcs;
  prepeletecbargs_ = newCBArgs;
}

void TextBuffer::remove_predelete_callback(Text_Predelete_Cb bufPreDeleteCB, void *cbArg) {
    int i, toRemove = -1;
    Text_Predelete_Cb *newPreDeleteProcs;
    void **newCBArgs;

    /* find the matching callback to remove */
    for (i = 0; i < npredeleteprocs_; i++) {
    	if (predeleteprocs_[i] == bufPreDeleteCB && 
	       prepeletecbargs_[i] == cbArg) {
    	    toRemove = i;
    	    break;
    	}
    }
    if (toRemove == -1) {
      fltk::warning("TextBuffer::remove_predelete_callback(): Can't find pre-delete CB to remove");
    	return;
    }
    
    /* Allocate new lists for remaining callback procs and args (if any are left) */
    npredeleteprocs_--;
    if (npredeleteprocs_ == 0) {
    	npredeleteprocs_ = 0;
		  delete[] predeleteprocs_;
    	predeleteprocs_ = NULL;
		  delete[] prepeletecbargs_;
	    prepeletecbargs_ = NULL;
	    return;
    }
    newPreDeleteProcs = new Text_Predelete_Cb[npredeleteprocs_];
    newCBArgs = new void * [npredeleteprocs_];
    
    /* copy out the remaining members and free the old lists */
    for (i = 0; i < toRemove; i++) {
    	newPreDeleteProcs[i] = predeleteprocs_[i];
    	newCBArgs[i] = prepeletecbargs_[i];
    }
    for (; i < npredeleteprocs_; i++) {
	    newPreDeleteProcs[i] = predeleteprocs_[i+1];
    	newCBArgs[i] = prepeletecbargs_[i+1];
    }
    delete[] predeleteprocs_;
    delete[] prepeletecbargs_;
    predeleteprocs_ = newPreDeleteProcs;
    prepeletecbargs_ = newCBArgs;
}

/**
 * Return the text from the entire line containing position "pos"
 */
char *TextBuffer::line_text(int pos) {
  return text_range(line_start(pos), line_end(pos));
}

/**
 * Find the position of the start of the line containing position "pos"
 */
int TextBuffer::line_start(int pos) {
  int startpos;
  if (!findchar_backward(pos, '\n', &startpos))
    return 0;
  return startpos + 1;
}

/**
 * Find the position of the end of the line containing position "pos"
 * (which is either a pointer to the newline character ending the line,
 * or a pointer to one character beyond the end of the buffer)
 */
int TextBuffer::line_end( int pos ) {
  int endpos;
  if (!findchar_forward( pos, '\n', &endpos))
    endpos = length_;
  return endpos;
}

int TextBuffer::word_start(int pos) {
  while (pos && (isalnum(character(pos)) || character( pos ) == '_' )) {
    pos--;
  }
  if (!(isalnum(character(pos)) || character(pos) == '_')) pos++;
  return pos;
}

int TextBuffer::word_end(int pos) {
  while (pos < length_ && (isalnum(character(pos)) || character(pos) == '_')) {
    pos++;
  }
  return pos;
}

/**
 * Get a character from the text buffer expanded into it's screen
 * representation (which may be several characters for a tab or a
 * control code).  Returns the number of characters written to "out_str".
 * "indent" is the number of characters from the start of the line
 * for figuring tabs.  Output string is guranteed to be shorter or
 * equal in length to TEXT_MAX_EXP_CHAR_LEN
 */
int TextBuffer::expand_character(int pos, int indent, char *out_str) {
  return expand_character(character(pos), indent, out_str, tabdist_, nullsubschar_);
}

/**
 * Expand a single character from the text buffer into it's screen
 * representation (which may be several characters for a tab or a
 * control code).  Returns the number of characters added to "out_str".
 * "indent" is the number of characters from the start of the line
 * for figuring tabs.  Output string is guranteed to be shorter or
 * equal in length to TEXT_MAX_EXP_CHAR_LEN
 */
int TextBuffer::expand_character(char c, int indent, char *out_str, int tabDist, char nullSubsChar) {
  int i, nspaces;

  /* Convert tabs to spaces */
  if (c == '\t') {
    nspaces = tabDist - (indent % tabDist);
    for (i = 0; i < nspaces; i++)
      out_str[i] = ' ';
    return nspaces;
  }

  /* Convert control codes to readable character sequences */
  /*... is this safe with international character sets? */
  // WAS: yes it is safe with UTF-8
#if 0
  if ( ( ( unsigned char ) c ) <= 31 ) {
    sprintf( out_str, "<%s>", ControlCodeTable[( unsigned char ) c] );
    return strlen( out_str );
  } else if ( c == 127 ) {
    sprintf( out_str, "<del>" );
    return 5;
  } else if ( c == nullSubsChar ) {
    sprintf( out_str, "<nul>" );
    return 5;
  }
#else
  if ((unsigned char)c < 32 || c == 127) {
    out_str[0] = '^';
    out_str[1] = c^0x40;
    return 2;
  } else if (c == nullSubsChar) {
    out_str[0] = '^';
    out_str[1] = '@';
    return 2;
  }
#endif

  /* Otherwise, just return the character */
  *out_str = c;
  return 1;
}

/**
 * Return the length in displayed characters of character "c" expanded
 * for display (as discussed above in BufGetExpandedChar).  If the
 * buffer for which the character width is being measured is doing null
 * substitution, nullSubsChar should be passed as that character (or nul
 * to ignore).
 */
int TextBuffer::character_width(char c, int indent, int tabDist, char nullSubsChar) {
  /* Note, this code must parallel that in TextBuffer::ExpandCharacter */
  if ( c == '\t' )
    return tabDist - (indent % tabDist);
  else if ((unsigned char)c <= 31)
    return 2; //strlen( ControlCodeTable[( unsigned char ) c] ) + 2;
  else if (c == 127)
    return 2; //5;
  else if (c == nullSubsChar)
    return 2; //5;
  return 1;
}

/**
 * Count the number of displayed characters between buffer position
 * "linestartpos" and "targetpos". (displayed characters are the characters
 * shown on the screen to represent characters in the buffer, where tabs and
 * control characters are expanded)
 */
int TextBuffer::count_displayed_characters(int linestartpos, int targetpos) {
  int pos, char_count = 0;
  char expandedChar[TEXT_MAX_EXP_CHAR_LEN];

  pos = linestartpos;
  while (pos < targetpos)
    char_count += expand_character(pos++, char_count, expandedChar);
  return char_count;
}

int TextBuffer::count_displayed_characters_utf(int linestartpos, int targetpos) {
  int pos, char_count = 0;
  char expandedChar[TEXT_MAX_EXP_CHAR_LEN];

  pos = linestartpos;
  while (pos < targetpos) {
    char_count += expand_character(pos, char_count, expandedChar);
    pos += utf8len(character(pos));
  }
  return char_count;
}

/**
 * Count forward from buffer position "startpos" in displayed characters
 * (displayed characters are the characters shown on the screen to represent
 * characters in the buffer, where tabs and control characters are expanded)
 */
int TextBuffer::skip_displayed_characters(int linestartpos, int nchars) {
  int pos, char_count = 0;
  char c;

  pos = linestartpos;
  while (char_count < nchars && pos < length_) {
    c = character(pos);
    if (c == '\n')
      return pos;
    char_count += character_width(c, char_count, tabdist_, nullsubschar_);
    pos += utf8len(c);
  }
  return pos;
}

int TextBuffer::skip_displayed_characters_utf(int linestartpos, int nchars) {
  int pos, char_count = 0;
  char c;

  pos = linestartpos;
  while (char_count < nchars && pos < length_) {
    c = character(pos);
    if (c == '\n')
      return pos;
    char_count += character_width(c, char_count, tabdist_, nullsubschar_);
    pos += utf8len(c);
  }
  return pos;
}

/**
 * Count the number of newlines between startpos and endpos in buffer "buf".
 * The character at position "endpos" is not counted.
 */
int TextBuffer::count_lines(int startpos, int endpos) {
  int pos, gaplen = gapend_ - gapstart_;
  int line_count = 0;

  pos = startpos;
  while (pos < gapstart_) {
    if (pos == endpos)
      return line_count;
    if (buf_[pos++] == '\n')
      line_count++;
  }
  while (pos < length_) {
    if (pos == endpos)
      return line_count;
    if (buf_[pos++ + gaplen] == '\n')
      line_count++;
  }
  return line_count;
}

/**
 * Find the first character of the line "nlines" forward from "startpos"
 * in "buf" and return its position
 */
int TextBuffer::skip_lines(int startpos, int nlines) {
  int pos, gaplen = gapend_ - gapstart_;
  int line_count = 0;

  if (nlines == 0)
    return startpos;

  pos = startpos;
  while (pos < gapstart_) {
    if (buf_[pos++] == '\n') {
      line_count++;
      if (line_count == nlines)
        return pos;
    }
  }
  while (pos < length_) {
    if (buf_[pos++ + gaplen] == '\n') {
      line_count++;
      if (line_count >= nlines)
        return pos;
    }
  }
  return pos;
}

/**
 * Find the position of the first character of the line "nlines" backwards
 * from "startpos" (not counting the character pointed to by "startpos" if
 * that is a newline) in "buf".  nlines == 0 means find the beginning of
 * the line
 */
int TextBuffer::rewind_lines( int startpos, int nlines ) {
  int pos, gaplen = gapend_ - gapstart_;
  int line_count = -1;

  pos = startpos - 1;
  if ( pos <= 0 )
    return 0;

  while (pos >= gapstart_) {
    if (buf_[pos + gaplen] == '\n') {
      if (++line_count >= nlines)
        return pos + 1;
    }
    pos--;
  }
  while (pos >= 0) {
    if (buf_[pos] == '\n') {
      if (++line_count >= nlines)
        return pos + 1;
    }
    pos--;
  }
  return 0;
}

/**
 * Search forwards in buffer for string "searchString", starting with the
 * character "startpos", and returning the result in "foundPos"
 * returns 1 if found, 0 if not.
 */
bool TextBuffer::findchars_forward(int startpos, const char *searchChars, int *foundPos)
{
  int pos, gaplen = gapend_ - gapstart_;
  const char *c;
  
  if (!searchChars) {
    *foundPos = 0;
    return false;
  }
  
  pos = startpos;
  while (pos < gapstart_) {
    for (c=searchChars; *c!='\0'; c++) {
	    if (buf_[pos]==*c) {
        *foundPos = pos;
        return true;
      }
    }
    pos++;
  }
  while (pos < length_) {
    for (c=searchChars; *c!='\0'; c++) {
    	if (buf_[pos + gaplen]==*c) {
        *foundPos = pos;
        return true;
      }
    }
    pos++;
  }
  *foundPos = length_;
  return false;
}

bool TextBuffer::search_forward(int startpos, const char *searchString,
                               int *foundPos, bool matchCase)
{ 
  if (!searchString) return 0;
  int bp;
  const char *sp;
  while (startpos < length_)
  {
    bp = startpos;
    sp = searchString;
    do {
       if (!*sp) { 
        *foundPos = startpos; 
        return true; 
       }
    } while ((matchCase ? character(bp++) == *sp++ :
             toupper(character(bp++)) == toupper(*sp++))
             && bp <= length_);
    startpos++;
  }
  return 0;
}

/**
 * Search backwards in buffer "buf" for characters in "searchChars", starting
 * with the character BEFORE "startpos", returning the result in "foundPos"
 * returns True if found, False if not.
 */
bool TextBuffer::findchars_backward(int startpos, const char *searchChars, int *foundPos)
{
  int pos, gapLen = gapend_ - gapstart_;
  const char *c;
  
  if (startpos == 0) {
    *foundPos = 0;
    return false;
  }

  pos = startpos == 0 ? 0 : startpos - 1;
  while (pos >= gapstart_) {
    for (c=searchChars; *c!='\0'; c++) {
      if (buf_[pos + gapLen] == *c) {
        *foundPos = pos;
        return true;
      }
    }
    pos--;
  }
  while (pos >= 0) {
    for (c=searchChars; *c!='\0'; c++) {
      if (buf_[pos] == *c) {
        *foundPos = pos;
        return true;
      }
    }
    pos--;
  }
  *foundPos = 0;
  return false;
}

bool TextBuffer::search_backward(int startpos, const char *searchString,
                                 int *foundPos, bool matchCase)
{
  if (!searchString) return 0;
  int bp;
  int searchStringLen = strlen(searchString);
  const char *sp;
  while (startpos > 0)
  {
    bp = startpos-1;
    sp = searchString+searchStringLen-1;
    do {
      if (sp < searchString) { 
        *foundPos = bp+1; 
        return true; 
      }
    } while ((matchCase ? character(bp--) == *sp-- :
             toupper(character(bp--)) == toupper(*sp--))
             && bp >= 0);
    startpos--;
  }
  return false;
}

/**
 * Internal (non-redisplaying) version of BufInsert.  Returns the length of
 * text inserted (this is just strlen(text), however this calculation can be
 * expensive and the length will be required by any caller who will continue
 * on to call redisplay).  pos must be contiguous with the existing text in
 * the buffer (i.e. not past the end).
 */
int TextBuffer::insert_(int pos, const char *s) {
  int insertedLength = strlen(s);

  /* Prepare the buffer to receive the new text.  If the new text fits in
     the current buffer, just move the gap (if necessary) to where
     the text should be inserted.  If the new text is too large, reallocate
     the buffer with a gap large enough to accomodate the new text and a
     gap of PREFERRED_GAP_SIZE */
  if (insertedLength > gapend_ - gapstart_)
    reallocate_with_gap(pos, insertedLength + PREFERRED_GAP_SIZE);
  else if (pos != gapstart_)
    move_gap(pos);

  /* Insert the new text (pos now corresponds to the start of the gap) */
  memcpy(&buf_[pos], s, insertedLength);
  gapstart_ += insertedLength;
  length_ += insertedLength;
  update_selections(pos, 0, insertedLength);

  if (mCanUndo) {
    if (undowidget==this && undoat==pos && undoinsert) {
      undoinsert += insertedLength;
    } else {
      undoinsert = insertedLength;
      undoyankcut = (undoat==pos) ? undocut : 0 ;
    }
    undoat = pos+insertedLength;
    undocut = 0;
    undowidget = this;
  }

  return insertedLength;
}

/**
 * Internal (non-redisplaying) version of BufRemove.  Removes the contents
 * of the buffer between start and end (and moves the gap to the site of
 * the delete).
 */
void TextBuffer::remove_(int start, int end) {
  
  if (mCanUndo) {
    if (undowidget==this && undoat==end && undocut) {
      undobuffersize(undocut+end-start+1);
      memmove(undobuffer+end-start, undobuffer, undocut);
      undocut += end-start;
    } else {
      undocut = end-start;
      undobuffersize(undocut);
    }
    undoat = start;
    undoinsert = 0;
    undoyankcut = 0;
    undowidget = this;
  }

  /* if the gap is not contiguous to the area to remove, move it there */
  if (start > gapstart_) {
    if (mCanUndo)
      memcpy(undobuffer, buf_+(gapend_-gapstart_)+start, end-start);
    move_gap(start);
  }
  else if (end < gapstart_) {
    if (mCanUndo)
      memcpy(undobuffer, buf_+start, end-start);
    move_gap(end);
  }
  else if (mCanUndo) {
    int prelen = gapstart_ - start;
    memcpy(undobuffer, buf_+start, prelen);
    memcpy(undobuffer+prelen, buf_+gapend_, end-start-prelen);
  }

  /* expand the gap to encompass the deleted characters */
  gapend_ += end - gapstart_;
  gapstart_ -= gapstart_ - start;

  /* update the length */
  length_ -= end - start;

  /* fix up any selections which might be affected by the change */
  update_selections(start, end - start, 0);
}

/**
 * Insert a column of text without calling the modify callbacks.  Note that
 * in some pathological cases, inserting can actually decrease the size of
 * the buffer because of spaces being coalesced into tabs.  "ndeleted" and
 * "ninserted" return the number of characters deleted and inserted beginning
 * at the start of the line containing "startpos".  "endpos" returns buffer
 * position of the lower left edge of the inserted column (as a hint for
 * routines which need to set a cursor position).
 */
void TextBuffer::insert_column_(int column, int startpos, const char *ins_text,
                                int *ndeleted, int *ninserted, int *endpos) {
  int nlines, start, end, inswidth, linestart, lineend;
  int expReplLen, expInsLen, len, end_offset;
  //char *c; 
  char *out_str, *out_ptr, *exp_text, *ins_line;
  char *line, *replText;
  const char *ins_ptr;

  if (column < 0)
    column = 0;

  /* Allocate a buffer for the replacement string large enough to hold
     possibly expanded tabs in both the inserted text and the replaced
     area, as well as per line: 1) an additional 2*TEXT_MAX_EXP_CHAR_LEN
     characters for padding where tabs and control characters cross the
     column of the selection, 2) up to "column" additional spaces per
     line for padding out to the position of "column", 3) padding up
     to the width of the inserted text if that must be padded to align
     the text beyond the inserted column.  (Space for additional
     newlines if the inserted text extends beyond the end of the buffer
     is counted with the length of ins_text) */
  start = line_start(startpos);
  nlines = ::count_lines(ins_text) + 1;
  inswidth = text_width(ins_text, tabdist_, nullsubschar_ );
  end = line_end(skip_lines(start, nlines - 1));
  replText = text_range(start, end);
  exp_text = expand_tabs(replText, 0, tabdist_, nullsubschar_, &expReplLen);
  free(replText);
  free(exp_text);
  exp_text = expand_tabs(ins_text, 0, tabdist_, nullsubschar_, &expInsLen);
  free(exp_text);
  out_str = (char *)malloc(expReplLen + expInsLen +
                          nlines * (column + inswidth + TEXT_MAX_EXP_CHAR_LEN) + 1);

  /* Loop over all lines in the buffer between start and end removing the
     text between rectstart and rectend and padding appropriately.  Trim
     trailing space from line (whitespace at the ends of lines otherwise
     tends to multiply, since additional padding is added to maintain it */
  out_ptr = out_str;
  linestart = start;
  ins_ptr = ins_text;
  for (;;) {
    lineend = line_end(linestart);
    line = text_range(linestart, lineend);
    ins_line = copy_line(ins_ptr, &len);
    ins_ptr += len;
    insertColInLine(line, ins_line, column, inswidth, tabdist_,
                    usetabs_, nullsubschar_, out_ptr, &len, &end_offset);
    free(line);
    free(ins_line);
#if 0   /* Earlier comments claimed that trailing whitespace could multiply on
        the ends of lines, but insertColInLine looks like it should never
        add space unnecessarily, and this trimming interfered with
        paragraph filling, so lets see if it works without it. MWE */
    for (c = out_ptr + len - 1; c > out_ptr && isspace(*c); c--)
      len--;
#endif
    out_ptr += len;
    *out_ptr++ = '\n';
    linestart = lineend < length_ ? lineend + 1 : length_;
    if (*ins_ptr == '\0')
      break;
    ins_ptr++;
  }
  if (out_ptr != out_str)
    out_ptr--;   /* trim back off extra newline */
  *out_ptr = '\0';

  /* replace the text between start and end with the new stuff */
  remove_(start, end);
  insert_(start, out_str);
  *ninserted = out_ptr - out_str;
  *ndeleted = end - start;
  *endpos = start + ( out_ptr - out_str ) - len + end_offset;
  free(out_str);
}

/**
 * Delete a rectangle of text without calling the modify callbacks.  Returns
 * the number of characters replacing those between start and end.  Note that
 * in some pathological cases, deleting can actually increase the size of
 * the buffer because of tab expansions.  "endpos" returns the buffer position
 * of the point in the last line where the text was removed (as a hint for
 * routines which need to position the cursor after a delete operation)
 */
void TextBuffer::remove_rectangular_(int start, int end, int rectstart,
                                     int rectend, int *replaceLen, int *endpos) {
  int nlines, linestart, lineend, len, end_offset;
  char *out_str, *out_ptr, *exp_text;
  char *s, *line;

  /* allocate a buffer for the replacement string large enough to hold
     possibly expanded tabs as well as an additional  TEXT_MAX_EXP_CHAR_LEN * 2
     characters per line for padding where tabs and control characters cross
     the edges of the selection */
  start = line_start(start);
  end = line_end(end);
  nlines = count_lines(start, end) + 1;
  s = text_range(start, end);
  exp_text = expand_tabs(s, 0, tabdist_, nullsubschar_, &len);
  free(s);
  free(exp_text);
  out_str = (char *)malloc(len + nlines * TEXT_MAX_EXP_CHAR_LEN * 2 + 1);

  /* loop over all lines in the buffer between start and end removing
     the text between rectstart and rectend and padding appropriately */
  linestart = start;
  out_ptr = out_str;
  end_offset = 0;
  while (linestart <= length_ && linestart <= end) {
    lineend = line_end(linestart);
    line = text_range(linestart, lineend);
    deleteRectFromLine(line, rectstart, rectend, tabdist_,
                       usetabs_, nullsubschar_, out_ptr, &len, &end_offset);
    free(line);
    out_ptr += len;
    *out_ptr++ = '\n';
    linestart = lineend + 1;
  }
  if (out_ptr != out_str)
    out_ptr--;   /* trim back off extra newline */
  *out_ptr = '\0';

  /* replace the text between start and end with the newly created string */
  remove_(start, end);
  insert_(start, out_str);
  *replaceLen = out_ptr - out_str;
  *endpos = start + (out_ptr - out_str) - len + end_offset;
  free(out_str);
}

/**
 * Overlay a rectangular area of text without calling the modify callbacks.
 * "ndeleted" and "ninserted" return the number of characters deleted and
 * inserted beginning at the start of the line containing "startpos".
 * "endpos" returns buffer position of the lower left edge of the inserted
 * column (as a hint for routines which need to set a cursor position).
 */
void TextBuffer::overlay_rectangular_(int startpos, int rectstart,
    int rectend, const char *ins_text,
    int *ndeleted, int *ninserted,
    int *endpos) {
  int nlines, start, end, linestart, lineend;
  int expInsLen, len, end_offset;
  char *c, *out_str, *out_ptr, *exp_text, *ins_line;
  char *line;
  const char *ins_ptr;

  /* Allocate a buffer for the replacement string large enough to hold
     possibly expanded tabs in the inserted text, as well as per line: 1)
     an additional 2*TEXT_MAX_EXP_CHAR_LEN characters for padding where tabs
     and control characters cross the column of the selection, 2) up to
     "column" additional spaces per line for padding out to the position
     of "column", 3) padding up to the width of the inserted text if that
     must be padded to align the text beyond the inserted column.  (Space
     for additional newlines if the inserted text extends beyond the end
     of the buffer is counted with the length of ins_text) */
  start = line_start(startpos);
  nlines = ::count_lines( ins_text ) + 1;
  end = line_end(skip_lines(start, nlines - 1));
  exp_text = expand_tabs(ins_text, 0, tabdist_, nullsubschar_, &expInsLen);
  free(exp_text);
  out_str = (char *)malloc(end - start + expInsLen +
                           nlines * (rectend + TEXT_MAX_EXP_CHAR_LEN) + 1);

  /* Loop over all lines in the buffer between start and end overlaying the
     text between rectstart and rectend and padding appropriately.  Trim
     trailing space from line (whitespace at the ends of lines otherwise
     tends to multiply, since additional padding is added to maintain it */
  out_ptr = out_str;
  linestart = start;
  ins_ptr = ins_text;
  for (;;) {
    lineend = line_end(linestart);
    line = text_range(linestart, lineend);
    ins_line = copy_line(ins_ptr, &len);
    ins_ptr += len;
    overlayRectInLine(line, ins_line, rectstart, rectend, tabdist_,
                      usetabs_, nullsubschar_, out_ptr, &len, &end_offset);
    free(line);
    free(ins_line);
    for (c = out_ptr + len - 1; c > out_ptr && isspace(*c); c--)
      len--;
    out_ptr += len;
    *out_ptr++ = '\n';
    linestart = lineend < length_ ? lineend + 1 : length_;
    if (*ins_ptr == '\0')
      break;
    ins_ptr++;
  }
  if (out_ptr != out_str)
    out_ptr--;   /* trim back off extra newline */
  *out_ptr = '\0';

  /* replace the text between start and end with the new stuff */
  remove_(start, end);
  insert_(start, out_str);
  *ninserted = out_ptr - out_str;
  *ndeleted = end - start;
  *endpos = start + (out_ptr - out_str) - len + end_offset;
  free(out_str);
}

/**
 * Insert characters from single-line string "ins_line" in single-line string
 * "line" at "column", leaving "inswidth" space before continuing line.
 * "out_len" returns the number of characters written to "out_str", "end_offset"
 * returns the number of characters from the beginning of the string to
 * the right edge of the inserted text (as a hint for routines which need
 * to position the cursor).
 */
static void insertColInLine(const char *line, char *ins_line, int column, int inswidth,
                            int tabDist, int useTabs, char nullSubsChar, char *out_str, int *out_len,
                            int *end_offset) {
  char * c, *out_ptr, *retabbed_str;
  const char *line_ptr;
  int indent, toIndent, len, postColIndent;

  /* copy the line up to "column" */
  out_ptr = out_str;
  indent = 0;
  for (line_ptr = line; *line_ptr != '\0'; line_ptr++) {
    len = TextBuffer::character_width( *line_ptr, indent, tabDist, nullSubsChar );
    if (indent + len > column)
      break;
    indent += len;
    *out_ptr++ = *line_ptr;
  }

  /* If "column" falls in the middle of a character, and the character is a
     tab, leave it off and leave the indent short and it will get padded
     later.  If it's a control character, insert it and adjust indent
     accordingly. */
  if (indent < column && *line_ptr != '\0') {
    postColIndent = indent + len;
    if (*line_ptr == '\t')
      line_ptr++;
    else {
      *out_ptr++ = *line_ptr++;
      indent += len;
    }
  } else
    postColIndent = indent;

  /* If there's no text after the column and no text to insert, that's all */
  if ( *ins_line == '\0' && *line_ptr == '\0' ) {
    *out_len = *end_offset = out_ptr - out_str;
    return;
  }

  /* pad out to column if text is too short */
  if (indent < column) {
    add_padding( out_ptr, indent, column, tabDist, useTabs, nullSubsChar, &len );
    out_ptr += len;
    indent = column;
  }

  /* Copy the text from "ins_line" (if any), recalculating the tabs as if
     the inserted string began at column 0 to its new column destination */
  if (*ins_line != '\0') {
    retabbed_str = realign_tabs(ins_line, 0, indent, tabDist, useTabs,
                              nullSubsChar, &len);
    for (c = retabbed_str; *c != '\0'; c++) {
      *out_ptr++ = *c;
      len = TextBuffer::character_width(*c, indent, tabDist, nullSubsChar);
      indent += len;
    }
    free(retabbed_str);
  }

  /* If the original line did not extend past "column", that's all */
  if (*line_ptr == '\0') {
    *out_len = *end_offset = out_ptr - out_str;
    return;
  }

  /* Pad out to column + width of inserted text + (additional original
     offset due to non-breaking character at column) */
  toIndent = column + inswidth + postColIndent - column;
  add_padding( out_ptr, indent, toIndent, tabDist, useTabs, nullSubsChar, &len );
  out_ptr += len;
  indent = toIndent;

  /* realign tabs for text beyond "column" and write it out */
  retabbed_str = realign_tabs(line_ptr, postColIndent, indent, tabDist,
                            useTabs, nullSubsChar, &len);
  strcpy( out_ptr, retabbed_str );
  free( (void *) retabbed_str );
  *end_offset = out_ptr - out_str;
  *out_len = ( out_ptr - out_str ) + len;
}

/**
 * Remove characters in single-line string "line" between displayed positions
 * "rectstart" and "rectend", and write the result to "out_str", which is
 * assumed to be large enough to hold the returned string.  Note that in
 * certain cases, it is possible for the string to get longer due to
 * expansion of tabs.  "end_offset" returns the number of characters from
 * the beginning of the string to the point where the characters were
 * deleted (as a hint for routines which need to position the cursor).
 */
static void deleteRectFromLine(const char *line, int rectstart, int rectend,
                               int tabDist, int useTabs, char nullSubsChar, char *out_str, int *out_len,
                               int *end_offset) {
  int indent, preRectIndent, postRectIndent, len;
  const char *c;
  char *retabbed_str, *out_ptr;

  /* copy the line up to rectstart */
  out_ptr = out_str;
  indent = 0;
  for (c = line; *c != '\0'; c++) {
    if (indent > rectstart)
      break;
    len = TextBuffer::character_width(*c, indent, tabDist, nullSubsChar);
    if (indent + len > rectstart && (indent == rectstart || *c == '\t'))
      break;
    indent += len;
    *out_ptr++ = *c;
  }
  preRectIndent = indent;

  /* skip the characters between rectstart and rectend */
  for ( ; *c != '\0' && indent < rectend; c++ )
    indent += TextBuffer::character_width(*c, indent, tabDist, nullSubsChar);
  postRectIndent = indent;

  /* If the line ended before rectend, there's nothing more to do */
  if (*c == '\0') {
    *out_ptr = '\0';
    *out_len = *end_offset = out_ptr - out_str;
    return;
  }

  /* fill in any space left by removed tabs or control characters
     which straddled the boundaries */
  indent = max(rectstart + postRectIndent - rectend, preRectIndent);
  add_padding(out_ptr, preRectIndent, indent, tabDist, useTabs, nullSubsChar, &len);
  out_ptr += len;

  /* Copy the rest of the line.  If the indentation has changed, preserve
     the position of non-whitespace characters by converting tabs to
     spaces, then back to tabs with the correct offset */
  retabbed_str = realign_tabs(c, postRectIndent, indent, tabDist, useTabs, nullSubsChar, &len );
  strcpy(out_ptr, retabbed_str);
  free(retabbed_str);
  *end_offset = out_ptr - out_str;
  *out_len = (out_ptr - out_str) + len;
}

/**
 * Overlay characters from single-line string "ins_line" on single-line string
 * "line" between displayed character offsets "rectstart" and "rectend".
 * "out_len" returns the number of characters written to "out_str", "end_offset"
 * returns the number of characters from the beginning of the string to
 * the right edge of the inserted text (as a hint for routines which need
 * to position the cursor).
 */
static void overlayRectInLine(const char *line, char *ins_line, int rectstart,
                              int rectend, int tabDist, int useTabs, char nullSubsChar, char *out_str,
                              int *out_len, int *end_offset) {
  char * c, *out_ptr, *retabbed_str;
  int inIndent, out_indent, len, postRectIndent;
  const char *line_ptr;

  /* copy the line up to "rectstart" */
  out_ptr = out_str;
  inIndent = out_indent = 0;
  for ( line_ptr = line; *line_ptr != '\0'; line_ptr++ ) {
    len = TextBuffer::character_width(*line_ptr, inIndent, tabDist, nullSubsChar);
    if (inIndent + len > rectstart)
      break;
    inIndent += len;
    out_indent += len;
    *out_ptr++ = *line_ptr;
  }

  /* If "rectstart" falls in the middle of a character, and the character
     is a tab, leave it off and leave the out_indent short and it will get
     padded later.  If it's a control character, insert it and adjust
     out_indent accordingly. */
  if (inIndent < rectstart && *line_ptr != '\0') {
    if ( *line_ptr == '\t' ) {
      line_ptr++;
      inIndent += len;
    } else {
      *out_ptr++ = *line_ptr++;
      out_indent += len;
      inIndent += len;
    }
  }

  /* skip the characters between rectstart and rectend */
  postRectIndent = rectend;
  for (; *line_ptr != '\0'; line_ptr++) {
    inIndent += TextBuffer::character_width(*line_ptr, inIndent, tabDist, nullSubsChar);
    if (inIndent >= rectend) {
      line_ptr++;
      postRectIndent = inIndent;
      break;
    }
  }

  /* If there's no text after rectstart and no text to insert, that's all */
  if (*ins_line == '\0' && *line_ptr == '\0') {
    *out_len = *end_offset = out_ptr - out_str;
    return;
  }

  /* pad out to rectstart if text is too short */
  if (out_indent < rectstart) {
    add_padding(out_ptr, out_indent, rectstart, tabDist, useTabs, nullSubsChar, &len);
    out_ptr += len;
  }
  out_indent = rectstart;

  /* Copy the text from "ins_line" (if any), recalculating the tabs as if
     the inserted string began at column 0 to its new column destination */
  if (*ins_line != '\0') {
    retabbed_str = realign_tabs(ins_line, 0, rectstart, tabDist, useTabs,
                               nullSubsChar, &len);
    for (c = retabbed_str; *c != '\0'; c++) {
      *out_ptr++ = *c;
      len = TextBuffer::character_width(*c, out_indent, tabDist, nullSubsChar);
      out_indent += len;
    }
    free(retabbed_str);
  }

  /* If the original line did not extend past "rectstart", that's all */
  if (*line_ptr == '\0') {
    *out_len = *end_offset = out_ptr - out_str;
    return;
  }

  /* Pad out to rectend + (additional original offset
     due to non-breaking character at right boundary) */
  add_padding(out_ptr, out_indent, postRectIndent, tabDist, useTabs, nullSubsChar, &len);
  out_ptr += len;
  out_indent = postRectIndent;

  /* copy the text beyond "rectend" */
  strcpy( out_ptr, line_ptr );
  *end_offset = out_ptr - out_str;
  *out_len = ( out_ptr - out_str ) + strlen( line_ptr );
}

char *TextBuffer::selection_text_(TextSelection *sel) {
  int start, end, isRect, rectstart = 0, rectend = 0;
  char *s;

  /* If there's no selection, return an allocated empty string */
  if (!sel->position(&start, &end, &isRect, &rectstart, &rectend)) {
    s = (char *)malloc(1);
    *s = '\0';
    return s;
  }

  /* If the selection is not rectangular, return the selected range */
  if (isRect)
    return text_in_rectangle(start, end, rectstart, rectend);
  
  return text_range(start, end);
}

void TextBuffer::remove_selection_(TextSelection *sel) {
  int start, end, isRect, rectstart = 0, rectend = 0;

  if (!sel->position(&start, &end, &isRect, &rectstart, &rectend))
    return;
  if (isRect)
    remove_rectangular(start, end, rectstart, rectend);
  else {
    remove(start, end);
    //undoyankcut = undocut;
  }
}

void TextBuffer::replace_selection_(TextSelection *sel, const char *s) {
  int start, end, isRect, rectstart = 0, rectend = 0;
  TextSelection old_selection = *sel;

  /* If there's no selection, return */
  if (!sel->position( &start, &end, &isRect, &rectstart, &rectend))
    return;

  /* Do the appropriate type of replace */
  if (isRect)
    replace_rectangular(start, end, rectstart, rectend, s);
  else
    replace(start, end, s);

  /* Unselect (happens automatically in BufReplace, but BufReplaceRect
     can't detect when the contents of a selection goes away) */
  sel->selected(false);
  redisplay_selection(&old_selection, sel);
}

static void add_padding(char *string, int startIndent, int toIndent,
                        int tabDist, int useTabs, char nullSubsChar, int *charsAdded) {
  char *out_ptr;
  int len, indent;

  indent = startIndent;
  out_ptr = string;
  if (useTabs) {
    while (indent < toIndent) {
      len = TextBuffer::character_width('\t', indent, tabDist, nullSubsChar);
      if (len > 1 && indent + len <= toIndent) {
        *out_ptr++ = '\t';
        indent += len;
      } else {
        *out_ptr++ = ' ';
        indent++;
      }
    }
  } else {
    while (indent < toIndent) {
      *out_ptr++ = ' ';
      indent++;
    }
  }
  *charsAdded = out_ptr - string;
}

/**
 * Call the stored modify callback procedure(s) for this buffer to update the
 * changed area(s) on the screen and any other listeners.
 */
void TextBuffer::call_modify_callbacks(int pos, int ndeleted,
                                       int ninserted, int nRestyled, const char *deleted_text) {
  int i;

  for (i = 0; i < nmodifyprocs_; i++)
    (*modifyprocs_[i])(pos, ninserted, ndeleted, nRestyled, deleted_text, modifycbargs_[i]);
}

/**
 * Call the stored pre-delete callback procedure(s) for this buffer to update 
 * the changed area(s) on the screen and any other listeners.
 */
void TextBuffer::call_predelete_callbacks(int pos, int ndeleted) {
    int i;
    
    for (i=0; i<npredeleteprocs_; i++)
    	(*predeleteprocs_[i])(pos, ndeleted, prepeletecbargs_[i]);
}

/**
 * Call the stored redisplay procedure(s) for this buffer to update the
 * screen for a change in a selection.
 */
void TextBuffer::redisplay_selection(TextSelection *old_selection,
                                     TextSelection *newSelection) {
  int oldStart, oldEnd, newStart, newEnd, ch1Start, ch1End, ch2Start, ch2End;

  /* If either selection is rectangular, add an additional character to
     the end of the selection to request the redraw routines to wipe out
     the parts of the selection beyond the end of the line */
  oldStart = old_selection->start();
  newStart = newSelection->start();
  oldEnd = old_selection->end();
  newEnd = newSelection->end();
  if (old_selection->rectangular())
    oldEnd++;
  if (newSelection->rectangular())
    newEnd++;

  /* If the old or new selection is unselected, just redisplay the
     single area that is (was) selected and return */
  if (!old_selection->selected() && !newSelection->selected())
    return;
  if (!old_selection->selected()) {
    call_modify_callbacks(newStart, 0, 0, newEnd - newStart, NULL);
    return;
  }
  if (!newSelection->selected()) {
    call_modify_callbacks(oldStart, 0, 0, oldEnd - oldStart, NULL);
    return;
  }

  /* If the selection changed from normal to rectangular or visa versa, or
     if a rectangular selection changed boundaries, redisplay everything */
  if ((old_selection->rectangular() && !newSelection->rectangular()) ||
      (!old_selection->rectangular() && newSelection->rectangular()) ||
      (old_selection->rectangular() && (
           (old_selection->rectstart() != newSelection->rectstart()) ||
           (old_selection->rectend() != newSelection->rectend())))) {
    call_modify_callbacks(min(oldStart, newStart), 0, 0,
                          max(oldEnd, newEnd) - min(oldStart, newStart), NULL);
    return;
  }

  /* If the selections are non-contiguous, do two separate updates
     and return */
  if (oldEnd < newStart || newEnd < oldStart) {
    call_modify_callbacks(oldStart, 0, 0, oldEnd - oldStart, NULL);
    call_modify_callbacks(newStart, 0, 0, newEnd - newStart, NULL);
    return;
  }

  /* Otherwise, separate into 3 separate regions: ch1, and ch2 (the two
     changed areas), and the unchanged area of their intersection,
     and update only the changed area(s) */
  ch1Start = min(oldStart, newStart);
  ch2End = max(oldEnd, newEnd);
  ch1End = max(oldStart, newStart);
  ch2Start = min(oldEnd, newEnd);
  if (ch1Start != ch1End)
    call_modify_callbacks(ch1Start, 0, 0, ch1End - ch1Start, NULL);
  if (ch2Start != ch2End)
    call_modify_callbacks(ch2Start, 0, 0, ch2End - ch2Start, NULL);
}

void TextBuffer::move_gap(int pos) {
  int gaplen = gapend_ - gapstart_;

  if (pos > gapstart_)
    memmove(&buf_[gapstart_], &buf_[gapend_], pos - gapstart_);
  else
    memmove(&buf_[pos + gaplen], &buf_[pos], gapstart_ - pos);
  gapend_ += pos - gapstart_;
  gapstart_ += pos - gapstart_;
}

/**
 * reallocate the text storage in "buf" to have a gap starting at "newGapStart"
 * and a gap size of "newGapLen", preserving the buffer's current contents.
 */
void TextBuffer::reallocate_with_gap(int newGapStart, int newGapLen) {
  char *newBuf;
  int newGapEnd;

  newBuf = (char *)malloc(length_ + newGapLen);
  newGapEnd = newGapStart + newGapLen;
  if (newGapStart <= gapstart_) {
    memcpy(newBuf, buf_, newGapStart );
    memcpy(&newBuf[newGapEnd], &buf_[newGapStart], gapstart_ - newGapStart);
    memcpy(&newBuf[newGapEnd + gapstart_ - newGapStart], &buf_[gapend_], length_ - gapstart_);
  } else { /* newGapStart > gapstart_ */
    memcpy(newBuf, buf_, gapstart_);
    memcpy(&newBuf[gapstart_], &buf_[gapend_], newGapStart - gapstart_);
    memcpy(&newBuf[newGapEnd], &buf_[gapend_ + newGapStart - gapstart_], length_ - newGapStart);
  }
  free(buf_);
  buf_ = newBuf;
  gapstart_ = newGapStart;
  gapend_ = newGapEnd;
#ifdef PURIFY
  { int i; for ( i = gapstart_; i < gapend_; i++ ) buf_[i] = '.'; }
#endif
}

/**
 * Update all of the selections in "buf" for changes in the buffer's text
 */
void TextBuffer::update_selections(int pos, int ndeleted, int ninserted) {
  primary_.update(pos, ndeleted, ninserted);
  secondary_.update(pos, ndeleted, ninserted);
  highlight_.update(pos, ndeleted, ninserted);
}

/**
 * Search forwards in buffer "buf" for character "searchChar", starting
 * with the character "startpos", and returning the result in "foundPos"
 * returns 1 if found, 0 if not.  (The difference between this and
 * BufSearchForward is that it's optimized for single characters.  The
 * overall performance of the text widget is dependent on its ability to
 * count lines quickly, hence searching for a single character: newline)
 */
bool TextBuffer::findchar_forward(int startpos, char searchChar, int *foundPos) {
  int pos, gaplen = gapend_ - gapstart_;

  if (startpos < 0 || startpos >= length_) {
    *foundPos = length_;
    return false;
  }

  pos = startpos;
  while (pos < gapstart_) {
    if (buf_[pos] == searchChar) {
      *foundPos = pos;
      return true;
    }
    pos++;
  }
  while (pos < length_) {
    if (buf_[pos + gaplen] == searchChar) {
      *foundPos = pos;
      return true;
    }
    pos++;
  }
  *foundPos = length_;
  return false;
}

/**
 * Search backwards in buffer "buf" for character "searchChar", starting
 * with the character BEFORE "startpos", returning the result in "foundPos"
 * returns 1 if found, 0 if not.  (The difference between this and
 * BufSearchBackward is that it's optimized for single characters.  The
 * overall performance of the text widget is dependent on its ability to
 ** count lines quickly, hence searching for a single character: newline)
 */
bool TextBuffer::findchar_backward(int startpos, char searchChar, int *foundPos) {
  int pos, gaplen = gapend_ - gapstart_;

  if (startpos <= 0 || startpos > length_) {
    *foundPos = 0;
    return false;
  }

  pos = startpos - 1;
  while (pos >= gapstart_) {
    if (buf_[pos + gaplen] == searchChar) {
      *foundPos = pos;
      return true;
    }
    pos--;
  }
  while (pos >= 0) {
    if (buf_[pos] == searchChar) {
      *foundPos = pos;
      return true;
    }
    pos--;
  }
  *foundPos = 0;
  return false;
}

/**
 * Copy from "text" to end up to but not including newline (or end of "text")
 * and return the copy as the function value, and the length of the line in
 * "lineLen"
 */
static char *copy_line(const char *text, int *lineLen) {
  int len = 0;
  const char *c;
  char *out_str;

  for (c = text; *c != '\0' && *c != '\n'; c++)
    len++;
  out_str = (char *)malloc(len + 1);
  strncpy(out_str, text, len);
  out_str[len] = '\0';
  *lineLen = len;
  return out_str;
}

/**
 * Count the number of newlines in a null-terminated text string;
 */
static int count_lines(const char *string) {
  const char * c;
  int line_count = 0;

  for (c = string; *c != '\0'; c++)
    if (*c == '\n') line_count++;
  return line_count;
}

/**
 * Measure the width in displayed characters of string "text"
 */
static int text_width(const char *text, int tabDist, char nullSubsChar) {
  int width = 0, maxWidth = 0;
  const char *c;

  for (c = text; *c != '\0'; c++) {
    if (*c == '\n') {
      if (width > maxWidth)
        maxWidth = width;
      width = 0;
    } else
      width += TextBuffer::character_width(*c, width, tabDist, nullSubsChar);
  }
  if (width > maxWidth)
    return width;
  return maxWidth;
}

/**
 * Find the first and last character position in a line within a rectangular
 * selection (for copying).  Includes tabs which cross rectstart, but not
 * control characters which do so.  Leaves off tabs which cross rectend.
 *
 * Technically, the calling routine should convert tab characters which
 * cross the right boundary of the selection to spaces which line up with
 * the edge of the selection.  Unfortunately, the additional memory
 * management required in the parent routine to allow for the changes
 * in string size is not worth all the extra work just for a couple of
 * shifted characters, so if a tab protrudes, just lop it off and hope
 * that there are other characters in the selection to establish the right
 * margin for subsequent columnar pastes of this data.
 */
void TextBuffer::rectangular_selection_boundaries(int linestartpos, 
                                                  int rectstart, int rectend, 
                                                  int *selstart, int *selend) {
  int pos, width, indent = 0;
  char c;

  /* find the start of the selection */
  for (pos = linestartpos; pos < length_; pos++) {
    c = character(pos);
    if (c == '\n')
      break;
    width = TextBuffer::character_width(c, indent, tabdist_, nullsubschar_);
    if (indent + width > rectstart) {
      if (indent != rectstart && c != '\t') {
        pos++;
        indent += width;
      }
      break;
    }
    indent += width;
  }
  *selstart = pos;

  /* find the end */
  for (; pos < length_; pos++) {
    c = character(pos);
    if (c == '\n')
      break;
    width = TextBuffer::character_width(c, indent, tabdist_, nullsubschar_);
    indent += width;
    if (indent > rectend) {
      if (indent - width != rectend && c != '\t')
        pos++;
      break;
    }
  }
  *selend = pos;
}

/**
 * Adjust the space and tab characters from string "text" so that non-white
 * characters remain stationary when the text is shifted from starting at
 * "origIndent" to starting at "newIndent".  Returns an allocated string
 * which must be freed by the caller with XtFree.
 */
static char *realign_tabs(const char *text, int origIndent, int newIndent,
                          int tabDist, int useTabs, char nullSubsChar, int *newLength) {
  char *exp_str, *out_str;
  int len;

  /* If the tabs settings are the same, retain original tabs */
  if (origIndent % tabDist == newIndent % tabDist) {
    len = strlen(text);
    out_str = (char *)malloc(len + 1);
    strcpy(out_str, text);
    *newLength = len;
    return out_str;
  }

  /* If the tab settings are not the same, brutally convert tabs to
     spaces, then back to tabs in the new position */
  exp_str = expand_tabs(text, origIndent, tabDist, nullSubsChar, &len);
  if (!useTabs) {
    *newLength = len;
    return exp_str;
  }
  out_str = unexpandTabs(exp_str, newIndent, tabDist, nullSubsChar, newLength);
  free(exp_str);
  return out_str;
}

/**
 * Expand tabs to spaces for a block of text.  The additional parameter
 * "startIndent" if nonzero, indicates that the text is a rectangular selection
 * beginning at column "startIndent"
 */
static char *expand_tabs(const char *text, int startIndent, int tabDist,
                         char nullSubsChar, int *newLen) {
  char * out_str, *out_ptr;
  const char *c;
  int indent, len, out_len = 0;

  /* rehearse the expansion to figure out length for output string */
  indent = startIndent;
  for (c = text; *c != '\0'; c++) {
    if (*c == '\t') {
      len = TextBuffer::character_width(*c, indent, tabDist, nullSubsChar);
      out_len += len;
      indent += len;
    } else if (*c == '\n') {
      indent = startIndent;
      out_len++;
    } else {
      indent += TextBuffer::character_width(*c, indent, tabDist, nullSubsChar);
      out_len++;
    }
  }

  /* do the expansion */
  out_str = (char *)malloc(out_len + 1);
  out_ptr = out_str;
  indent = startIndent;
  for (c = text; *c != '\0'; c++) {
    if (*c == '\t') {
      len = TextBuffer::expand_character(*c, indent, out_ptr, tabDist, nullSubsChar);
      out_ptr += len;
      indent += len;
    } else if (*c == '\n') {
      indent = startIndent;
      *out_ptr++ = *c;
    } else {
      indent += TextBuffer::character_width(*c, indent, tabDist, nullSubsChar);
      *out_ptr++ = *c;
    }
  }
  out_str[out_len] = '\0';
  *newLen = out_len;
  return out_str;
}

/**
 * Convert sequences of spaces into tabs.  The threshold for conversion is
 * when 3 or more spaces can be converted into a single tab, this avoids
 * converting double spaces after a period withing a block of text.
 */
static char *unexpandTabs(char *text, int startIndent, int tabDist,
                          char nullSubsChar, int *newLen) {
  char *out_str, *out_ptr, expandedChar[TEXT_MAX_EXP_CHAR_LEN];
  const char *c;
  int indent, len;

  out_str = (char *)malloc(strlen(text) + 1);
  out_ptr = out_str;
  indent = startIndent;
  for (c = text; *c != '\0';) {
    if (*c == ' ') {
      len = TextBuffer::expand_character('\t', indent, expandedChar, tabDist, nullSubsChar);
      if (len >= 3 && !strncmp(c, expandedChar, len)) {
        c += len;
        *out_ptr++ = '\t';
        indent += len;
      } else {
        *out_ptr++ = *c++;
        indent++;
      }
    } else if (*c == '\n') {
      indent = startIndent;
      *out_ptr++ = *c++;
    } else {
      *out_ptr++ = *c++;
      indent++;
    }
  }
  *out_ptr = '\0';
  *newLen = out_ptr - out_str;
  return out_str;
}

static int max( int i1, int i2 ) {
  return i1 >= i2 ? i1 : i2;
}

static int min( int i1, int i2 ) {
  return i1 <= i2 ? i1 : i2;
}

int
TextBuffer::insertfile(const char *file, int pos, int buflen) {
  FILE *fp;
  int r;
  if (!(fp = fopen(file, "r"))) return 1;
  char *buffer = new char[buflen];
  for (; (r = fread(buffer, 1, buflen - 1, fp)) > 0; pos += r) {
    buffer[r] = '\0';
    insert(pos, buffer);
  }

  int e = ferror(fp) ? 2 : 0;
  fclose(fp);
  delete[] buffer;
  return e;
}

int
TextBuffer::outputfile(const char *file, int start, int end, int buflen) {
  FILE *fp;
  if (!(fp = fopen(file, "w"))) return 1;
  for (int n; (n = min(end - start, buflen)); start += n) {
    const char *p = text_range(start, start + n);
    int r = fwrite(p, 1, n, fp);
    free((void *)p);
    if (r != n) break;
  }

  int e = ferror(fp) ? 2 : 0;
  fclose(fp);
  return e;
}

//////////////////////////////////////////////////

TextSelection::TextSelection()
{
  selected_ = 0;
  rectangular_ = 0;
  zerowidth_ = 0;
  start_ = end_ = 0;
  rectstart_ = rectend_ = 0;
}

void TextSelection::set(int startpos, int endpos) {
  selected_ = (startpos != endpos);
  zerowidth_ = (startpos == endpos) ? 1 : 0;
  rectangular_ = 0;
  start_ = min( startpos, endpos );
  end_ = max( startpos, endpos );
}

void TextSelection::set_rectangular(int startpos, int endpos, int rectstart, int rectend) {
  selected_ = rectstart < rectend;
  zerowidth_ = (rectstart == rectend) ? 1 : 0;
  rectangular_ = 1;
  start_ = startpos;
  end_ = endpos;
  rectstart_ = rectstart;
  rectend_ = rectend;
}

int TextSelection::position(int *startpos, int *endpos) {
  if (!selected_)
    return 0;
  *startpos = start_;
  *endpos = end_;
  return 1;
}

int TextSelection::position(int *startpos, int *endpos,
                            int *isrect, int *rectstart, int *rectend) {
  if (!selected_)
    return 0;
  *isrect = zerowidth_;
  *startpos = start_;
  *endpos = end_;
  if (rectangular_) {
    *rectstart = rectstart_;
    *rectend = rectend_;
  }
  return 1;
}

/**
 * Return true if position "pos" with indentation "dispIndex" is in
 * the TextSelection.
 */
bool TextSelection::includes(int pos, int linestartpos, int dispindex) {
  return selected_ &&
         ( (!rectangular_ && pos >= start_ && pos < end_) ||
           (rectangular_ && pos >= start_ && linestartpos <= end_ &&
            dispindex >= rectstart_ && dispindex < rectend_)
         );
}

/**
 * Update an individual selection for changes in the corresponding text
 */
void TextSelection::update(int pos, int ndeleted, int ninserted) {
  if ((!selected_ && !zerowidth_) || pos > end_)
    return;
  if (pos + ndeleted <= start_) {
    start_ += ninserted - ndeleted;
    end_ += ninserted - ndeleted;
  } else if (pos <= start_ && pos + ndeleted >= end_) {
    start_ = pos;
    end_ = pos;
    selected_ = false;
    zerowidth_ = false;
  } else if (pos <= start_ && pos + ndeleted < end_) {
    start_ = pos;
    end_ = ninserted + end_ - ndeleted;
  } else if (pos < end_) {
    end_ += ninserted - ndeleted;
    if (end_ <= start_)
      selected_ = false;
  }
}


//
// End of "$Id: TextBuffer.cxx 8750 2011-05-28 11:26:33Z bgbnbigben $".
//

