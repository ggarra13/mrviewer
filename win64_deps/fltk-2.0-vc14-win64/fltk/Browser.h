// "$Id: Browser.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// Copyright 2006 by Bill Spitzak and others.
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

#ifndef fltk_Browser_h
#define fltk_Browser_h

#include "Menu.h"
#include "Scrollbar.h"

namespace fltk {

class FL_API Browser : public Menu {
public:
  
  /** The Browser's custom handle function \n
      See Widget::handle
    \param event The event to be handled
    \return 1 if the event was successfully handled, 0 otherwise
  */
  int handle(int);
  /** The Browser's custom layout function \n
      see Widget::layout
  */
  void layout();
  /** The standard draw function. See Widget::draw */
  void draw();

  Browser(int X,int Y,int W,int H,const char*l=0);
  static NamedStyle* default_style;
  ~Browser();
  
  /** Values for type() */
  enum {
    IS_MULTI = 1, 	   //!< added to make testing with type() & IS_MULTI easier
    NORMAL =  GROUP_TYPE,  //!< means single selection can be achieved by user 
    MULTI  =  GROUP_TYPE+1 //!< means multiple selection can be achieved by user
  };
  /** Values for selected_column */
  enum {
    NO_COLUMN_SELECTED = -1 //!< means that no column has been selected by user
  };

  /** A "Mark" is like a pointer ot a widget somewhere in the hierarchy
      of the Browser. It is an array of child indicies, and also the
      vertical position and indexnumber (which is used to decide strip
      colour) of the item
  */
  class FL_API Mark {
    friend class fltk::Browser;
    unsigned level; //!< depth in hierarchy of the item
    unsigned open_level; //!< depth of highest closed parent
    int position;  //!< distance in pixels from top of browser
    unsigned indexes_size; //!< allocated size
    int* indexes;  //!< array of indexes
    int index0;    //!< used as *indexes if indexes_size==1
  public:
    /** The default constructor initialises every member to 0 */
    Mark() {
      level = 0;
      open_level = 0;
      position = 0;
      indexes_size = 1;
      indexes = &index0;
      index0 = 0;
    }
    Mark(const Mark&);
    /** The destructor deletes the array of cihld indicies */
    ~Mark() {
      if (indexes != &index0) delete[] indexes;
    }
    const Mark& operator=(const Mark&);
    int compare(const Mark& mark2) const;
    /** Removes this Mark */
    void unset() {indexes[0] = -1;}
    /** Checks whether this Mark is being used
      \return true if it's being used, false otherwise
    */
    bool is_set() const {return indexes[0] >= 0;}
  };

  /** Argument to make_item_visible() */
  enum linepos {     
    NOSCROLL,	    //!< move as little as possible so item is visible
    TOP,	    //!< position current item to top
    MIDDLE,	    //!< position current item to middle
    BOTTOM	    //!< position current item to bottom
  };

  int width() const {return width_;}
  int height() const {return height_;}
  int box_width() const {return interior.w();}
  int box_height() const {return interior.h();}
  /** \return The horizontal scrolling position measured in pixels */
  int xposition() const {return xposition_;}
  void xposition(int);
  /** \return The vertical scrolling position measured in pixels */
  int yposition() const {return yposition_;}
  void yposition(int);
  /** \return Whether or not indentation has been turned on, to facilitate drawing open/close boxes to the left of items */
  bool indented() const {return indented_;}
  void indented(bool v) {indented_ = v;}

  Scrollbar scrollbar; //!< The vertical scrollbar
  Scrollbar hscrollbar; //!< The horizontal scrollbar

  Widget* goto_top();
  Widget* goto_focus() {return goto_mark(FOCUS);}
  Widget* goto_position(int y);
  Widget* goto_index(const int* indexes, unsigned level);
  Widget* goto_index(int);
  Widget* goto_index(int,int,int=-1,int=-1,int=-1);
  Widget* goto_mark(const Mark&); // set HERE to mark
  Widget* next();
  Widget* next_visible();
  Widget* previous_visible();
  /** \return whether or not we're at the current Mark */
  bool at_mark(const Mark& mark) const {return !HERE.compare(mark);} 
  /** \returns The values described in Mark::compare, based on the current position */
  int compare_to_mark(const Mark& mark) const {return HERE.compare(mark);} 
  bool item_is_visible() const;
  bool item_is_parent() const;
  bool item_is_open() const;
  int item_h() const;

  bool set_focus();
  /** Set the browser's current Mark to the passed value
     \param dest The Mark to be set
  */
  void set_mark(Mark& dest) const {dest = HERE;}
  /** Set the browser's current focused Mark to the passed value
     \param dest The Mark to be set
  */
  void set_mark_to_focus(Mark& dest) const {dest = FOCUS;}
  bool select(Widget*e, int val, int do_callback=0);
  bool set_item_selected(bool value = true, int do_callback = 0);
  bool select_only_this(int do_callback = 0);
  bool deselect(int do_callback = 0);
  bool make_item_visible(linepos = NOSCROLL);
  void damage_item() {damage_item(HERE);}
  void damage_item(const Mark&); // make this item redraw
  bool set_item_opened(bool);
  bool set_item_visible(bool);

  int current_level() const {return HERE.level;}
  const int* current_index() const {return HERE.indexes;}
  int current_position() const {return HERE.position;}

  int focus_level() const {return FOCUS.level;}
  const int* focus_index() const {return FOCUS.indexes;}
  int focus_position() const {return FOCUS.position;}

  // Maybe these should be moved to base Menu class:
  /** \return a constant pointer to the integer width of the columns */
  const int *column_widths() const { return column_widths_p; }
  void column_widths(const int *pWidths);
  /** \return a constant array of the labels used for columns */
  const char **column_labels() const { return column_labels_; }
  void column_labels(const char **pLabels);
  int selected_column() { return selected_column_; }
  int set_column_start(int column, int x);

  Widget *header(int col) { if(col<0 || col>=nHeader) return 0; return header_[col]; }
  int nheader() const { return nHeader; }

  // Convienence functions for flat browsers:
  void value(int v) {goto_index(v); set_focus();}
  int value() const {return FOCUS.indexes[0];}
  bool select(int line, bool value = true);
  bool selected(int line);
  int topline() const {return FIRST_VISIBLE.indexes[0];}
  void topline(int line) {goto_index(line); make_item_visible(TOP);}
  void bottomline(int line) {goto_index(line); make_item_visible(BOTTOM);}
  void middleline(int line) {goto_index(line); make_item_visible();}
  bool displayed(int line);
  bool display(int line, bool value = true);

  bool display_lines() const;
  void display_lines(bool display);

  int load(const char *filename);
  
  /** \return Whether or not this browser has its type() set to include MULTI */
  int multi() const {return type()&IS_MULTI;}

  /** \return The symbol used to draw an item that is not a parent */
  const Symbol* leaf_symbol() const {return leaf_symbol_;}
  void leaf_symbol(const Symbol* s) {leaf_symbol_ = s;}
  /** \return The symbol used to draw on each hierarchy parent item */
  const Symbol* group_symbol() const {return group_symbol_;}
  void group_symbol(const Symbol* s) {group_symbol_ = s;}

protected:
  /** Defines how callbacks are handled in the browser
    \param doit no idea.
    \a This \a function \a looks \a NYI
    \todo Implement it, if I can work out what it does
  */
  void handle_callback(int doit);

private:
  bool displaylines_; //!< Flag to determine whether or not to draw lines
  bool indented_;	     //!< Flag to determine whether or not to draw + symbols
  const int *column_widths_; //!< Original column widths
  int *column_widths_i;      //!< Original column widths after user interaction
  int *column_widths_p;      //!< Actual column widths
  const char **column_labels_; //!< The labels to be used across the top of the Browser
  int xposition_, //!< The horizontal scrolling position
      yposition_; //!< The vertical scrolling position
  int width_,  //!< The width of the longest item in the browser
      height_; //!< The height of all the visible items
  int scrolldx, //!< The amount the user has scrolled horizontally since the last redraw
      scrolldy; //!< The amount the user has scrolled vertically since the last redraw
  static void hscrollbar_cb(Widget*, void*);
  static void scrollbar_cb(Widget*, void*);
  void draw_item(int);
  void draw_clip(const Rectangle&);
  static void draw_clip_cb(void*,const Rectangle&);
  Rectangle interior; //!< A Rectangle representing the area inside box edges and scrollbars
  void set_belowmouse();
  void clear_belowmouse();

  Widget **header_; //!< The set of widgets heading each column
  int nHeader, //!< The amount of widgets heading each column
      nColumn, //!< The amount of columns
      selected_column_; //!< The index of the currently selected column
  void set_level(unsigned);
  /** The maximum number of items to redraw at a time */
  enum {
    NUM_REDRAW = 2 //!< At most 2 items can be redrawn at a time
  };
  Mark HERE, //!< The current item
   FOCUS, //!< The item that has focus
   FIRST_VISIBLE, //!< The first visible item
   REDRAW[NUM_REDRAW], //!< An array of Marks that need to be redrawn on the next call to redraw()
   OPEN, //!< The item that has been opened 
   BELOWMOUSE; //!< The Mark that's below the mouse

  /** Sets HERE to the currently focused widget, if it's visible
      \return A pointer to the widget with focus
  */
  Widget* goto_visible_focus();

  int siblings; //!< Number of children of the parent of the HERE item
  static void column_click_cb_(Widget*, void*);

  const Symbol* leaf_symbol_; //!< The symbol used to draw child items.
  const Symbol* group_symbol_; //!< The symbol used to draw parent items
};

}

#endif

//
// End of "$Id: Browser.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
