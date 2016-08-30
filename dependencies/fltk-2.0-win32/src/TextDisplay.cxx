//
// "$Id: TextDisplay.cxx 8313 2011-01-25 15:44:40Z bgbnbigben $"
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
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <fltk/error.h>
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/TextBuffer.h>
#include <fltk/TextDisplay.h>
#include <fltk/Window.h>
#include <fltk/layout.h>
#include <fltk/utf.h>
#include <fltk/Input.h>

// Similar to utf8len in TextBuffer except it returns zero for characters
// that are illegal. Code here seems to rely on this and does not work
// without it. All this is depreciated, it should be testing for legal
// utf8 sequences by using utf8fwd and other such calls.
static int utf8seqlen(char cc) {
  unsigned char c = (unsigned char)cc;
  if (c < 0x80) return 1;
  else if (c < 0xc2) return 0; // this returns 1 in utf8len
  else if (c < 0xe0) return 2;
  else if (c < 0xf0) return 3;
  else if (c < 0xf5) return 4;
  else return 1;
}

using namespace fltk;

extern void fl_set_spot(fltk::Font *f, Widget *w, int x, int y);

#undef min
#undef max
static int inline max( int i1, int i2) {
  return i1 >= i2 ? i1 : i2;
}

static int inline min( int i1, int i2) {
  return i1 <= i2 ? i1 : i2;
}

// Text area margins.  Left & right margins should be at least 3 so that
// there is some room for the overhanging parts of the cursor!
#define TOP_MARGIN    1
#define BOTTOM_MARGIN 1
#define LEFT_MARGIN   3 
#define RIGHT_MARGIN  3

#define NO_HINT -1

/* Masks for text drawing methods.  These are or'd together to form an
   integer which describes what drawing calls to use to draw a string */
#define FILL_MASK         0x0100
#define SECONDARY_MASK    0x0200
#define PRIMARY_MASK      0x0400
#define HIGHLIGHT_MASK    0x0800
#define BG_ONLY_MASK      0x1000
#define STYLE_LOOKUP_MASK   0xff

/* Maximum displayable line length (how many characters will fit across the
   widest window).  This amount of memory is temporarily allocated from the
   stack in the draw_vline() method for drawing strings */
#define MAX_DISP_LINE_LEN 1000

static int countlines( const char *string);

TextDisplay::TextDisplay(int X, int Y, int W, int H,  const char* l)
  : Group(X, Y, W, H, l), text_area(W,H) {
  int i;

  damage_range1_start = damage_range1_end = -1;
  damage_range2_start = damage_range2_end = -1;
  dragpos_ = dragtype_ = dragging_ = 0;

  begin();
  vscrollbar = new Scrollbar(0,0,0,0);
  vscrollbar->callback((Callback*)v_scrollbar_cb, this);
  vscrollbar->set_vertical();
  vscrollbar->clear_visible();
  hscrollbar = new Scrollbar(0,0,0,0);
  hscrollbar->callback((Callback*)h_scrollbar_cb, this);
  hscrollbar->clear_visible();
  end();

  ascent_ = descent_ = 0;   
  maxsize_ = 0;
  cursor_on_ = false;
  cursor_pos_ = 0;
  cursor_oldy_ = -100;
  cursor_oldx_ = -100;
  cursor_hint_ = NO_HINT;
  cursor_style_ = NORMAL_CURSOR;
  cursor_preferred_col_ = -1;
  own_buffer = true;
  buffer_ = new TextBuffer();
  buffer_->add_modify_callback(buffer_modified_cb, this);
  buffer_->add_predelete_callback(buffer_predelete_cb, this);
  firstchar_ = 0;
  lastchar_ = 0;
  bufferlines_cnt_ = 0;
  topline_num_ = 1;
  abs_topline_num_ = 1;
  need_abs_topline_num_ = false;
  horiz_offset_ = 0;
  cursor_color_ = BLACK;
  fixed_fontwidth_ = -1;
  stylebuffer_ = 0;
  styletable_ = 0;
  numstyles_ = 0;
  visiblelines_cnt_ = 1;
  linestarts_ = new int[visiblelines_cnt_];
  linestarts_[0] = 0;
  for (i=1; i<visiblelines_cnt_; i++) {
    linestarts_[i] = -1;
  }
  suppressresync_ = false;
  nlinesdeleted_ = 0;
  unfinished_style_ = 0;
  unfinished_highlight_cb_ = 0;
  highlight_cbarg_ = 0;
  continuous_wrap_ = 0;
  wrapmargin_ = 0;
  nlinesdeleted_ = 0;

  linenumleft_ = linenumwidth_ = 0;

  // Copied from Input:
  style(Input::default_style);
  clear_flag(ALIGN_MASK);
  set_flag(ALIGN_LEFT);
  set_click_to_focus();
}

/*
** Free a text display and release its associated memory.  Note, the text
** BUFFER that the text display displays is a separate entity and is not
** freed, nor are the style buffer or style table.
*/
TextDisplay::~TextDisplay() {
  if (own_buffer) {
    delete buffer_;
  } else if (buffer_) {
    buffer_->remove_modify_callback(buffer_modified_cb, this);
    buffer_->remove_predelete_callback(buffer_predelete_cb, this);
  }
  if (linestarts_) delete[] linestarts_;
}

/*
 * Attach a text buffer to display, replacing the current buffer (if any)
 */
void TextDisplay::buffer(TextBuffer *buf) {
  if (buf == buffer_) return;

  /* If the text display is already displaying a buffer, clear it off
     of the display and remove our callback from it */
  if (own_buffer) {
    delete buffer_;
    own_buffer = 0;
    buffer_ = 0;
  } else if (buffer_ != 0) {
    buffer_modified_cb(0, 0, buffer_->length(), 0, 0, this);
    buffer_->remove_modify_callback(buffer_modified_cb, this);
    buffer_->remove_predelete_callback(buffer_predelete_cb, this);
  }

  /* Add the buffer to the display, and attach a callback to the buffer for
     receiving modification information when the buffer contents change */
  buffer_ = buf;
  if (buffer_) {
    buffer_->add_modify_callback(buffer_modified_cb, this);
    buffer_->add_predelete_callback(buffer_predelete_cb, this);

    /* Update the display */
    buffer_modified_cb(0, buf->length(), 0, 0, 0, this);
  }

  /* Relayout/redraw widget */
  relayout();
  redraw();
}

/*
 * Attach (or remove) highlight information in text display and redisplay.
 * Highlighting information consists of a style buffer which parallels the
 * normal text buffer, but codes font and color information for the display;
 * a style table which translates style buffer codes (indexed by buffer
 * character - 'A') into fonts and colors; and a callback mechanism for
 * as-needed highlighting, triggered by a style buffer entry of
 * "unfinishedStyle".  Style buffer can trigger additional redisplay during
 * a normal buffer modification if the buffer contains a primary TextSelection
 * (see extendRangeForStyleMods for more information on this protocol).
 *
 * Style buffers, tables and their associated memory are managed by the caller.
 */
void TextDisplay::highlight_data(TextBuffer *styleBuffer,
				 StyleTableEntry *styleTable,
				 int nStyles, char unfinishedStyle,
				 UnfinishedStyleCb unfinishedHighlightCB, void *cbArg) {
  stylebuffer_ = styleBuffer;
  styletable_ = styleTable;
  numstyles_ = nStyles;
  unfinished_style_ = unfinishedStyle;
  unfinished_highlight_cb_ = unfinishedHighlightCB;
  highlight_cbarg_ = cbArg;

  stylebuffer_->canUndo(0);

  /* Relayout/redraw widget */
  relayout();
  redraw();
}

int TextDisplay::longest_vline() {
  int longest = 0;
  for (int i = 0; i < visiblelines_cnt_; i++)
    longest = max(longest, measure_vline(i));
  return longest;
}

/*
** Change the size of the displayed text area
*/
void TextDisplay::layout() {
  
  int i, maxAscent, maxDescent;
  int fontWidth;
  unsigned ldamage = layout_damage();

  if (!(ldamage&~LAYOUT_XY)) {
    Widget::layout(); 
    return;
  }
  


  // printf("ldamage %x\n", ldamage);

  Rectangle area(w(), h());
  box()->inset(area);
    
  
  if (!buffer()) {
    Widget::layout(); 
    return;
  }

  text_area.set(area.x()+LEFT_MARGIN,
                area.y()+TOP_MARGIN, 
                area.w()-LEFT_MARGIN-RIGHT_MARGIN,
                area.h()-TOP_MARGIN-BOTTOM_MARGIN);

  fltk::setfont(textfont(), textsize());
  maxAscent = (int)fltk::getascent();
  maxDescent = (int)fltk::getdescent();
  stdfontwidth_ = (int)fltk::getwidth("W"); /* 'W' as standard font */

  if (linenumwidth_ != 0) {
    linenumleft_ = text_area.x();
    area.move_x(linenumwidth_);
    text_area.move_x(linenumwidth_);
  }

  /* If there is a (syntax highlighting) style table in use, find the new
     maximum font height for this text display */
  for (i=0; i<numstyles_; i++) {
      Font *styleFont = styletable_[i].font;
      if (styleFont != NULL) {
        fltk::setfont(styleFont, styletable_[i].size);
        if ((int)fltk::getascent() > maxAscent)
          maxAscent = (int)fltk::getascent();
        if ((int)fltk::getdescent() > maxDescent)
          maxDescent = (int)fltk::getdescent();
      }
  }
  ascent_ = maxAscent;
  descent_ = maxDescent;   
  maxsize_ = (ascent_ + descent_);
  
  /* If all of the current fonts are fixed and match in width, compute */
  fltk::setfont(textfont(), textsize());
  fontWidth = (int)fltk::getwidth("i");
  if (fontWidth != stdfontwidth_)
    fontWidth = -1;
  else {
    for (i=0; i<numstyles_; i++) {
        Font *styleFont = styletable_[i].font;
        if (styleFont != NULL) {
          fltk::setfont(styleFont, styletable_[i].size);
          int styleFontWidth = (int)fltk::getwidth("i"); 
          int styleFontWidthW = (int)fltk::getwidth("W");
          if (styleFontWidth != styleFontWidthW ||
              fontWidth != styleFontWidthW) {
            fontWidth = -1;
            break;
          }
        }
    }
  }
  fixed_fontwidth_ = fontWidth;

  int longestvline = 0;

  // try without scrollbars first
  vscrollbar->clear_visible();
  hscrollbar->clear_visible();

  for (int again = 1; again;) {
     again = 0;
    /* In continuous wrap mode, a change in width affects the total number of
       lines in the buffer, and can leave the top line number incorrect, and
       the top character no longer pointing at a valid line start */
    if (continuous_wrap_ && !wrapmargin_ && ldamage&LAYOUT_W) {
      int oldFirstChar = firstchar_;
      bufferlines_cnt_ = count_lines(0, buffer()->length(), true);
      firstchar_ = line_start(firstchar_);
      topline_num_ = count_lines(0, firstchar_, true)+1;
      absolute_top_line_number(oldFirstChar);
    }
 
    /* reallocate and update the line starts array, which may have changed
       size and / or contents.  */
    int nvlines = (text_area.h() + maxsize_ - 1) / maxsize_;
    if (nvlines < 1) nvlines = 1;
    if (visiblelines_cnt_ < nvlines) {
      if (linestarts_) delete[] linestarts_;
      linestarts_ = new int [nvlines];
    }
    visiblelines_cnt_ = nvlines;

    calc_line_starts(0, visiblelines_cnt_);
    calc_last_char();

    longestvline = longest_vline();

    // figure the scrollbars
    if (scrollbar_width()) {
      /* Decide if the vertical scroll bar needs to be visible */
      if (scrollbar_align() & (fltk::ALIGN_LEFT|fltk::ALIGN_RIGHT) &&
          bufferlines_cnt_ >= visiblelines_cnt_ - 1)
      {
        if (!vscrollbar->visible()) {
          vscrollbar->set_visible();    
          if (continuous_wrap_ && !wrapmargin_)
            again = 1;
        }
        
        if (scrollbar_align() & fltk::ALIGN_LEFT) {
          text_area.x(area.x()+scrollbar_width()+LEFT_MARGIN);
          text_area.w(area.w()-scrollbar_width()-LEFT_MARGIN-RIGHT_MARGIN);
          vscrollbar->resize(area.x(), text_area.y()-TOP_MARGIN, scrollbar_width(),
                              text_area.h()+TOP_MARGIN+BOTTOM_MARGIN);
        } else {
          text_area.x(area.x()+LEFT_MARGIN);
          text_area.w(area.w()-scrollbar_width()-LEFT_MARGIN-RIGHT_MARGIN);
          vscrollbar->resize(area.x()+area.w()-scrollbar_width(), text_area.y()-TOP_MARGIN,
                              scrollbar_width(), text_area.h()+TOP_MARGIN+BOTTOM_MARGIN);
        }

      }

      /*
         Decide if the horizontal scroll bar needs to be visible.  If there
         is a vertical scrollbar, a horizontal is always created too.  This
         is because the alternatives are unatractive:
          * Dynamically creating a horizontal scrollbar based on the currently
            visible lines is what the original nedit does, but it always wastes
            space for the scrollbar even when it's not used.  Since the FLTK
            widget dynamically allocates the space for the scrollbar and
            rearranges the widget to make room for it, this would create a very
            visually displeasing "bounce" effect when the vertical scrollbar is
            dragged.  Trust me, I tried it and it looks really bad.
          * The other alternative would be to keep track of what the longest
            line in the entire buffer is and base the scrollbar on that.  I
            didn't do this because I didn't see any easy way to do that using
            the nedit code and this could involve a lengthy calculation for
            large buffers.  If an efficient and non-costly way of doing this
            can be found, this might be a way to go.
      */
      /* WAS: Suggestion: Try turning the horizontal scrollbar on when
	 you first see a line that is too wide in the window, but then
	 don't turn it off (ie mix both of your solutions). */
      if ((!continuous_wrap_ || (continuous_wrap_ && wrapmargin_)) &&
          scrollbar_align() & (fltk::ALIGN_TOP|fltk::ALIGN_BOTTOM) && 
          (vscrollbar->visible() || longestvline > text_area.w()))
      {
        if (!hscrollbar->visible()) {
          hscrollbar->set_visible();
          again = 1; // loop again to see if we now need vert. & recalc sizes
        }
        if (scrollbar_align() & fltk::ALIGN_TOP) {
          text_area.y(area.y() + scrollbar_width()+TOP_MARGIN);
          text_area.h(area.h() - scrollbar_width()-TOP_MARGIN-BOTTOM_MARGIN);
          hscrollbar->resize(text_area.x()-LEFT_MARGIN, area.y(),
                              text_area.w()+LEFT_MARGIN+RIGHT_MARGIN, scrollbar_width());
        } else {
          text_area.y(area.y()+TOP_MARGIN);
          text_area.h(area.h() - scrollbar_width()-TOP_MARGIN-BOTTOM_MARGIN);
          hscrollbar->resize(text_area.x()-LEFT_MARGIN, area.y()+area.h()-scrollbar_width(),
                              text_area.w()+LEFT_MARGIN+RIGHT_MARGIN, scrollbar_width());
        }
      }
    }
  }

  // everything will fit in the viewport
  if (bufferlines_cnt_ < visiblelines_cnt_-1 || buffer_ == NULL || buffer_->length() == 0) {
    scroll_(1, horiz_offset_);
  }
  // if empty lines become visible, there may be an opportunity to
  // display more text by scrolling down:
  else while (linestarts_[visiblelines_cnt_-2] == -1) {
    scroll_(topline_num_-1, horiz_offset_);
  }
  // in case horizontal offset is now greater than longest line
  int maxhoffset = max(0, longestvline-text_area.w());
  if (horiz_offset_ > maxhoffset) {
    scroll_(topline_num_, maxhoffset);
  }

  update_v_scrollbar();
  update_h_scrollbar(longestvline);

  // clear the layout flag
  Widget::layout();
}

/*
 * Refresh a rectangle of the text display.  left and top are in coordinates of
 * the text drawing window
 */
void TextDisplay::draw_text(int left, int top, int width, int height) {
  int fontHeight, firstLine, lastLine, line;

  /* find the line number range of the display */
  fontHeight = maxsize_;
  firstLine = (top - text_area.y() - fontHeight + 1) / fontHeight;
  lastLine = (top + height - text_area.y()) / fontHeight + 1;

  /* draw the lines */
  for (line = firstLine; line <= lastLine; line++) {
    draw_vline(line, left, left + width, 0, INT_MAX);
  }
}

void TextDisplay::redisplay_range(int startpos, int endpos) {

  startpos = find_prev_char(startpos-1);
  endpos   = find_next_char(endpos+1);

  if (damage_range1_start == -1 && damage_range1_end == -1) {
    damage_range1_start = startpos;
    damage_range1_end = endpos;
  } else if ((startpos >= damage_range1_start && startpos <= damage_range1_end) ||
	     (endpos >= damage_range1_start && endpos <= damage_range1_end)) {
    damage_range1_start = min(damage_range1_start, startpos);
    damage_range1_end = max(damage_range1_end, endpos);
  } else if (damage_range2_start == -1 && damage_range2_end == -1) {
    damage_range2_start = startpos;
    damage_range2_end = endpos;
  } else {
    damage_range2_start = min(damage_range2_start, startpos);
    damage_range2_end = max(damage_range2_end, endpos);
  }
  redraw(DAMAGE_SCROLL);
}

/*
 * Refresh all of the text between buffer positions "start" and "end"
 * not including the character at the position "end".
 * If end points beyond the end of the buffer, refresh the whole display
 * after pos, including blank lines which are not technically part of
 * any range of characters.
 */
void TextDisplay::draw_range(int startpos, int endpos) {
  int i, startLine, lastLine, startIndex, endIndex;

  /* If the range is outside of the displayed text, just return */
  if (endpos < firstchar_ || ( startpos > lastchar_ &&
			       !empty_vlines())) return;

  /* Clean up the starting and ending values */
  if (startpos < 0) startpos = 0;
  if (startpos > buffer_->length()) startpos = buffer_->length();
  if (endpos < 0) endpos = 0;
  if (endpos > buffer_->length()) endpos = buffer_->length();

  /* Get the starting and ending lines */
  if (startpos < firstchar_)
    startpos = firstchar_;
  if (!position_to_line(startpos, &startLine))
    startLine = visiblelines_cnt_ - 1;
  if (endpos >= lastchar_) {
    lastLine = visiblelines_cnt_ - 1;
  } else {
    if (!position_to_line( endpos, &lastLine)) {
      /* shouldn't happen */
      lastLine = visiblelines_cnt_ - 1;
    }
  }

  /* Get the starting and ending positions within the lines */
  startIndex = linestarts_[ startLine ] == -1 ? 0 :
    startpos - linestarts_[ startLine ];
  if (endpos >= lastchar_)
    endIndex = INT_MAX;
  else if (linestarts_[lastLine] == -1)
    endIndex = 0;
  else
    endIndex = endpos - linestarts_[lastLine];

  /* If the starting and ending lines are the same, redisplay the single
     line between "start" and "end" */
  if (startLine == lastLine) {
    draw_vline(startLine, 0, INT_MAX, startIndex, endIndex);
    return;
  }

  /* Redisplay the first line from "start" */
  draw_vline(startLine, 0, INT_MAX, startIndex, INT_MAX);

  /* Redisplay the lines in between at their full width */
  for (i = startLine + 1; i < lastLine; i++)
    draw_vline( i, 0, INT_MAX, 0, INT_MAX);

  /* Redisplay the last line to "end" */
  draw_vline(lastLine, 0, INT_MAX, 0, endIndex);
}

/*
 * Set the position of the text insertion cursor for text display
 */
void TextDisplay::insert_position(int newPos) {
  /* make sure new position is ok, do nothing if it hasn't changed */
  if (newPos == cursor_pos_)
    return;
  if (newPos < 0) newPos = 0;
  if (newPos > buffer_->length()) newPos = buffer_->length();

  /* cursor movement cancels vertical cursor motion column */
  cursor_preferred_col_ = -1;

  /* erase the cursor at it's previous position */
  redisplay_range(cursor_pos_ - 1, cursor_pos_ + 1);

  cursor_pos_ = newPos;

  /* draw cursor at its new position */
  redisplay_range(cursor_pos_ - 1, cursor_pos_ + 1);
}

void TextDisplay::show_cursor(bool b) {
  cursor_on_ = b;
  redisplay_range(cursor_pos_ - 1, cursor_pos_ + 1);
}

void TextDisplay::cursor_style(int style) {
  cursor_style_ = style;
  if (cursor_on_) show_cursor();
}

void TextDisplay::linenumber_width(int width)
{
  if (linenumwidth_ != width) {
    linenumwidth_ = width;
    relayout();
    redraw();
  }
}

void TextDisplay::wrap_mode(bool wrap, int wrapMargin) {
  wrapmargin_ = wrapMargin;
  continuous_wrap_ = wrap;

  /* wrapping can change change the total number of lines, re-count */
  bufferlines_cnt_ = count_lines(0, buffer()->length(), true);

  /* changing wrap margins wrap or changing from wrapped mode to non-wrapped
     can leave the character at the top no longer at a line start, and/or
     change the line number */
  firstchar_ = line_start(firstchar_);
  topline_num_ = count_lines(0, firstchar_, true) + 1;
  reset_absolute_top_line_number();

  /* update the line starts array */
  calc_line_starts(0, visiblelines_cnt_);
  calc_last_char();

  relayout();
  redraw();
}

/*
 * Insert "text" at the current cursor location.  This has the same
 * effect as inserting the text into the buffer using BufInsert and
 * then moving the insert position after the newly inserted text, except
 * that it's optimized to do less redrawing.
 */
void TextDisplay::insert(const char *text) {
  int pos = cursor_pos_;

  cursor_hint_ = pos + strlen(text);
  buffer_->insert(pos, text);
  cursor_hint_ = NO_HINT;
}

/*
 * Insert "text" (which must not contain newlines), overstriking the current
 * cursor location.
 */
void TextDisplay::overstrike(const char *text) {
  int startPos = cursor_pos_;
  TextBuffer *buf = buffer_;
  int lineStart = buf->line_start(startPos);
  int textLen = strlen(text);
  int i, p, endPos, indent, startIndent, endIndent;
  const char *c;
  char ch, *paddedText = NULL;

  /* determine how many displayed character positions are covered */
  startIndent = buffer_->count_displayed_characters(lineStart, startPos);
  indent = startIndent;
  for (c = text; *c != '\0';) {
    indent += TextBuffer::character_width(*c, indent, buf->tab_distance(), buf->null_substitution_character());

    // UTF-8
    if (*c & 0x80)
      c += utf8seqlen(*c);
    else
      c++;
  }
  endIndent = indent;

  /* find which characters to remove, and if necessary generate additional
     padding to make up for removed control characters at the end */
  indent = startIndent;
  for (p = startPos; ;) {
    if (p == buf->length())
      break;
    ch = buf->character(p);
    if (ch == '\n')
      break;
    indent += TextBuffer::character_width(ch, indent, buf->tab_distance(), buf->null_substitution_character());

    if (indent == endIndent) {
      p = find_next_char(p+1);
      break;
    } else if (indent > endIndent) {
      if (ch != '\t') {
	p++;
	paddedText = new char [textLen + TEXT_MAX_EXP_CHAR_LEN + 1];
	strcpy(paddedText, text);
	for (i = 0; i < indent - endIndent; i++)
	  paddedText[textLen + i] = ' ';
	paddedText[textLen + i] = '\0';
      }
      break;
    }

    p = find_next_char(p+1);
  }
  endPos = p;

  cursor_hint_ = startPos + textLen;
  buf->replace(startPos, endPos, paddedText == NULL ? text : paddedText);
  cursor_hint_ = NO_HINT;
  if (paddedText != NULL)
    delete[] paddedText;
}

/**
 * Translate a buffer text position to the XY location where the top left
 * of the cursor would be positioned to point to that character.  Returns
 * 0 if the position is not displayed because it is VERTICALLY out
 * of view.  If the position is horizontally out of view, returns the
 * X coordinate where the position would be if it were visible.
 */
bool TextDisplay::position_to_xy(int pos, int *X, int *Y) {
  int charIndex, lineStartPos, fontHeight, lineLen;
  int visLineNum, charLen, outIndex, xStep, charStyle;
  char expandedChar[TEXT_MAX_EXP_CHAR_LEN];
  char *lineStr;

  /* If position is not displayed, return false */
  if (pos < firstchar_ || (pos > lastchar_ && !empty_vlines())) {
    return false;
  }

  /* Calculate Y coordinate */
  if (!position_to_line(pos, &visLineNum)) {
    return false;
  }

  if (visLineNum < 0 || visLineNum > bufferlines_cnt_) {
    return false;
  }

  fontHeight = maxsize_;
  *Y = text_area.y() + visLineNum * fontHeight;

  /* Get the text, length, and  buffer position of the line. If the position
     is beyond the end of the buffer and should be at the first position on
     the first empty line, don't try to get or scan the text  */
  lineStartPos = linestarts_[visLineNum];
  if (lineStartPos == -1) {
    *X = text_area.x() - horiz_offset_;
    return true;
  }
  lineLen = vline_length(visLineNum);
  lineStr = buffer_->text_range(lineStartPos, lineStartPos + lineLen);

  /* Step through character positions from the beginning of the line
     to "pos" to calculate the X coordinate */
  xStep = text_area.x() - horiz_offset_;
  outIndex = 0;
  for (charIndex = 0; charIndex < lineLen && charIndex < pos - lineStartPos; charIndex++) {
    charLen = TextBuffer::expand_character(lineStr[charIndex], outIndex, expandedChar,
    		                          buffer_->tab_distance(), buffer_->null_substitution_character());

    // UTF-8
    bool utf8 = false;
    if (lineStr[charIndex] & 0x80) {
      charLen = utf8seqlen(lineStr[charIndex]);
      memcpy(expandedChar, &lineStr[charIndex], charLen);
      utf8 = true;
    }

    charStyle = position_style(lineStartPos, lineLen, charIndex, outIndex);
    xStep += string_width(expandedChar, charLen, charStyle);
    // UTF-8
    if (utf8 && charLen > 1) charIndex += (charLen-1);
    outIndex += charLen;    
  }
  *X = xStep;
  free(lineStr);
  return true;
}

/*
 * Find the line number of position "pos".  Note: this only works for
 * displayed lines.  If the line is not displayed, the function returns
 * 0 (without the linestarts_ array it could turn in to very long
 * calculation involving scanning large amounts of text in the buffer).
 * If continuous wrap mode is on, returns the absolute line number (as opposed
 * to the wrapped line number which is used for scrolling).
 */
bool TextDisplay::position_to_linecol(int pos, int *lineNum, int *column) {
  
  /* In continuous wrap mode, the absolute (non-wrapped) line count is
     maintained separately, as needed.  Only return it if we're actually
     keeping track of it and pos is in the displayed text */
  if (continuous_wrap_) {
    if (!maintaining_absolute_top_line_number() ||
	pos < firstchar_ || pos > lastchar_)
      return false;
    *lineNum = abs_topline_num_ + buffer()->count_lines(firstchar_, pos);
    *column = buffer()->count_displayed_characters(buffer()->line_start(pos), pos);
    return true;
  }

  if (!position_to_line(pos, lineNum))
    return false;

  *column = buffer_->count_displayed_characters(linestarts_[*lineNum], pos);
  *lineNum += topline_num_;
  
  return true;
}

/*
 * Return 1 if position (X, Y) is inside of the primary TextSelection
 */
bool TextDisplay::in_selection(int X, int Y) {
  int row, column, pos = xy_to_position(X, Y, CHARACTER_POS);
  TextBuffer *buf = buffer_;

  xy_to_rowcol(X, Y, &row, &column, CHARACTER_POS);
  if (range_touches_selection(buf->primary_selection(), firstchar_, lastchar_))
    column = wrapped_column(row, column);
  return buf->primary_selection()->includes(pos, buf->line_start(pos), column);
}

/*
 * Correct a column number based on an unconstrained position (as returned by
 * TextDXYToUnconstrainedPosition) to be relative to the last actual newline
 * in the buffer before the row and column position given, rather than the
 * last line start created by line wrapping.  This is an adapter
 * for rectangular selections and code written before continuous wrap mode,
 * which thinks that the unconstrained column is the number of characters
 * from the last newline.  Obviously this is time consuming, because it
 * invloves character re-counting.
 */
int TextDisplay::wrapped_column(int row, int column) {
  int lineStart, dispLineStart;

  if (!continuous_wrap_ || row < 0 || row > visiblelines_cnt_)
    return column;
  dispLineStart = linestarts_[row];
  if (dispLineStart == -1)
    return column;
  lineStart = buffer()->line_start(dispLineStart);
  return column + buffer()->count_displayed_characters(lineStart, dispLineStart);
}

/*
 * Correct a row number from an unconstrained position (as returned by
 * TextDXYToUnconstrainedPosition) to a straight number of newlines from the
 * top line of the display.  Because rectangular selections are based on
 * newlines, rather than display wrapping, and anywhere a rectangular selection
 * needs a row, it needs it in terms of un-wrapped lines.
 */
int TextDisplay::wrapped_row(int row) {
  if (!continuous_wrap_ || row < 0 || row > visiblelines_cnt_)
    return row;
  return buffer()->count_lines(firstchar_, linestarts_[row]);
}

/*
 * Scroll the display to bring insertion cursor into view.
 *
 * Note: it would be nice to be able to do this without counting lines twice
 * (scroll_() counts them too) and/or to count from the most efficient
 * starting point, but the efficiency of this routine is not as important to
 * the overall performance of the text display.
 */
void TextDisplay::display_insert() {
  int hOffset, topLine, X, Y;
  int lastChar = lastchar_;
  int oldTopLine = topline_num_; // determine whether to update v_scrollbar

  /* FLTK widget shows one line too much (clipped at the bottom), so get last char from line before that */
  if (visiblelines_cnt_ > 1 && linestarts_[visiblelines_cnt_-2] != -1) {
    lastChar = line_end(linestarts_[visiblelines_cnt_-2]);
  }

  hOffset = horiz_offset_;
  topLine = topline_num_;

  if (cursor_pos_ < firstchar_) {
    topLine -= count_lines(cursor_pos_, firstchar_, false);
  } else if (cursor_pos_ > lastChar && !empty_vlines()) {
    topLine += count_lines(lastChar -
            (wrap_uses_character(lastChar) ? 0 : 1),
            cursor_pos_, false);
  } else if (cursor_pos_ == lastChar && !empty_vlines() &&
            !wrap_uses_character(lastChar)) {
    topLine++;
  } 

  if (topLine < 1) {
    fltk::warning("TextDisplay::display_insert(): internal consistency check tl1 failed\n");
    topLine = 1;
  }

  /* Find the new setting for horizontal offset (this is a bit ungraceful).
     If the line is visible, just use PositionToXY to get the position
     to scroll to, otherwise, do the vertical scrolling first, then the
     horizontal */
  if (!position_to_xy(cursor_pos_, &X, &Y)) {
    scroll_(topLine, hOffset);
    if (!position_to_xy(cursor_pos_, &X, &Y))
      return;   /* Give up, it's not worth it (but why does it fail?) */
  }
  if (X+10 > text_area.r())
    hOffset += X-text_area.r()+10;
  else if (X-10 < text_area.x())
    hOffset += X-text_area.x()-10;

  if(hOffset<0) hOffset = 0;

  /* Do the scroll */
  if (topLine != oldTopLine || hOffset != horiz_offset_) {
    scroll_(topLine, hOffset);
    update_v_scrollbar();
    update_h_scrollbar();
  }
}

void TextDisplay::show_insert_position() {
  display_insert();
}

int TextDisplay::find_next_char(int pos)
{
  char c = buffer()->character(pos);
  while ((c & 0x80) && !(c & 0x40) && pos < buffer()->length()) {
    c = buffer()->character(++pos);
  }
  return pos;
}

int TextDisplay::find_prev_char(int pos)
{
  if (pos < 1) {
    return 0;
  }
  char c = buffer()->character(pos);
  while ((c & 0x80) && !(c & 0x40) && pos >= 0) {
    c = buffer()->character(--pos);
  }
  return pos;
}

/*
 * Cursor movement functions
 */
bool TextDisplay::move_right() {
  if (cursor_pos_ >= buffer_->length())
    return false;
  insert_position(find_next_char(cursor_pos_+1));
  //insert_position(cursor_pos_ + 1);
  return true;
}

bool TextDisplay::move_left() {
  if (cursor_pos_ <= 0)
    return false;
  insert_position(find_prev_char(cursor_pos_-1));
  //insert_position(cursor_pos_ - 1);
  return true;
}

bool TextDisplay::move_up() {
  int lineStartPos, column, prevLineStartPos, newPos, visLineNum;

  /* Find the position of the start of the line.  Use the line starts array
     if possible */
  if (position_to_line(cursor_pos_, &visLineNum))
    lineStartPos = linestarts_[visLineNum];
  else {
    lineStartPos = line_start(cursor_pos_);
    visLineNum = -1;
  }
  if (lineStartPos == 0)
    return false;

  /* Decide what column to move to, if there's a preferred column use that */
  column = cursor_preferred_col_ >= 0 ? cursor_preferred_col_ :
            buffer_->count_displayed_characters_utf(lineStartPos, cursor_pos_);

  /* count forward from the start of the previous line to reach the column */
  if (visLineNum != -1 && visLineNum != 0)
    prevLineStartPos = linestarts_[visLineNum - 1];
  else
    prevLineStartPos = rewind_lines(lineStartPos, 1);

  newPos = buffer_->skip_displayed_characters_utf(prevLineStartPos, column);

  if (continuous_wrap_)
    newPos = min(newPos, line_end(prevLineStartPos, true));

  /* move the cursor */
  insert_position(find_prev_char(newPos));
  //insert_position(newPos);

  /* if a preferred column wasn't aleady established, establish it */
  cursor_preferred_col_ = column;
  return true;
}

bool TextDisplay::move_down() {
  int lineStartPos, column, nextLineStartPos, newPos, visLineNum;

  if (cursor_pos_ == buffer_->length())
    return false;
  
  if (position_to_line(cursor_pos_, &visLineNum))
    lineStartPos = linestarts_[visLineNum];
  else {
    lineStartPos = line_start(cursor_pos_);
    visLineNum = -1;
  }
  column = cursor_preferred_col_ >= 0 ? cursor_preferred_col_ :
            buffer_->count_displayed_characters_utf(lineStartPos, cursor_pos_);
  nextLineStartPos = skip_lines(lineStartPos, 1, true);
  newPos = buffer_->skip_displayed_characters_utf(nextLineStartPos, column);
  if (continuous_wrap_)
    newPos = min(newPos, line_end(nextLineStartPos, true));
  
  insert_position(find_next_char(newPos));
  //insert_position(newPos);

  cursor_preferred_col_ = column;
  return true;
}

/*
 * Same as BufCountLines, but takes in to account wrapping if wrapping is
 * turned on.  If the caller knows that startPos is at a line start, it
 * can pass "startPosIsLineStart" as True to make the call more efficient
 * by avoiding the additional step of scanning back to the last newline.
 */
int TextDisplay::count_lines(int startPos, int endPos,
			     bool startPosIsLineStart) {
  int retLines, retPos, retLineStart, retLineEnd;

  /* If we're not wrapping use simple (and more efficient) BufCountLines */
  if (!continuous_wrap_)
    return buffer()->count_lines(startPos, endPos);

  wrapped_line_counter(buffer(), startPos, endPos, INT_MAX,
		       startPosIsLineStart, 0, &retPos, &retLines, &retLineStart,
		       &retLineEnd);

  return retLines;
}

/*
 * Same as BufCountForwardNLines, but takes in to account line breaks when
 * wrapping is turned on. If the caller knows that startPos is at a line start,
 * it can pass "startPosIsLineStart" as True to make the call more efficient
 * by avoiding the additional step of scanning back to the last newline.
 */
int TextDisplay::skip_lines(int startPos, int nLines,
			    bool startPosIsLineStart) {
  int retLines, retPos, retLineStart, retLineEnd;

  /* if we're not wrapping use more efficient BufCountForwardNLines */
  if (!continuous_wrap_)
    return buffer()->skip_lines(startPos, nLines);

  /* wrappedLineCounter can't handle the 0 lines case */
  if (nLines == 0)
    return startPos;

  /* use the common line counting routine to count forward */
  wrapped_line_counter(buffer(), startPos, buffer()->length(),
		       nLines, startPosIsLineStart, 0, &retPos, &retLines, &retLineStart,
		       &retLineEnd);
  return retPos;
}

/*
 * Same as BufEndOfLine, but takes in to account line breaks when wrapping
 * is turned on.  If the caller knows that startPos is at a line start, it
 * can pass "startPosIsLineStart" as True to make the call more efficient
 * by avoiding the additional step of scanning back to the last newline.
 *
 * Note that the definition of the end of a line is less clear when continuous
 * wrap is on.  With continuous wrap off, it's just a pointer to the newline
 * that ends the line.  When it's on, it's the character beyond the last
 * DISPLAYABLE character on the line, where a whitespace character which has
 * been "converted" to a newline for wrapping is not considered displayable.
 * Also note that, a line can be wrapped at a non-whitespace character if the
 * line had no whitespace.  In this case, this routine returns a pointer to
 * the start of the next line.  This is also consistent with the model used by
 * visLineLength.
 */
int TextDisplay::line_end(int pos, bool startPosIsLineStart) {
  int retLines, retPos, retLineStart, retLineEnd;

  /* If we're not wrapping use more efficien BufEndOfLine */
  if (!continuous_wrap_)
    return buffer()->line_end(pos);

  if (pos == buffer()->length())
    return pos;
  wrapped_line_counter(buffer(), pos, buffer()->length(), 1,
		       startPosIsLineStart, 0, &retPos, &retLines, &retLineStart,
		       &retLineEnd);
  return retLineEnd;
}

/*
 * Same as BufStartOfLine, but returns the character after last wrap point
 * rather than the last newline.
 */
int TextDisplay::line_start(int pos) {
  int retLines, retPos, retLineStart, retLineEnd;

  /* If we're not wrapping, use the more efficient BufStartOfLine */
  if (!continuous_wrap_)
    return buffer()->line_start(pos);

  wrapped_line_counter(buffer(), buffer()->line_start(pos), pos, INT_MAX, true, 0,
		       &retPos, &retLines, &retLineStart, &retLineEnd);
  return retLineStart;
}

/*
 * Same as BufCountBackwardNLines, but takes in to account line breaks when
 * wrapping is turned on.
 */
int TextDisplay::rewind_lines(int startPos, int nLines) {
  TextBuffer *buf = buffer();
  int pos, lineStart, retLines, retPos, retLineStart, retLineEnd;

  /* If we're not wrapping, use the more efficient BufCountBackwardNLines */
  if (!continuous_wrap_)
    return buf->rewind_lines(startPos, nLines);

  pos = startPos;
  while (true) {
    lineStart = buf->line_start(pos);
    wrapped_line_counter(buf, lineStart, pos, INT_MAX,
			 true, 0, &retPos, &retLines, &retLineStart, &retLineEnd);
    if (retLines > nLines)
      return skip_lines(lineStart, retLines-nLines, true);
    nLines -= retLines;
    pos = lineStart - 1;
    if (pos < 0)
      return 0;
    nLines -= 1;
  }
}

static inline int isseparator(int c) {
  return c != '$' && c != '_' && (isspace(c) || ispunct(c));
}

void TextDisplay::next_word() {
  int pos = find_next_char(insert_position());
  char ch = buffer()->character(pos);
  while (pos < buffer()->length() && !isseparator(ch)) {
    pos = find_next_char(pos+1);
    ch = buffer()->character(pos);
    if (ch == '\n') {
      // Stop at end of the line 
      insert_position(pos);
      return;
    }
  }
  while (pos < buffer()->length() && isseparator(ch)) {
    pos = find_next_char(pos+1);
    ch = buffer()->character(pos);
  }
  insert_position(pos);
}

void TextDisplay::previous_word() {
  int pos = find_prev_char(insert_position()-1);
  if (pos==0) return;
  char ch = buffer()->character(pos);
  while (pos && isseparator(ch)) {
    pos = find_prev_char(pos-1);
    ch = buffer()->character(pos);
    if (ch=='\n') break;
  }
  while (pos && !isseparator(ch)) {
    pos = find_prev_char(pos-1);
    ch = buffer()->character(pos);
  }
  if (isseparator(ch) || ch=='\n') pos = find_next_char(pos+1);

  insert_position(pos);
}

/*
 * Callback attached to the text buffer to receive delete information before
 * the modifications are actually made.
 */
void TextDisplay::buffer_predelete_cb(int pos, int nDeleted, void *cbArg) {
  TextDisplay *textD = (TextDisplay *)cbArg;
  if (textD->continuous_wrap_ &&
      (textD->fixed_fontwidth_ == -1))
    /* Note: we must perform this measurement, even if there is not a
       single character deleted; the number of "deleted" lines is the
       number of visual lines spanned by the real line in which the
       modification takes place.
       Also, a modification of the tab distance requires the same
       kind of calculations in advance, even if the font width is "fixed",
       because when the width of the tab characters changes, the layout
       of the text may be completely different. */
    textD->measure_deleted_lines(pos, nDeleted);
  else
    textD->suppressresync_ = false; /* Probably not needed, but just in case */
}

/*
 * Callback attached to the text buffer to receive modification information
 */
void TextDisplay::buffer_modified_cb( int pos, int nInserted, int nDeleted,
				      int nRestyled, const char *deletedText, void *cbArg) {
  int linesInserted, linesDeleted, startDispPos, endDispPos;
  TextDisplay *textD = (TextDisplay *) cbArg;
  TextBuffer *buf = textD->buffer_;
  int oldFirstChar = textD->firstchar_;
  int origCursorPos = textD->cursor_pos_;
  int wrapModStart, wrapModEnd;
  bool scrolled;

  /* buffer modification cancels vertical cursor motion column */
  if (nInserted != 0 || nDeleted != 0)
    textD->cursor_preferred_col_ = -1;

  /* Count the number of lines inserted and deleted, and in the case
     of continuous wrap mode, how much has changed */
  if (textD->continuous_wrap_) {
    textD->find_wrap_range(deletedText, pos, nInserted, nDeleted,
			   &wrapModStart, &wrapModEnd, &linesInserted, &linesDeleted);
  } else {
    linesInserted = nInserted == 0 ? 0 :
                    buf->count_lines(pos, pos + nInserted);
    linesDeleted = nDeleted == 0 ? 0 : countlines(deletedText);
  }

  /* Update the line starts and topline_num_ */
  if (nInserted != 0 || nDeleted != 0) {
    if (textD->continuous_wrap_) {
      textD->update_line_starts( wrapModStart, wrapModEnd-wrapModStart,
				 nDeleted + pos-wrapModStart + (wrapModEnd-(pos+nInserted)),
				 linesInserted, linesDeleted, &scrolled);
    } else {
      textD->update_line_starts( pos, nInserted, nDeleted, linesInserted,
				 linesDeleted, &scrolled);
    }
  } else
    scrolled = false;

  /* If we're counting non-wrapped lines as well, maintain the absolute
     (non-wrapped) line number of the text displayed */
  if (textD->maintaining_absolute_top_line_number() && (nInserted != 0 || nDeleted != 0)) {
    if (pos + nDeleted < oldFirstChar)
      textD->abs_topline_num_ += buf->count_lines(pos, pos + nInserted) -
	                       countlines(deletedText);
    else if (pos < oldFirstChar)
      textD->reset_absolute_top_line_number();
  }

  /* Update the line count for the whole buffer */
  textD->bufferlines_cnt_ += linesInserted - linesDeleted;

  /* Update the cursor position */
  if (textD->cursor_hint_ != NO_HINT) {
    textD->cursor_pos_ = textD->cursor_hint_;
    textD->cursor_hint_ = NO_HINT;
  } else if (textD->cursor_pos_ > pos) {
    if (textD->cursor_pos_ < pos + nDeleted)
      textD->cursor_pos_ = pos;
    else
      textD->cursor_pos_ += nInserted - nDeleted;
  }

  // refigure scrollbars & stuff
  textD->relayout();

  // don't need to do anything else if not visible?
  if (!textD->visible_r()) return;

  /* If the changes caused scrolling, re-paint everything and we're done. */
  if (scrolled) {
    textD->redraw();
    if (textD->stylebuffer_) {   /* See comments in extendRangeForStyleMods */
      textD->stylebuffer_->primary_selection()->selected(false);
      textD->stylebuffer_->primary_selection()->zerowidth(false);
    }
    return;
  }

  /* If the changes didn't cause scrolling, decide the range of characters
     that need to be re-painted.  Also if the cursor position moved, be
     sure that the redisplay range covers the old cursor position so the
     old cursor gets erased, and erase the bits of the cursor which extend
     beyond the left and right edges of the text. */
  startDispPos = textD->continuous_wrap_ ? wrapModStart : pos;
  if (origCursorPos == startDispPos && textD->cursor_pos_ != startDispPos)
    startDispPos = min( startDispPos, origCursorPos - 1);
  if (linesInserted == linesDeleted) {
    if (nInserted == 0 && nDeleted == 0)
      endDispPos = pos + nRestyled;
    else {
      endDispPos = textD->continuous_wrap_ ? wrapModEnd :
	            buf->line_end( pos + nInserted) + 1;
    }
    /* If more than one line is inserted/deleted, a line break may have
           been inserted or removed in between, and the line numbers may
           have changed. If only one line is altered, line numbers cannot
           be affected (the insertion or removal of a line break always 
           results in at least two lines being redrawn). */
    //if (linesInserted > 1) textD->draw_line_numbers(false);
  } else {
    endDispPos = textD->lastchar_ + 1;
    //textD->draw_line_numbers(false);
  }

  /* If there is a style buffer, check if the modification caused additional
     changes that need to be redisplayed.  (Redisplaying separately would
     cause double-redraw on almost every modification involving styled
     text).  Extend the redraw range to incorporate style changes */
  if (textD->stylebuffer_)
    textD->extend_range_for_styles( &startDispPos, &endDispPos);

  /* Redisplay computed range */
  textD->redisplay_range(startDispPos, endDispPos);
}

/*
 * In continuous wrap mode, internal line numbers are calculated after
 * wrapping.  A separate non-wrapped line count is maintained when line
 * numbering is turned on.  There is some performance cost to maintaining this
 * line count, so normally absolute line numbers are not tracked if line
 * numbering is off.  This routine allows callers to specify that they still
 * want this line count maintained (for use via TextDPosToLineAndCol).
 * More specifically, this allows the line number reported in the statistics
 * line to be calibrated in absolute lines, rather than post-wrapped lines.
 */
void TextDisplay::maintain_absolute_top_line_number(bool state) {
  need_abs_topline_num_ = state;
  reset_absolute_top_line_number();
}

/*
 * Returns the absolute (non-wrapped) line number of the first line displayed.
 * Returns 0 if the absolute top line number is not being maintained.
 */
int TextDisplay::get_absolute_top_line_number() {
  if (!continuous_wrap_)
    return topline_num_;
  if (maintaining_absolute_top_line_number())
    return abs_topline_num_;
  return 0;
}

/*
 * Re-calculate absolute top line number for a change in scroll position.
 */
void TextDisplay::absolute_top_line_number(int oldFirstChar) {
  if (maintaining_absolute_top_line_number()) {
    if (firstchar_ < oldFirstChar)
      abs_topline_num_ -= buffer()->count_lines(firstchar_, oldFirstChar);
    else
      abs_topline_num_ += buffer()->count_lines(oldFirstChar, firstchar_);
  }
}

/*
 * Return true if a separate absolute top line number is being maintained
 * (for displaying line numbers or showing in the statistics line).
 */
int TextDisplay::maintaining_absolute_top_line_number() {
  return continuous_wrap_ &&
         (linenumwidth_ != 0 || need_abs_topline_num_);
}

/*
 * Count lines from the beginning of the buffer to reestablish the
 * absolute (non-wrapped) top line number.  If mode is not continuous wrap,
 * or the number is not being maintained, does nothing.
 */
void TextDisplay::reset_absolute_top_line_number() {
  abs_topline_num_ = 1;
  absolute_top_line_number(0);
}

/*
 * Find the line number of position "pos" relative to the first line of
 * displayed text. Returns 0 if the line is not displayed.
 */
bool TextDisplay::position_to_line(int pos, int *lineNum) {
  int i;

  *lineNum = 0;
  if (pos < firstchar_) return false;
  if (pos > lastchar_) {
    if (empty_vlines()) {
      if (lastchar_ < buffer_->length()) {
	if (!position_to_line(lastchar_, lineNum)) {
	  fltk::warning("TextDisplay::position_to_line(): Consistency check ptvl failed");
	  return false;
	}
	return ++(*lineNum) <= visiblelines_cnt_ - 1;
      } else {
	position_to_line(max(lastchar_ - 1, 0), lineNum);
	return true;
      }
    }
    return false;
  }

  for (i = visiblelines_cnt_ - 1; i >= 0; i--) {
    if (linestarts_[i] != -1 && pos >= linestarts_[i]) {
      *lineNum = i;
      return true;
    }
  }
  return false; /* probably never be reached */
}

/*
 * Draw the text on a single line represented by "visLineNum" (the
 * number of lines down from the top of the display), limited by
 * "leftClip" and "rightClip" window coordinates and "leftCharIndex" and
 * "rightCharIndex" character positions (not including the character at
 * position "rightCharIndex").
 */
void TextDisplay::draw_vline(int visLineNum, int leftClip, int rightClip,
			     int leftCharIndex, int rightCharIndex) {
  TextBuffer *buf = buffer_;
  int i, X, Y, startX, charIndex, lineStartPos, lineLen, fontHeight;
  int stdCharWidth, charWidth, startIndex, charStyle, style;
  int charLen, outStartIndex, outIndex;
  int cursorX = 0, hasCursor = 0;
  int dispIndexOffset, cursorPos = cursor_pos_;
  char expandedChar[TEXT_MAX_EXP_CHAR_LEN], outStr[MAX_DISP_LINE_LEN];
  char *outPtr;
  char *lineStr;

  //  printf("draw_vline(visLineNum=%d, leftClip=%d, rightClip=%d, leftCharIndex=%d, rightCharIndex=%d)\n",
  //         visLineNum, leftClip, rightClip, leftCharIndex, rightCharIndex);
  //  printf("nNVisibleLines=%d\n", visiblelines_cnt_);

  /* If line is not displayed, skip it */
  if (visLineNum < 0 || visLineNum >= visiblelines_cnt_)
    return;

  /* Shrink the clipping range to the active display area */
  leftClip = max(text_area.x(), leftClip);
  rightClip = min(rightClip, text_area.r());

  if (leftClip > rightClip) {
    return;
  }

  /* Calculate Y coordinate of the string to draw */
  fontHeight = maxsize_;
  Y = text_area.y() + visLineNum * fontHeight;

  /* Get the text, length, and  buffer position of the line to display */
  lineStartPos = linestarts_[visLineNum];
  if (lineStartPos == -1) {
    lineLen = 0;
    lineStr = NULL;
  } else {
    lineLen = vline_length(visLineNum);
    lineStr = buf->text_range(lineStartPos, lineStartPos + lineLen);
  }

  /* Space beyond the end of the line is still counted in units of characters
     of a standardized character width (this is done mostly because style
     changes based on character position can still occur in this region due
     to rectangular TextSelections). stdCharWidth must be non-zero to
     prevent a potential infinite loop if X does not advance */
  stdCharWidth = 20; // Bigger value for less loops 

  /* Rectangular TextSelections are based on "real" line starts (after
     a newline or start of buffer).  Calculate the difference between the
     last newline position and the line start we're using.  Since scanning
     back to find a newline is expensive, only do so if there's actually a
     rectangular TextSelection which needs it */
  if (continuous_wrap_ && (range_touches_selection(buf->primary_selection(), lineStartPos, lineStartPos + lineLen) ||
			  range_touches_selection(buf->secondary_selection(), lineStartPos, lineStartPos + lineLen) ||
			  range_touches_selection(buf->highlight_selection(), lineStartPos, lineStartPos + lineLen))) {
    dispIndexOffset = buf->count_displayed_characters(buf->line_start(lineStartPos), lineStartPos);
  } else
    dispIndexOffset = 0;

  /* Step through character positions from the beginning of the line (even if
     that's off the left edge of the displayed area) to find the first
     character position that's not clipped, and the X coordinate for drawing
     that character */
  X = text_area.x() - horiz_offset_;
  outIndex = 0;
  for (charIndex = 0; ; charIndex++) {
    charLen = charIndex >= lineLen ? 1 :
            TextBuffer::expand_character(lineStr[charIndex], outIndex,
    	                                 expandedChar, buf->tab_distance(), buf->null_substitution_character());
      
    // UTF-8
    bool utf8 = false;
    if (charIndex < lineLen && (lineStr[charIndex] & 0x80)) {
      charLen = utf8seqlen(lineStr[charIndex]);
      memcpy(expandedChar, &lineStr[charIndex], charLen);
      utf8 = true;
    }

    style = position_style(lineStartPos, lineLen, charIndex,
                           outIndex + dispIndexOffset);
    charWidth = charIndex >= lineLen ? stdCharWidth :
    	        string_width(expandedChar, charLen, style);
    if (X + charWidth >= leftClip && charIndex >= leftCharIndex) {
    	startIndex = charIndex;
    	outStartIndex = outIndex;
    	startX = X;
    	break;
    }
    X += charWidth;
    // UTF-8
    if (utf8 && charLen > 1) charIndex += (charLen-1);
    outIndex += charLen;
  }

  /* Scan character positions from the beginning of the clipping range, and
     draw parts whenever the style changes (also note if the cursor is on
     this line, and where it should be drawn to take advantage of the x
     position which we've gone to so much trouble to calculate) */
  /* since characters between style may overlap, we draw the full
     background first */
  outPtr = outStr;
  outIndex = outStartIndex;
  X = startX;
  for (charIndex = startIndex; charIndex < rightCharIndex; charIndex++) {
    if (lineStartPos+charIndex == cursorPos) {
      if (charIndex < lineLen || (charIndex == lineLen && cursorPos >= buf->length())) {
        hasCursor = 1;
        cursorX = X - 1;
      } else if (charIndex == lineLen) {
        if (wrap_uses_character(cursorPos)) {
          hasCursor = 1;
    	  cursorX = X - 1;
        }
      }
    }
    charLen = charIndex >= lineLen ? 1 :
              TextBuffer::expand_character(lineStr[charIndex], outIndex,
    	                                   expandedChar, buf->tab_distance(), buf->null_substitution_character());

    // UTF-8
    bool utf8 = false;
    if (charIndex < lineLen && (lineStr[charIndex] & 0x80)) {
      charLen = utf8seqlen(lineStr[charIndex]);
      memcpy(expandedChar, &lineStr[charIndex], charLen);
      utf8 = true;
    }

    charStyle = position_style(lineStartPos, lineLen, charIndex, outIndex + dispIndexOffset);
    for (i=0; i<charLen; i++) {
      if (i != 0 && charIndex < lineLen && lineStr[charIndex] == '\t')
        charStyle = position_style(lineStartPos, lineLen, charIndex, outIndex + dispIndexOffset);
      if (charStyle != style) {
    	draw_string(style, startX, Y, X, outStr, outPtr - outStr);
        outPtr = outStr;
    	startX = X;
        style = charStyle;
      }
      if (charIndex < lineLen) {
        *outPtr = expandedChar[i];
        int strl = 1;
        // UTF-8
        if (*outPtr & 0x80)
          strl = utf8seqlen(*outPtr);
        if (strl > 0)
          charWidth = string_width(&expandedChar[i], strl, charStyle);
        else 
          charWidth = 0;
      } else
        charWidth = stdCharWidth;
      outPtr++;
      X += charWidth;
      outIndex++;
    }
    // UTF-8
    if (utf8 && charLen > 1) charIndex += (charLen-1);
    if (outPtr-outStr+TEXT_MAX_EXP_CHAR_LEN>=MAX_DISP_LINE_LEN || X>=rightClip)
      break;
  }

  /* Draw the remaining style segment */
  draw_string(style, startX, Y, X, outStr, outPtr - outStr);

  /* Draw the cursor if part of it appeared on the redisplayed part of
     this line.  Also check for the cases which are not caught as the
     line is scanned above: when the cursor appears at the very end
     of the redisplayed section. */
  if (cursor_on_) {
    if (hasCursor)
      draw_cursor(cursorX, Y);
    else if (charIndex<lineLen && (lineStartPos+charIndex+1 == cursorPos)
	     && X == rightClip) {
      if (cursorPos >= buf->length())
    	draw_cursor(X - 1, Y);
      else {
        if (wrap_uses_character(cursorPos))
    	  draw_cursor(X - 1, Y);
      }
    } 
  }
  if (lineStr != NULL)
    free(lineStr);
}

/*
 * Draw a string or blank area according to parameter "style", using the
 * appropriate colors and drawing method for that style, with top left
 * corner at X, y.  If style says to draw text, use "string" as source of
 * characters, and draw "nChars", if style is FILL, erase
 * rectangle where text would have drawn from X to toX and from Y to
 * the maximum Y extent of the current font(s).
 */
void TextDisplay::draw_string( int style, int X, int Y, int toX,
			       const char *string, int nChars) {
  const StyleTableEntry *styleRec = NULL;
  unsigned attr = 0; 

  /* Draw blank area rather than text, if that was the request */
  if (style & FILL_MASK) {
    clear_rect(style, X, Y, toX - X, maxsize_);
    return;
  }

  /* Set font, color, and gc depending on style.  For normal text, GCs
     for normal drawing, or drawing within a TextSelection or highlight are
     pre-allocated and pre-configured.  For syntax highlighting, GCs are
     configured here, on the fly. */

  Color background;
  if (style & (HIGHLIGHT_MASK | PRIMARY_MASK)) {
    if (focused()) {
      background = selection_color();
    } else {
      background = lerp(color(), selection_color(), 0.5f);
    }
  } else {
    background = color();
  }

  Font *font = textfont();
  float fsize = textsize();
  Color foreground;

  if (style & STYLE_LOOKUP_MASK) {
    int si = (style & STYLE_LOOKUP_MASK) - 'A';
    if (si < 0) {
      si = 0;
    } else if (si >= numstyles_) {
      si = numstyles_ - 1;
    }
    styleRec = styletable_ + si;
    attr  = styleRec->attr;
    font  = styleRec->font;
    fsize = styleRec->size;
    foreground = contrast(styleRec->color, background);
  } else if (style & (HIGHLIGHT_MASK | PRIMARY_MASK)) {
    foreground = contrast(textcolor(), background);
  } else {
    foreground = textcolor();
  }

  fltk::setcolor(background);
  fltk::fillrect(X, Y, toX - X, maxsize_);
  
  if (!(style & BG_ONLY_MASK) && !(attr & ATTR_HIDDEN)) {
    fltk::setcolor(foreground);
    fltk::setfont(font, fsize);
    fltk::drawtext(string, nChars, (float)X, (float)(Y + maxsize_ - fltk::getdescent()));
  }

  /* Underline if style is secondary TextSelection */
  if (style & SECONDARY_MASK || (attr & ATTR_UNDERLINE)) {
    fltk::setcolor(foreground);
    fltk::drawline((int)X, (int)(Y + ascent_), (int)(toX - 1), (int)(Y + fltk::getascent()));
  }
}

/*
 * Clear a rectangle with the appropriate background color for "style"
 */
void TextDisplay::clear_rect( int style, int X, int Y,
			      int width, int height) {
  /* A width of zero means "clear to end of window" to XClearArea */
  if (width == 0)
    return;

  if (X < text_area.x()) X = text_area.x();
  if (Y < text_area.y()) Y = text_area.y();
  if (width > text_area.w()) width = text_area.w();
  if (height > text_area.w()) height = text_area.h();

  if (!focused()) {
    if (style & (HIGHLIGHT_MASK | PRIMARY_MASK)) {
      fltk::setcolor(lerp(color(), selection_color(), 0.5f));
    } else {
      fltk::setcolor(color());
    }
    //setcolor(MAGENTA);
  } else if (style & HIGHLIGHT_MASK) {
    fltk::setcolor(contrast(textcolor(), color()));
  } else if (style & PRIMARY_MASK) {
    fltk::setcolor(selection_color());
  } else {
    fltk::setcolor(color());
  }
  fltk::fillrect(X, Y, width, height);
}

/*
 * Draw a cursor with top center at X, Y.
 */
void TextDisplay::draw_cursor(int X, int Y) {
  typedef struct {
    int x1, y1, x2, y2;
  } Segment;

  int fontHeight = maxsize_;
  int bot = Y + fontHeight - 1;

  if (X < text_area.x() - 1 || X > text_area.x() + text_area.w())
    return;
  if (Y < text_area.y() - 1 || bot > text_area.y() + text_area.h())
    return;

  fltk::setcolor(cursor_color_);

  fltk::push_no_clip();
#if 1
  fltk::fillrect(X, Y, 2, bot-Y);
#else

  /* For cursors other than the block, make them around 2/3 of a character
     width, rounded to an even number of pixels so that X will draw an
     odd number centered on the stem at x. */
  Segment segs[5];
  int nSegs = 0;
  int fontWidth = stdfontwidth_;
  int cursorWidth = (fontWidth/3) * 2;
  int left = X - cursorWidth / 2;
  int right = left + cursorWidth;
  int midY;

  /* Create segments and draw cursor */
  if (cursor_style_ == CARET_CURSOR) {
    midY = bot - fontHeight / 5;
    segs[ 0 ].x1 = left; segs[ 0 ].y1 = bot; segs[ 0 ].x2 = X; segs[ 0 ].y2 = midY;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = midY; segs[ 1 ].x2 = right; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = left; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = X; segs[ 2 ].y2 = midY - 1;
    segs[ 3 ].x1 = X; segs[ 3 ].y1 = midY - 1; segs[ 3 ].x2 = right; segs[ 3 ].y2 = bot;
    nSegs = 4;
  } else if (cursor_style_ == NORMAL_CURSOR) {
    segs[ 0 ].x1 = left; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = right; segs[ 0 ].y2 = Y;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = Y; segs[ 1 ].x2 = X; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = left; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = right; segs[ 2 ].y2 = bot;
    nSegs = 3;
  } else if (cursor_style_ == HEAVY_CURSOR) {
    segs[ 0 ].x1 = X - 1; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = X - 1; segs[ 0 ].y2 = bot;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = Y; segs[ 1 ].x2 = X; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = X + 1; segs[ 2 ].y1 = Y; segs[ 2 ].x2 = X + 1; segs[ 2 ].y2 = bot;
    segs[ 3 ].x1 = left; segs[ 3 ].y1 = Y; segs[ 3 ].x2 = right; segs[ 3 ].y2 = Y;
    segs[ 4 ].x1 = left; segs[ 4 ].y1 = bot; segs[ 4 ].x2 = right; segs[ 4 ].y2 = bot;
    nSegs = 5;
  } else if (cursor_style_ == DIM_CURSOR) {
    midY = Y + fontHeight / 2;
    segs[ 0 ].x1 = X; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = X; segs[ 0 ].y2 = Y;
    segs[ 1 ].x1 = X; segs[ 1 ].y1 = midY; segs[ 1 ].x2 = X; segs[ 1 ].y2 = midY;
    segs[ 2 ].x1 = X; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = X; segs[ 2 ].y2 = bot;
    nSegs = 3;
  } else if (cursor_style_ == BLOCK_CURSOR) {
    right = X + fontWidth;
    segs[ 0 ].x1 = X; segs[ 0 ].y1 = Y; segs[ 0 ].x2 = right; segs[ 0 ].y2 = Y;
    segs[ 1 ].x1 = right; segs[ 1 ].y1 = Y; segs[ 1 ].x2 = right; segs[ 1 ].y2 = bot;
    segs[ 2 ].x1 = right; segs[ 2 ].y1 = bot; segs[ 2 ].x2 = X; segs[ 2 ].y2 = bot;
    segs[ 3 ].x1 = X; segs[ 3 ].y1 = bot; segs[ 3 ].x2 = X; segs[ 3 ].y2 = Y;
    nSegs = 4;
  }

  for (int k = 0; k < nSegs; k++) {
    drawline(segs[k].x1, segs[k].y1, segs[k].x2, segs[k].y2);
  }
#endif

  fltk::pop_clip();

  /* Save the last position drawn */
  cursor_oldx_ = X;
  cursor_oldy_ = Y;
  transform(X, Y);
  fl_set_spot(textfont(), this, X, Y);
}

/*
 * Determine the drawing method to use to draw a specific character from "buf".
 * "lineStartPos" gives the character index where the line begins, "lineIndex",
 * the number of characters past the beginning of the line, and "dispIndex",
 * the number of displayed characters past the beginning of the line.  Passing
 * lineStartPos of -1 returns the drawing style for "no text".
 *
 * Why not just: position_style(pos)?  Because style applies to blank areas
 * of the window beyond the text boundaries, and because this routine must also
 * decide whether a position is inside of a rectangular TextSelection, and do
 * so efficiently, without re-counting character positions from the start of the
 * line.
 *
 * Note that style is a somewhat incorrect name, drawing method would
 * be more appropriate.
 */
int TextDisplay::position_style( int lineStartPos,
				 int lineLen, int lineIndex, int dispIndex) {
  TextBuffer *buf = buffer_;
  TextBuffer *styleBuf = stylebuffer_;
  int pos, style = 0;

  if (lineStartPos == -1 || buf == NULL)
    return FILL_MASK;

  pos = lineStartPos + min(lineIndex, lineLen);

  if (lineIndex >= lineLen)
    style = FILL_MASK;
  else if (styleBuf != NULL) {
    style = (unsigned char)styleBuf->character( pos);

    if (style == unfinished_style_ && unfinished_highlight_cb_) {
      /* encountered "unfinished" style, trigger parsing */
      (unfinished_highlight_cb_)(pos, highlight_cbarg_);
      style = (unsigned char) styleBuf->character( pos);
    }
  }
  if (buf->primary_selection()->includes(pos, lineStartPos, dispIndex))
    style |= PRIMARY_MASK;
  if (buf->highlight_selection()->includes(pos, lineStartPos, dispIndex))
    style |= HIGHLIGHT_MASK;
  if (buf->secondary_selection()->includes(pos, lineStartPos, dispIndex))
    style |= SECONDARY_MASK;

  return style;
}

/*
 * Find the width of a string in the font of a particular style
 */
int TextDisplay::string_width(const char *string, int length, int style) {
  Font *font;
  float fsize;

  if (style & STYLE_LOOKUP_MASK) {
    int si = (style & STYLE_LOOKUP_MASK) - 'A';
    if (si < 0) {
      si = 0;
    } else if (si >= numstyles_) {
      si = numstyles_ - 1;
    }
    font  = styletable_[si].font;
    fsize = styletable_[si].size;
  } else {
    font  = textfont();
    fsize = textsize();
  }
  fltk::setfont(font, fsize);
  return (int)fltk::getwidth(string, length);
}

/**
 * Translate window coordinates to the nearest (insert cursor or character
 * cell) text position.  The parameter posType specifies how to interpret the
 * position: CURSOR_POS means translate the coordinates to the nearest cursor
 * position, and CHARACTER_POS means return the position of the character
 * closest to (X, Y).
 */
int TextDisplay::xy_to_position(int X, int Y, int posType) {
  int charIndex, lineStart, lineLen, fontHeight;
  int charWidth, charLen, charStyle, visLineNum, xStep, outIndex;
  char expandedChar[TEXT_MAX_EXP_CHAR_LEN];
  char *lineStr;

  /* Find the visible line number corresponding to the Y coordinate */
  fontHeight = maxsize_;
  visLineNum = (Y - text_area.y()) / fontHeight;
  if (visLineNum < 0)
    return firstchar_;
  if (visLineNum >= visiblelines_cnt_)
    visLineNum = visiblelines_cnt_ - 1;

  /* Find the position at the start of the line */
  lineStart = linestarts_[visLineNum];

  /* If the line start was empty, return the last position in the buffer */
  if (lineStart == -1)
    return buffer_->length();

  /* Get the line text and its length */
  lineLen = vline_length(visLineNum);
  lineStr = buffer_->text_range(lineStart, lineStart + lineLen);

  /* Step through character positions from the beginning of the line
     to find the character position corresponding to the X coordinate */
  xStep = text_area.x() - horiz_offset_;
  outIndex = 0;
  for (charIndex = 0; charIndex < lineLen; charIndex++) {
    charLen = TextBuffer::expand_character( lineStr[charIndex], outIndex, expandedChar,
					    buffer_->tab_distance(), buffer_->null_substitution_character());

    // UTF-8
    bool utf8 = false;
    if (lineStr[charIndex] & 0x80) {
      charLen = utf8seqlen(lineStr[charIndex]);
      memcpy(expandedChar, &lineStr[charIndex], charLen);
      utf8 = true;
    }

    charStyle = position_style(lineStart, lineLen, charIndex, outIndex);
    charWidth = string_width(expandedChar, charLen, charStyle);
    if (X < xStep + (posType == CURSOR_POS ? charWidth / 2 : charWidth)) {
      free(lineStr);
      return lineStart + charIndex;
    }
    xStep += charWidth;
    // UTF-8
    if (utf8 && charLen > 1) charIndex += (charLen-1);
    outIndex += charLen;
  }

  /* If the X position was beyond the end of the line, return the position
     of the newline at the end of the line */
  free(lineStr);
  return lineStart + lineLen;
}

/**
 * Translate window coordinates to the nearest row and column number for
 * positioning the cursor.  This, of course, makes no sense when the font is
 * proportional, since there are no absolute columns.  The parameter posType
 * specifies how to interpret the position: CURSOR_POS means translate the
 * coordinates to the nearest position between characters, and CHARACTER_POS
 * means translate the position to the nearest character cell.
 */
void TextDisplay::xy_to_rowcol(int X, int Y, int *row,
			       int *column, int posType) {
  int fontHeight = maxsize_;
  int fontWidth = stdfontwidth_;

  /* Find the visible line number corresponding to the Y coordinate */
  *row = (Y - text_area.y()) / fontHeight;
  if (*row < 0) *row = 0;
  if (*row >= visiblelines_cnt_) *row = visiblelines_cnt_ - 1;
  *column = ((X - text_area.x()) + horiz_offset_ +
	      (posType == CURSOR_POS ? fontWidth / 2 : 0)) / fontWidth;
  if (*column < 0) *column = 0;
}

/*
 * Offset the line starts array, topline_num_, firstchar_ and lastChar, for a new
 * vertical scroll position given by newTopLineNum.  If any currently displayed
 * lines will still be visible, salvage the line starts values, otherwise,
 * count lines from the nearest known line start (start or end of buffer, or
 * the closest value in the linestarts_ array)
 */
void TextDisplay::offset_line_starts(int newTopLineNum) {
  int oldTopLineNum = topline_num_;
  int oldFirstChar = firstchar_;
  int lineDelta = newTopLineNum - oldTopLineNum;
  int nVisLines = visiblelines_cnt_;
  int *lineStarts = linestarts_;
  int i, lastLineNum;
  TextBuffer *buf = buffer_;

  /* If there was no offset, nothing needs to be changed */
  if (lineDelta == 0)
    return;

  /* Find the new value for firstchar_ by counting lines from the nearest
     known line start (start or end of buffer, or the closest value in the
     lineStarts array) */
  lastLineNum = oldTopLineNum + nVisLines - 1;
  if (newTopLineNum < oldTopLineNum && newTopLineNum < -lineDelta) {
    firstchar_ = skip_lines(0, newTopLineNum - 1, true);
  } else if (newTopLineNum < oldTopLineNum) {
    firstchar_ = rewind_lines(firstchar_, -lineDelta);
  } else if (newTopLineNum < lastLineNum) {
    firstchar_ = lineStarts[newTopLineNum - oldTopLineNum];
  } else if (newTopLineNum - lastLineNum < bufferlines_cnt_ - newTopLineNum) {
    firstchar_ = skip_lines(lineStarts[ nVisLines - 1 ], newTopLineNum - lastLineNum, true);
  } else {
    firstchar_ = rewind_lines(buf->length(), bufferlines_cnt_ - newTopLineNum + 1);
  }

  /* Fill in the line starts array */
  if (lineDelta < 0 && -lineDelta < nVisLines) {
    for (i = nVisLines - 1; i >= -lineDelta; i--)
      lineStarts[i] = lineStarts[i + lineDelta];
    calc_line_starts(0, -lineDelta);
  } else if (lineDelta > 0 && lineDelta < nVisLines) {
    for (i = 0; i < nVisLines - lineDelta; i++)
      lineStarts[i] = lineStarts[ i + lineDelta ];
    calc_line_starts(nVisLines - lineDelta, nVisLines - 1);
  } else
    calc_line_starts(0, nVisLines);

  /* Set lastChar and topline_num_ */
  calc_last_char();
  topline_num_ = newTopLineNum;

  /* If we're numbering lines or being asked to maintain an absolute line
     number, re-calculate the absolute line number */
  absolute_top_line_number(oldFirstChar);
}

/*
 * Update the line starts array, topline_num_, firstchar_ and lastChar for text
 * display "textD" after a modification to the text buffer, given by the
 * position where the change began "pos", and the nmubers of characters
 * and lines inserted and deleted.
 */
void TextDisplay::update_line_starts(int pos, int charsInserted,
				     int charsDeleted, int linesInserted, int linesDeleted, bool *scrolled) {
  int *lineStarts = linestarts_;
  int i, lineOfPos, lineOfEnd, nVisLines = visiblelines_cnt_;
  int charDelta = charsInserted - charsDeleted;
  int lineDelta = linesInserted - linesDeleted;

  /* If all of the changes were before the displayed text, the display
     doesn't change, just update the top line num and offset the line
     start entries and first and last characters */
  if (pos + charsDeleted < firstchar_) {
    topline_num_ += lineDelta;
    for (i = 0; i < nVisLines && lineStarts[i] != -1; i++)
      lineStarts[i] += charDelta;
    firstchar_ += charDelta;
    lastchar_ += charDelta;
    *scrolled = false;
    return;
  }

  /* The change began before the beginning of the displayed text, but
     part or all of the displayed text was deleted */
  if (pos < firstchar_) {
    /* If some text remains in the window, anchor on that  */
    if (position_to_line(pos + charsDeleted, &lineOfEnd) &&
	++lineOfEnd < nVisLines && lineStarts[lineOfEnd] != -1) {
      topline_num_ = max(1, topline_num_ + lineDelta);
      firstchar_ = rewind_lines(lineStarts[lineOfEnd] + charDelta, lineOfEnd);
      /* Otherwise anchor on original line number and recount everything */
    } else {
      if (topline_num_ > bufferlines_cnt_ + lineDelta) {
	topline_num_ = 1;
	firstchar_ = 0;
      } else
	firstchar_ = skip_lines(0, topline_num_ - 1, true);
    }
    calc_line_starts(0, nVisLines - 1);
    /* calculate lastChar by finding the end of the last displayed line */
    calc_last_char();
    *scrolled = true;
    return;
  }

  /* If the change was in the middle of the displayed text (it usually is),
     salvage as much of the line starts array as possible by moving and
     offsetting the entries after the changed area, and re-counting the
     added lines or the lines beyond the salvaged part of the line starts
     array */
  if (pos <= lastchar_) {
    /* find line on which the change began */
    position_to_line(pos, &lineOfPos);
    /* salvage line starts after the changed area */
    if (lineDelta == 0) {
      for (i = lineOfPos + 1; i < nVisLines && lineStarts[i] != -1; i++)
	lineStarts[i] += charDelta;
    } else if (lineDelta > 0) {
      for (i = nVisLines - 1; i >= lineOfPos + lineDelta + 1; i--)
	lineStarts[i] = lineStarts[i - lineDelta] +
	  (lineStarts[i - lineDelta] == -1 ? 0 : charDelta);
    } else /* (lineDelta < 0) */ {
      for (i = max(0, lineOfPos + 1); i < nVisLines + lineDelta; i++)
	lineStarts[i] = lineStarts[i - lineDelta] +
	  (lineStarts[i - lineDelta] == -1 ? 0 : charDelta);
    }
    /* fill in the missing line starts */
    if (linesInserted >= 0)
      calc_line_starts(lineOfPos + 1, lineOfPos + linesInserted);
    if (lineDelta < 0)
      calc_line_starts(nVisLines + lineDelta, nVisLines);
    /* calculate lastChar by finding the end of the last displayed line */
    calc_last_char();
    *scrolled = false;
    return;
  }

  /* Change was past the end of the displayed text, but displayable by virtue
     of being an insert at the end of the buffer into visible blank lines */
  if (empty_vlines()) {
    position_to_line(pos, &lineOfPos);
    calc_line_starts(lineOfPos, lineOfPos + linesInserted);
    calc_last_char();
    *scrolled = false;
    return;
  }

  /* Change was beyond the end of the buffer and not visible, do nothing */
  *scrolled = false;
}

/*
 * Scan through the text in the "textD"'s buffer and recalculate the line
 * starts array values beginning at index "startLine" and continuing through
 * (including) "endLine".  It assumes that the line starts entry preceding
 * "startLine" (or firstchar_ if startLine is 0) is good, and re-counts
 * newlines to fill in the requested entries.  Out of range values for
 * "startLine" and "endLine" are acceptable.
 */
void TextDisplay::calc_line_starts(int startLine, int endLine) {
    int startPos, bufLen = buffer_ ? buffer_->length() : 0;
  int line, lineEnd, nextLineStart, nVis = visiblelines_cnt_;
  int *lineStarts = linestarts_;

  /* Clean up (possibly) messy input parameters */
  if (nVis == 0) return;
  if (endLine < 0) endLine = 0;
  if (endLine >= nVis) endLine = nVis - 1;
  if (startLine < 0) startLine = 0;
  if (startLine >= nVis) startLine = nVis - 1;
  if (startLine > endLine)
    return;

  /* Find the last known good line number -> position mapping */
  if (startLine == 0) {
    lineStarts[0] = firstchar_;
    startLine = 1;
  }
  startPos = lineStarts[startLine - 1];

  /* If the starting position is already past the end of the text,
     fill in -1's (means no text on line) and return */
  if (startPos == -1) {
    for (line = startLine; line <= endLine; line++)
      lineStarts[line] = -1;
    return;
  }

  /* Loop searching for ends of lines and storing the positions of the
     start of the next line in lineStarts */
  for (line = startLine; line <= endLine; line++) {
    find_line_end(startPos, true, &lineEnd, &nextLineStart);
    startPos = nextLineStart;
    if (startPos >= bufLen) {
      /* If the buffer ends with a newline or line break, put
	 buf->length() in the next line start position (instead of
	 a -1 which is the normal marker for an empty line) to
	 indicate that the cursor may safely be displayed there */
      if (line == 0 || (lineStarts[line - 1] != bufLen &&
			lineEnd != nextLineStart)) {
	lineStarts[line] = bufLen;
	line++;
      }
      break;
    }
    lineStarts[line] = startPos;
  }

  /* Set any entries beyond the end of the text to -1 */
  for (; line <= endLine; line++)
    lineStarts[line] = -1;
}

/*
 * Given a TextDisplay with a complete, up-to-date lineStarts array, update
 * the lastChar entry to point to the last buffer position displayed.
 */
void TextDisplay::calc_last_char() {
  int i;
  for (i = visiblelines_cnt_ - 1; i > 0 && linestarts_[i] == -1; i--) ;
  lastchar_ = i < 0 ? 0 : line_end(linestarts_[i], true);
}

void TextDisplay::scroll(int topLineNum, int horizOffset) {
  scroll_(topLineNum, horizOffset);
  update_v_scrollbar();
  update_h_scrollbar();
}

/*
 * Set the scroll position of the text display vertically by line number and
 * horizontally by pixel offset from the left margin
 */
void TextDisplay::scroll_(int topLineNum, int horizOffset) {
  int longestvline = longest_vline();

  /* Limit the requested scroll position to allowable values */
  if (topLineNum > bufferlines_cnt_ + 3 - visiblelines_cnt_)
    topLineNum = bufferlines_cnt_ + 3 - visiblelines_cnt_;
  if (topLineNum < 1) 
    topLineNum = 1;

  if (horizOffset > longestvline - text_area.w())
    horizOffset = longestvline - text_area.w();
  if (horizOffset < 0) 
    horizOffset = 0;

  /* Do nothing if scroll position hasn't actually changed or there's no
     window to draw in yet */
  if (horiz_offset_ == horizOffset && topline_num_ == topLineNum)
    return;

  /* If the vertical scroll position has changed, update the line
     starts array and related counters in the text display */
  offset_line_starts(topLineNum);

  /* Just setting horiz_offset_ is enough information for redisplay */
  horiz_offset_ = horizOffset;

  // redraw all text
  redraw(DAMAGE_EXPOSE);
}

/*
 * Update the minimum, maximum, slider size, page increment, and value
 * for vertical scroll bar.
 */
void TextDisplay::update_v_scrollbar() {
  /* The Vert. scroll bar value and slider size directly represent the top
     line number, and the number of visible lines respectively.  The scroll
     bar maximum value is chosen to generally represent the size of the whole
     buffer, with minor adjustments to keep the scroll bar widget happy */
  vscrollbar->value(topline_num_, visiblelines_cnt_, 1, bufferlines_cnt_+2);
  vscrollbar->linesize(3);
}

/*
 * Update the minimum, maximum, slider size, page increment, and value
 * for the horizontal scroll bar.
 */
void TextDisplay::update_h_scrollbar(int longestvline) {
  if (longestvline==0) longestvline = longest_vline();
  int sliderMax = max(longestvline, text_area.w() + horiz_offset_);
  hscrollbar->value(horiz_offset_, text_area.w(), 0, sliderMax);
  hscrollbar->linesize(3);
  /*if (longestvline < text_area.w()) {
    hscrollbar->deactivate();
  } else {
    hscrollbar->activate();
  }*/
}

/*
 * Callbacks for drag or valueChanged on scroll bars
 */
void TextDisplay::v_scrollbar_cb(Scrollbar* b, TextDisplay* textD) {
  if (b->value() == textD->topline_num_) return;
  textD->scroll(b->value(), textD->horiz_offset_);
}

void TextDisplay::h_scrollbar_cb(Scrollbar* b, TextDisplay* textD) {
  if (b->value() == textD->horiz_offset_) return;
  textD->scroll(textD->topline_num_, b->value());
}

/*
 * Refresh the line number area.  If clearAll is False, writes only over
 * the character cell areas.  Setting clearAll to True will clear out any
 * stray marks outside of the character cell area, which might have been
 * left from before a resize or font change.
 */
void TextDisplay::draw_line_numbers(bool clearAll) {
  int Y, line, visLine, nCols, lineStart;
  char lineNumString[16];
  int lineHeight = maxsize_;
  int charWidth = stdfontwidth_;
    
  /* Don't draw if lineNumWidth == 0 (line numbers are hidden), or widget is
     not yet realized */
  if (linenumwidth_ == 0)
      return;
  
  /* Make sure we reset the clipping range for the line numbers GC, because
     the GC may be shared (eg, if the line numbers and text have the same
     color) and therefore the clipping ranges may be invalid. */
  Rectangle clipRect(linenumwidth_, h());
  box()->inset(clipRect);
  clipRect.w(linenumwidth_);
  
  fltk::push_clip(clipRect);
  
  // Erase background
  Color bg = fltk::lerp(color(), fltk::BLACK, .1f);
  fltk::setcolor(bg);
  fltk::fillrect(clipRect);

  // Draw separator line
  fltk::setcolor(fltk::lerp(color(), fltk::BLACK, .5f));
  fltk::drawline(clipRect.r()-1, clipRect.y(), clipRect.r()-1, clipRect.b());

  // Draw text
  fltk::setcolor(fltk::contrast(color(), bg));
  fltk::setfont(textfont(), textsize());

  /* Draw the line numbers, aligned to the text */
  nCols = linenumwidth_ / charWidth - 1;
  Y = clipRect.y();
  line = get_absolute_top_line_number();
  for (visLine=0; visLine < visiblelines_cnt_; visLine++) {
    lineStart = linestarts_[visLine];
    if (lineStart != -1 && (lineStart==0 || buffer()->character(lineStart-1)=='\n')) {
      int lineNumStringLen = sprintf(lineNumString, "%*d", nCols, line);
      fltk::drawtext(lineNumString, lineNumStringLen, (float)linenumleft_, (float)(Y + ascent_));
      line++;
    } else {
      if (visLine == 0)
        line++;
    }
    Y += lineHeight;
  }
  fltk::pop_clip();
}

/*
 * Count the number of newlines in a null-terminated text string;
 */
static int countlines( const char *string) {
  const char * c;
  int lineCount = 0;

  if (!string) return 0;

  for ( c = string; *c != '\0'; c++)
    if (*c == '\n') lineCount++;
  return lineCount;
}

/*
 * Return the width in pixels of the displayed line pointed to by "visLineNum"
 */
int TextDisplay::measure_vline( int visLineNum) {
  int i, width = 0, len, style, lineLen = vline_length(visLineNum);
  int charCount = 0, lineStartPos = linestarts_[visLineNum];
  char expandedChar[TEXT_MAX_EXP_CHAR_LEN];

  if (lineStartPos < 0 || lineLen == 0) return 0;

  if (stylebuffer_ == NULL) {
    for (i = 0; i < lineLen; i++) {
      len = buffer_->expand_character(lineStartPos + i,
	      			      charCount, expandedChar);
      setfont(textfont(), textsize());
      width += (int)fltk::getwidth(expandedChar, len);
      charCount += len;
    }
  } else {
    for ( i = 0; i < lineLen; i++) {
      len = buffer_->expand_character(lineStartPos + i,
				      charCount, expandedChar);
      
      style = (unsigned char)stylebuffer_->character(lineStartPos + i) - 'A';
      if (style < 0) {
	style = 0;
      } else if (style >= numstyles_) {
	style = numstyles_ - 1;
      }
      setfont(styletable_[style].font, styletable_[style].size);
      width += (int)fltk::getwidth(expandedChar, len);
      charCount += len;
    }
  }
  return width;
}

/*
 * Return true if there are lines visible with no corresponding buffer text
 */
int TextDisplay::empty_vlines() {
  return visiblelines_cnt_ > 0 && linestarts_[visiblelines_cnt_ - 1] == -1;
}

/*
** When the cursor is at the left or right edge of the text, part of it
** sticks off into the clipped region beyond the text.  Normal redrawing
** can not overwrite this protruding part of the cursor, so it must be
** erased independently by calling this routine.
*/
void TextDisplay::blank_cursor_protrusions()
{
  if (cursor_oldx_ >= 0 && cursor_oldy_ >= 0) {
    fltk::setcolor(color());
    fltk::fillrect(cursor_oldx_-1, cursor_oldy_, 2, maxsize_);
    cursor_oldx_ = cursor_oldy_ = -100;
  }
}

/*
 * Return the length of a line (number of displayable characters) by examining
 * entries in the line starts array rather than by scanning for newlines
 */
int TextDisplay::vline_length(int visLineNum) {
  int nextLineStart, lineStartPos;

  if (visLineNum < 0 || visLineNum >= visiblelines_cnt_)
    return (0);

  lineStartPos = linestarts_[visLineNum];

  if (lineStartPos == -1)
    return 0;
  if (visLineNum + 1 >= visiblelines_cnt_)
    return lastchar_ - lineStartPos;
  nextLineStart = linestarts_[visLineNum + 1];
  if (nextLineStart == -1)
    return lastchar_ - lineStartPos;
  if (wrap_uses_character(nextLineStart-1))
    return nextLineStart-1 - lineStartPos;
  return nextLineStart - lineStartPos;
}

/*
 * When continuous wrap is on, and the user inserts or deletes characters,
 * wrapping can happen before and beyond the changed position.  This routine
 * finds the extent of the changes, and counts the deleted and inserted lines
 * over that range.  It also attempts to minimize the size of the range to
 * what has to be counted and re-displayed, so the results can be useful
 * both for delimiting where the line starts need to be recalculated, and
 * for deciding what part of the text to redisplay.
 */
void TextDisplay::find_wrap_range(const char *deletedText, int pos,
				  int nInserted, int nDeleted, int *modRangeStart, int *modRangeEnd,
				  int *linesInserted, int *linesDeleted) {
  int length, retPos, retLines, retLineStart, retLineEnd;
  TextBuffer *deletedTextBuf, *buf = buffer();
  int nVisLines = visiblelines_cnt_;
  int *lineStarts = linestarts_;
  int countFrom, countTo, lineStart, adjLineStart, i;
  int visLineNum = 0, nLines = 0;

  /*
  ** Determine where to begin searching: either the previous newline, or
  ** if possible, limit to the start of the (original) previous displayed
  ** line, using information from the existing line starts array
  */
  if (pos >= firstchar_ && pos <= lastchar_) {
    for (i=nVisLines-1; i>0; i--)
      if (lineStarts[i] != -1 && pos >= lineStarts[i])
	break;
    if (i > 0) {
      countFrom = lineStarts[i-1];
      visLineNum = i-1;
    } else
      countFrom = buf->line_start(pos);
  } else
    countFrom = buf->line_start(pos);

  /*
  ** Move forward through the (new) text one line at a time, counting
  ** displayed lines, and looking for either a real newline, or for the
  ** line starts to re-sync with the original line starts array
  */
  lineStart = countFrom;
  *modRangeStart = countFrom;
  while (true) {

    /* advance to the next line.  If the line ended in a real newline
       or the end of the buffer, that's far enough */
    wrapped_line_counter(buf, lineStart, buf->length(), 1, true, 0,
			 &retPos, &retLines, &retLineStart, &retLineEnd);
    if (retPos >= buf->length()) {
      countTo = buf->length();
      *modRangeEnd = countTo;
      if (retPos != retLineEnd)
	nLines++;
      break;
    } else
      lineStart = retPos;
    nLines++;
    if (lineStart > pos + nInserted &&
	buf->character(lineStart-1) == '\n') {
      countTo = lineStart;
      *modRangeEnd = lineStart;
      break;
    }

    /* Don't try to resync in continuous wrap mode with non-fixed font
       sizes; it would result in a chicken-and-egg dependency between
       the calculations for the inserted and the deleted lines.
       If we're in that mode, the number of deleted lines is calculated in
       advance, without resynchronization, so we shouldn't resynchronize
       for the inserted lines either. */
    if (suppressresync_)
      continue;

    /* check for synchronization with the original line starts array
       before pos, if so, the modified range can begin later */
    if (lineStart <= pos) {
      while (visLineNum<nVisLines && lineStarts[visLineNum] < lineStart)
	visLineNum++;
      if (visLineNum < nVisLines && lineStarts[visLineNum] == lineStart) {
	countFrom = lineStart;
	nLines = 0;
	if (visLineNum+1 < nVisLines && lineStarts[visLineNum+1] != -1)
	  *modRangeStart = min(pos, lineStarts[visLineNum+1]-1);
	else
	  *modRangeStart = countFrom;
      } else
	*modRangeStart = min(*modRangeStart, lineStart-1);
    }

    /* check for synchronization with the original line starts array
       after pos, if so, the modified range can end early */
    else if (lineStart > pos + nInserted) {
      adjLineStart = lineStart - nInserted + nDeleted;
      while (visLineNum<nVisLines && lineStarts[visLineNum]<adjLineStart)
	visLineNum++;
      if (visLineNum < nVisLines && lineStarts[visLineNum] != -1 &&
	  lineStarts[visLineNum] == adjLineStart) {
	countTo = line_end(lineStart, true);
	*modRangeEnd = lineStart;
	break;
      }
    }
  }
  *linesInserted = nLines;


  /* Count deleted lines between countFrom and countTo as the text existed
     before the modification (that is, as if the text between pos and
     pos+nInserted were replaced by "deletedText").  This extra context is
     necessary because wrapping can occur outside of the modified region
     as a result of adding or deleting text in the region. This is done by
     creating a textBuffer containing the deleted text and the necessary
     additional context, and calling the wrappedLineCounter on it.

     NOTE: This must not be done in continuous wrap mode when the font
     width is not fixed. In that case, the calculation would try
     to access style information that is no longer available (deleted
     text), or out of date (updated highlighting), possibly leading
     to completely wrong calculations and/or even crashes eventually.
     (This is not theoretical; it really happened.)

     In that case, the calculation of the number of deleted lines
     has happened before the buffer was modified (only in that case,
     because resynchronization of the line starts is impossible
     in that case, which makes the whole calculation less efficient).
  */
  if (suppressresync_) {
    *linesDeleted = nlinesdeleted_;
    suppressresync_ = false;
    return;
  }

  length = (pos-countFrom) + nDeleted +(countTo-(pos+nInserted));
  deletedTextBuf = new TextBuffer(length);
  deletedTextBuf->copy(buffer(), countFrom, pos, 0);
  if (nDeleted != 0)
    deletedTextBuf->insert(pos-countFrom, deletedText);
  deletedTextBuf->copy(buffer(),
		       pos+nInserted, countTo, pos-countFrom+nDeleted);
  /* Note that we need to take into account an offset for the style buffer:
     the deletedTextBuf can be out of sync with the style buffer. */
  wrapped_line_counter(deletedTextBuf, 0, length, INT_MAX, true,
		       countFrom, &retPos, &retLines, &retLineStart, &retLineEnd, false);
  delete deletedTextBuf;
  *linesDeleted = retLines;
  suppressresync_ = false;
}

/*
 * This is a stripped-down version of the findWrapRange() function above,
 * intended to be used to calculate the number of "deleted" lines during
 * a buffer modification. It is called _before_ the modification takes place.
 *
 * This function should only be called in continuous wrap mode with a
 * non-fixed font width. In that case, it is impossible to calculate
 * the number of deleted lines, because the necessary style information
 * is no longer available _after_ the modification. In other cases, we
 * can still perform the calculation afterwards (possibly even more
 * efficiently).
 */
void TextDisplay::measure_deleted_lines(int pos, int nDeleted) {
  int retPos, retLines, retLineStart, retLineEnd;
  TextBuffer *buf = buffer();
  int nVisLines = visiblelines_cnt_;
  int *lineStarts = linestarts_;
  int countFrom, lineStart;
  int nLines = 0, i;

  /*
  ** Determine where to begin searching: either the previous newline, or
  ** if possible, limit to the start of the (original) previous displayed
  ** line, using information from the existing line starts array
  */
  if (pos >= firstchar_ && pos <= lastchar_) {
    for (i=nVisLines-1; i>0; i--)
      if (lineStarts[i] != -1 && pos >= lineStarts[i])
	break;
    if (i > 0) {
      countFrom = lineStarts[i-1];
    } else
      countFrom = buf->line_start(pos);
  } else
    countFrom = buf->line_start(pos);

  /*
  ** Move forward through the (new) text one line at a time, counting
  ** displayed lines, and looking for either a real newline, or for the
  ** line starts to re-sync with the original line starts array
  */
  lineStart = countFrom;
  while (true) {
    /* advance to the next line.  If the line ended in a real newline
       or the end of the buffer, that's far enough */
    wrapped_line_counter(buf, lineStart, buf->length(), 1, true, 0,
			 &retPos, &retLines, &retLineStart, &retLineEnd);
    if (retPos >= buf->length()) {
      if (retPos != retLineEnd)
	nLines++;
      break;
    } else
      lineStart = retPos;
    nLines++;
    if (lineStart > pos + nDeleted &&
	buf->character(lineStart-1) == '\n') {
      break;
    }

    /* Unlike in the findWrapRange() function above, we don't try to
       resync with the line starts, because we don't know the length
       of the inserted text yet, nor the updated style information.

       Because of that, we also shouldn't resync with the line starts
       after the modification either, because we must perform the
       calculations for the deleted and inserted lines in the same way.

       This can result in some unnecessary recalculation and redrawing
       overhead, and therefore we should only use this two-phase mode
       of calculation when it's really needed (continuous wrap + variable
       font width). */
  }
  nlinesdeleted_ = nLines;
  suppressresync_ = true;
}

/*
 * Count forward from startPos to either maxPos or maxLines (whichever is
 * reached first), and return all relevant positions and line count.
 * The provided textBuffer may differ from the actual text buffer of the
 * widget. In that case it must be a (partial) copy of the actual text buffer
 * and the styleBufOffset argument must indicate the starting position of the
 * copy, to take into account the correct style information.
 *
 * Returned values:
 *
 *   retPos:        Position where counting ended.  When counting lines, the
 *                  position returned is the start of the line "maxLines"
 *                  lines beyond "startPos".
 *   retLines:      Number of line breaks counted
 *   retLineStart:  Start of the line where counting ended
 *   retLineEnd:    End position of the last line traversed
 */
void TextDisplay::wrapped_line_counter(TextBuffer *buf, int startPos,
				       int maxPos, int maxLines, bool startPosIsLineStart, int styleBufOffset,
				       int *retPos, int *retLines, int *retLineStart, int *retLineEnd,
				       bool countLastLineMissingNewLine) {
  int lineStart, newLineStart = 0, b, p, colNum, wrapMargin;
  int maxWidth, i, foundBreak, width;
  bool countPixels;
  int nLines = 0, tabDist = buffer()->tab_distance();
  unsigned char c;
  char nullSubsChar = buffer()->null_substitution_character();

  /* If the font is fixed, or there's a wrap margin set, it's more efficient
     to measure in columns, than to count pixels.  Determine if we can count
     in columns (countPixels == False) or must count pixels (countPixels ==
     True), and set the wrap target for either pixels or columns */
  if (fixed_fontwidth_ != -1 || wrapmargin_ != 0) {
    countPixels = false;
    wrapMargin = wrapmargin_ ? wrapmargin_ : text_area.w() / fixed_fontwidth_;
    maxWidth = INT_MAX;
  } else {
    countPixels = true;
    wrapMargin = INT_MAX;
    maxWidth = text_area.w();
  }

  /* Find the start of the line if the start pos is not marked as a
     line start. */
  if (startPosIsLineStart)
    lineStart = startPos;
  else
    lineStart = line_start(startPos);

  /*
  ** Loop until position exceeds maxPos or line count exceeds maxLines.
  ** (actually, contines beyond maxPos to end of line containing maxPos,
  ** in case later characters cause a word wrap back before maxPos)
  */
  colNum = 0;
  width = 0;
  for (p=lineStart; p<buf->length(); p++) {
    c = (unsigned char)buf->character(p);
    if ((c & 0xC0) == 0x80) {
      // Skip UTF-8 sequence middle parts
      continue;
    }

    /* If the character was a newline, count the line and start over,
       otherwise, add it to the width and column counts */
    if (c == '\n') {
      if (p >= maxPos) {
	*retPos = maxPos;
	*retLines = nLines;
	*retLineStart = lineStart;
	*retLineEnd = maxPos;
	return;
      }
      nLines++;
      if (nLines >= maxLines) {
	*retPos = p + 1;
	*retLines = nLines;
	*retLineStart = p + 1;
	*retLineEnd = p;
	return;
      }
      lineStart = p + 1;
      colNum = 0;
      width = 0;
    } else {
      colNum += TextBuffer::character_width(c, colNum, tabDist, nullSubsChar);
      if (countPixels)
	width += measure_proportional_character(buf, p, colNum, p+styleBufOffset);
    }

    /* If character exceeded wrap margin, find the break point
       and wrap there */
    if (colNum > wrapMargin || width > maxWidth) {
      foundBreak = false;
      for (b=p; b>=lineStart; b--) {
	c = (unsigned char)buf->character(b);
        if ((c & 0xC0) == 0x80) {
          // Skip UTF-8 sequence middle parts
          continue;
        }
	if (c == '\t' || c == ' ') {
	  newLineStart = b + 1;
	  if (countPixels) {
	    colNum = 0;
	    width = 0;
	    for (i=b+1; i<p+1; i++) {
	      width += measure_proportional_character(buf, i, colNum,
						      i+styleBufOffset);
	      colNum++;
	    }
	  } else
	    colNum = buf->count_displayed_characters(b+1, p+1);
	  foundBreak = true;
	  break;
	}
      }
      if (!foundBreak) { /* no whitespace, just break at margin */
	newLineStart = max(p, lineStart+1);
	colNum = TextBuffer::character_width(c, colNum, tabDist, nullSubsChar);
	if (countPixels)
	  width = measure_proportional_character(buf, b, colNum, p+styleBufOffset);
      }
      if (p >= maxPos) {
	*retPos = maxPos;
	*retLines = maxPos < newLineStart ? nLines : nLines + 1;
	*retLineStart = maxPos < newLineStart ? lineStart :
	  newLineStart;
	*retLineEnd = maxPos;
	return;
      }
      nLines++;
      if (nLines >= maxLines) {
	*retPos = foundBreak ? b + 1 : max(p, lineStart+1);
	*retLines = nLines;
	*retLineStart = lineStart;
	*retLineEnd = foundBreak ? b : p;
	return;
      }
      lineStart = newLineStart;
    }
  }

  /* reached end of buffer before reaching pos or line target */
  *retPos = buf->length();
  *retLines = nLines;
  if (countLastLineMissingNewLine && colNum > 0)
    ++(*retLines);
  *retLineStart = lineStart;
  *retLineEnd = buf->length();
}

/*
 * Measure the width in pixels of a character "c" at a particular column
 * "colNum" and buffer position "pos".  This is for measuring characters in
 * proportional or mixed-width highlighting fonts.
 *
 * A note about proportional and mixed-width fonts: the mixed width and
 * proportional font code in nedit does not get much use in general editing,
 * because nedit doesn't allow per-language-mode fonts, and editing programs
 * in a proportional font is usually a bad idea, so very few users would
 * choose a proportional font as a default.  There are still probably mixed-
 * width syntax highlighting cases where things don't redraw properly for
 * insertion/deletion, though static display and wrapping and resizing
 * should now be solid because they are now used for online help display.
 */
int TextDisplay::measure_proportional_character(TextBuffer *buf, int bufpos, int colNum, int pos) {
  int charLen, style;
  char expChar[TEXT_MAX_EXP_CHAR_LEN];
  TextBuffer *styleBuf = stylebuffer_;
  char c = buf->character(bufpos);

  charLen = TextBuffer::expand_character(c, colNum, expChar,
					 buffer()->tab_distance(), buffer()->null_substitution_character());
  
  // UTF-8
  if (c & 0x80) {
    charLen = utf8seqlen(c);
    for (int n = 0; n<charLen; n++) {
      expChar[n] = buf->character(bufpos + n);
    }
  }

  if (styleBuf == 0) {
    style = 0;
  } else {
    style = (unsigned char)styleBuf->character(pos);
    if (style == unfinished_style_ && unfinished_highlight_cb_) {
      /* encountered "unfinished" style, trigger parsing */
      (unfinished_highlight_cb_)(pos, highlight_cbarg_);
      style = (unsigned char)styleBuf->character(pos);
    }
  }
  return string_width(expChar, charLen, style);
}

/*
 * Finds both the end of the current line and the start of the next line.  Why?
 * In continuous wrap mode, if you need to know both, figuring out one from the
 * other can be expensive or error prone.  The problem comes when there's a
 * trailing space or tab just before the end of the buffer.  To translate an
 * end of line value to or from the next lines start value, you need to know
 * whether the trailing space or tab is being used as a line break or just a
 * normal character, and to find that out would otherwise require counting all
 * the way back to the beginning of the line.
 */
void TextDisplay::find_line_end(int startPos, bool startPosIsLineStart,
				int *lineEnd, int *nextLineStart) {
  int retLines, retLineStart;

  /* if we're not wrapping use more efficient BufEndOfLine */
  if (!continuous_wrap_) {
    *lineEnd = buffer()->line_end(startPos);
    *nextLineStart = min(buffer()->length(), *lineEnd + 1);
    return;
  }

  /* use the wrapped line counter routine to count forward one line */
  wrapped_line_counter(buffer(), startPos, buffer()->length(),
		       1, startPosIsLineStart, 0, nextLineStart, &retLines,
		       &retLineStart, lineEnd);
}

/*
 * Line breaks in continuous wrap mode usually happen at newlines or
 * whitespace.  This line-terminating character is not included in line
 * width measurements and has a special status as a non-visible character.
 * However, lines with no whitespace are wrapped without the benefit of a
 * line terminating character, and this distinction causes endless trouble
 * with all of the text display code which was originally written without
 * continuous wrap mode and always expects to wrap at a newline character.
 *
 * Given the position of the end of the line, as returned by TextDEndOfLine
 * or BufEndOfLine, this returns true if there is a line terminating
 * character, and false if there's not.  On the last character in the
 * buffer, this function can't tell for certain whether a trailing space was
 * used as a wrap point, and just guesses that it wasn't.  So if an exact
 * accounting is necessary, don't use this function.
 */
int TextDisplay::wrap_uses_character(int lineEndPos) {
  char c;

  if (!continuous_wrap_ || lineEndPos == buffer()->length())
    return 1;

  c = buffer()->character(lineEndPos);
  return c == '\n' || ((c == '\t' || c == ' ') &&
		       lineEndPos + 1 != buffer()->length());
}

/*
 * Return true if the selection "sel" is rectangular, and touches a
 * buffer position withing "rangeStart" to "rangeEnd"
 */
int TextDisplay::range_touches_selection(TextSelection *sel,
					 int rangeStart, int rangeEnd) {
  return sel->selected() && sel->rectangular() && sel->end() >= rangeStart &&
    sel->start() <= rangeEnd;
}

/*
 * Extend the range of a redraw request (from *start to *end) with additional
 * redraw requests resulting from changes to the attached style buffer (which
 * contains auxiliary information for coloring or styling text). 
 */
void TextDisplay::extend_range_for_styles( int *startpos, int *endpos) {
  TextSelection * sel = stylebuffer_->primary_selection();
  bool extended = false;

  /* The peculiar protocol used here is that modifications to the style
     buffer are marked by selecting them with the buffer's primary TextSelection.
     The style buffer is usually modified in response to a modify callback on
     the text buffer BEFORE TextDisplay.c's modify callback, so that it can keep
     the style buffer in step with the text buffer.  The style-update
     callback can't just call for a redraw, because TextDisplay hasn't processed
     the original text changes yet.  Anyhow, to minimize redrawing and to
     avoid the complexity of scheduling redraws later, this simple protocol
     tells the text display's buffer modify callback to extend it's redraw
     range to show the text color/and font changes as well. */
  if (sel->selected()) {
    if (sel->start() < *startpos) {
      *startpos = sel->start();
      extended = true;
    }
    if (sel->end() > *endpos) {
      *endpos = sel->end();
      extended = true;
    }
  }

  /* If the TextSelection was extended due to a style change, and some of the
     fonts don't match in spacing, extend redraw area to end of line to
     redraw characters exposed by possible font size changes */
  if (fixed_fontwidth_ == -1 && extended)
    *endpos = buffer_->line_end(*endpos) + 1;
}

// The draw() method.  It tries to minimize what is draw as much as possible.
void TextDisplay::draw(void) {
  // don't even try if there is no associated text buffer!
  if (!buffer()) { 
    draw_box(); 
    return; 
  }

  // prevent drawing outside widget area
  drawstyle(style(), flags()|OUTPUT);

  // draw the non-text, non-scrollbar areas.
  if (damage() & DAMAGE_ALL) {
    draw_frame();
    setcolor(color());

    // left margin
    fillrect(text_area.x()-LEFT_MARGIN, text_area.y()-TOP_MARGIN,
		       LEFT_MARGIN, text_area.h()+TOP_MARGIN+BOTTOM_MARGIN);
    // right margin
    fillrect(text_area.x()+text_area.w(), text_area.y()-TOP_MARGIN,
		       RIGHT_MARGIN, text_area.h()+TOP_MARGIN+BOTTOM_MARGIN);
    // top margin
    fillrect(text_area.x(), text_area.y()-TOP_MARGIN,
		       text_area.w(), TOP_MARGIN);
    // bottom margin
    fillrect(text_area.x(), text_area.y()+text_area.h(),
		       text_area.w(), BOTTOM_MARGIN);

    // draw that little box in the corner of the scrollbars
    if (vscrollbar->visible() && hscrollbar->visible()) {
      setcolor(buttoncolor());
      fillrect(vscrollbar->x(), hscrollbar->y(),
			 vscrollbar->w(), hscrollbar->h());
    }
    // blank the previous cursor protrusions
  }

  // draw the scrollbars
  if (damage() & (DAMAGE_ALL | DAMAGE_CHILD)) {
    vscrollbar->set_damage(DAMAGE_ALL);
    hscrollbar->set_damage(DAMAGE_ALL);
  }
  update_child(*vscrollbar);
  update_child(*hscrollbar);

  if (damage() & (DAMAGE_ALL | DAMAGE_EXPOSE | DAMAGE_SCROLL)) {
    // erase cursor artifacts
    blank_cursor_protrusions();
  }

  if (damage() & (DAMAGE_ALL | DAMAGE_EXPOSE)) {
    fltk::push_clip(text_area);
    // draw all of the text
    draw_text(text_area.x(), text_area.y(), text_area.w(), text_area.h());
    fltk::pop_clip();

    // draw the line numbers if exposed area includes them
    if (linenumwidth_ != 0) {
      draw_line_numbers(false);
    }
  }
  else if (damage() & DAMAGE_SCROLL) {
    fltk::push_clip(text_area);
    // draw some lines of text
    draw_range(damage_range1_start, damage_range1_end);
    if (damage_range2_end != -1) {
      draw_range(damage_range2_start, damage_range2_end);
    }
    damage_range1_start = damage_range1_end = -1;
    damage_range2_start = damage_range2_end = -1;
    fltk::pop_clip();
  }
}

// this processes drag events due to mouse for TextDisplay and
// also drags due to cursor movement with shift held down for
// TextEditor
void TextDisplay::text_drag_me(int pos) {
  if (dragtype_ == TextDisplay::DRAG_CHAR) {
    if (pos >= dragpos_) {
      buffer()->select(dragpos_, pos);
    } else {
      buffer()->select(pos, dragpos_);
    }
    insert_position(pos);
  } else if (dragtype_ == TextDisplay::DRAG_WORD) {
    if (pos >= dragpos_) {
      insert_position(word_end(pos));
      buffer()->select(word_start(dragpos_), word_end(pos));
    } else {
      insert_position(word_start(pos));
      buffer()->select(word_start(pos), word_end(dragpos_));
    }
  } else if (dragtype_ == TextDisplay::DRAG_LINE) {
    if (pos >= dragpos_) {
      insert_position(buffer()->line_end(pos)+1);
      buffer()->select(buffer()->line_start(dragpos_),
		       buffer()->line_end(pos)+1);
    } else {
      insert_position(buffer()->line_start(pos));
      buffer()->select(buffer()->line_start(pos),
		       buffer()->line_end(dragpos_)+1);
    }
  }
}

int TextDisplay::handle(int event) {
  if (!buffer()) 
    return Group::handle(event);

  switch (event) {
  case ENTER:
  case MOVE:
    //if (active_r()) cursor(CURSOR_INSERT);
    return Group::handle(event);

  case PUSH: {
    if (Group::handle(event)) return true;
    if (!event_inside(Rectangle(text_area.x(), text_area.y(), text_area.w(), text_area.h()))) {
      return Group::handle(event);
    }
    if (event_state()&SHIFT) return handle(DRAG);
    dragging_ = 1;
    int pos = xy_to_position(event_x(), event_y(), CURSOR_POS);
    dragtype_ = event_clicks();
    dragpos_ = pos;
    if (dragtype_ == DRAG_CHAR)
      buffer()->unselect();
    else if (dragtype_ == DRAG_WORD)
      buffer()->select(word_start(pos), word_end(pos));
    else if (dragtype_ == DRAG_LINE)
      buffer()->select(buffer()->line_start(pos), buffer()->line_end(pos)+1);

    if (buffer()->primary_selection()->selected())
      insert_position(buffer()->primary_selection()->end());
    else
      insert_position(pos);
    show_insert_position();
    return 1;
  }

  case DRAG: {
    if (dragtype_ < 0) return 1;
    int X = event_x(), Y = event_y(), pos;
    if (Y < text_area.y()) {
      move_up();
      scroll(topline_num_ - 1, horiz_offset_);
      pos = insert_position();
    } else if (Y >= text_area.y()+text_area.h()) {
      move_down();
      scroll(topline_num_ + 1, horiz_offset_);
      pos = insert_position();
    } else 
      pos = xy_to_position(X, Y, CURSOR_POS);
    text_drag_me(pos);
    return 1;
  }

  case RELEASE: {
    dragging_ = 0;

    // convert from WORD or LINE selection to CHAR
    if (insert_position() >= dragpos_) {
      dragpos_ = buffer()->primary_selection()->start();
    } else {
      dragpos_ = buffer()->primary_selection()->end();
    }
    dragtype_ = DRAG_CHAR;

    char *copy = buffer()->selection_text();
    if (*copy) {
      fltk::copy(copy, strlen(copy), 0);
    }
    free(copy);
    return 1;
  }

  case MOUSEWHEEL:
    if (event_key() == WheelUp || event_key() == WheelDown) {
      return vscrollbar->handle(event);
    } else {
      return hscrollbar->handle(event);
    }

  case FOCUS:
  case UNFOCUS:
    // disable input method
    fl_set_spot(NULL, this, 0, 0);
    if (buffer()->selected()) redraw();
    return 1;

  case KEY:
    // Copy?
    if ((event_state()&(CTRL|META)) && event_key()=='c') {
      if (buffer()->selected()) {
        char *copy = buffer()->selection_text();
        if (*copy) {
          fltk::copy(copy, strlen(copy), 1);
        }
        free(copy);
      }
      return 1;
    }

    // Select all ?
    if ((event_state()&(CTRL|META)) && event_key()=='a') {
      buffer()->select(0,buffer()->length());
      return 1;
    }
    break;
  }

  return 0;
}

//
// End of "$Id: TextDisplay.cxx 8313 2011-01-25 15:44:40Z bgbnbigben $".
//
