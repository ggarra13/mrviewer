//
// "$Id: TabGroup.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// For making a "tabbed dialog box". Each child widget creates a tab.
// Only one is visible at a time. This works best if all the children
// have the same size and they don't cover a strip at the top for the
// tabs.
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

#ifndef fltk_TabGroup_h
#define fltk_TabGroup_h

#include "Group.h"

#define MIN_TABSIZE 30

namespace fltk {

class TabGroup;


enum {TAB_LEFT, TAB_RIGHT, TAB_SELECTED};

class FL_API TabGroupPager {
public:

    TabGroupPager() : shift_(0), border_(10), slope_(5), extra_space_(5),_noclip(false) {}
    
    /* Pager template methods definition */

    /*! this method must update the tab positions and width array, returns the selected tab    */
    virtual int update_positions(TabGroup* g, int numchildren, int& selected, 
	int& cumulated_width, int& available_width , int* tab_pos, int* tab_width) { return selected;}
    /*! draw the tabs for this custom pager, return false means no custom draw is made */
    virtual bool draw_tabs(TabGroup* g, int selected, int* tab_pos, int* tab_width) { return false;}
    /*! determine and return the index of the child group at the corresponding pos */
    virtual int which(TabGroup* g, int m_x,int m_y)=0;
    /*! virtual copy from a prototype */
    virtual TabGroupPager* clone() const=0;

    virtual const char * mode_name() const = 0;
    virtual int id() const = 0;

    /*! return max width available for drawing tab thumbnails */
    virtual int available_width(TabGroup *g ) const;

    // tab appearance
    int  border()	const {return border_;}
    int  slope()		const {return slope_;}
    int  extra_space()	const {return extra_space_;}
    void border(int v)	{border_=v;}
    void slope(int v)	{slope_=v;}
    void extra_space(int v) {extra_space_=v;}
    
    /*! shifting to nth+1 tab to draw, permit to 'keep' a position while 
	changing tabs and tab is in the interval
    */
    int  shift()	const {return shift_;}
    void shift(int v)	{shift_=v;}
    int spacing()	const {return slope_+extra_space_;}
    /*! determines if we forbid partial tabs drawing with clipping */
    void noclip(bool v) {_noclip=v;}
  
private:
    int shift_, border_, slope_, extra_space_;
    bool _noclip; 
  
};

// fltk default factory pagers
const int PAGER_MENU   = 0; //<-- two left and right buttons  provide prev page  and next page 
const int PAGER_SHRINK = 1; //<-- tabs outside rect are shrinked to very small slice to fit


class FL_API TabGroup : public Group {

public:
  int handle(int);

  TabGroup(int,int,int,int,const char * = 0, bool begin=false);
  ~TabGroup() {delete pager_;}
  static NamedStyle* default_style;

  int value() const;
  bool value(int);
  int which(int event_x, int event_y);
  Widget *selected_child();
  bool selected_child(Widget *);
  void set_draw_outline( bool draw_outline );
  
  //! setting the pager_ to a tabgroup, pager is _never_ null by design
  void pager(TabGroupPager * value);
  //! returning the current pager_ responsible of this instance
  TabGroupPager * pager() const {return pager_;}
  //! setting the default pager_ for future tabgroups, a default pager is _never_ null by design
  static void default_pager(TabGroupPager * v);
  //! setting the default pager_ from the built-in ones
  static void default_pager(int factory_pager_index);

  int tab_height();
  int tab_positions(int*, int*);

  void draw_tab(int x1, int x2, int W, int H, Widget* o, int sel=0);
  void draw_tab_background();

protected:
  void draw();

private:
  int push(Widget *);
  bool _drawOutline;
  TabGroupPager* pager_;
  static TabGroupPager* default_pager_;
};

}

#endif

//
// End of "$Id: TabGroup.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
