// "$Id"
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

/*! \class fltk::TabGroupPager 
    This Helper class defines generically the TabGroup behavior when 
    there is more tabs than available horizontal width, 
    i.e it can generates a popup menu or shrink 
    It opens the door to new Pagers creation as 
    left- rights arrows scrolling pagers and others ...
    To create a new pager, inherits from TabGroupPager
    and redefines the pure virtual methods of this template.
*/

#include <fltk/Box.h>
#include <fltk/draw.h>

#include <fltk/TabGroup.h>
#include <fltk/MenuBuild.h>
#include <fltk/ask.h>

using namespace fltk;

const int BORDER = 10;
const int TABSLOPE = 5;
const int EXTRASPACE = 5;

const int BTN_WIDTH  = 14;
const int BTN_HEIGHT = 14;

////////////////////////////////////////////////////////////////////////////////////
class FL_API ShrinkTabPager : public TabGroupPager {
public:
    virtual int update_positions(TabGroup* g, int nc, int& sel, int & cw, int& aw,int *p, int *w);
    virtual int which(TabGroup* g, int m_x,int m_y);
    virtual TabGroupPager* clone() const;
    virtual const char * mode_name() const {return "Shrink";}
    virtual int id() const {return PAGER_SHRINK;}
    
    ShrinkTabPager() : TabGroupPager() {noclip(true);}
};

////////////////////////////////////////////////////////////////////////////////////
class FL_API MenuTabPager : public TabGroupPager {
public:
    virtual int update_positions(TabGroup* g, int nc, int& sel, int & cw, int& aw,int *p, int *w);
    virtual bool draw_tabs(TabGroup* g, int selected, int* tab_pos, int* tab_width);
    virtual int which(TabGroup* g, int m_x,int m_y);
    virtual TabGroupPager* clone() const;
    virtual int available_width(TabGroup *g ) const;
    
    virtual const char * mode_name() const {return "Menu";}
    virtual int id() const {return PAGER_MENU;}
    MenuTabPager() : TabGroupPager(), extension_(0) {
	
    }
private:
    PopupMenu * extension_;
    void createExtMenu(TabGroup * g);
};

////////////////////////////////////////////////////////////////////////////////////
TabGroupPager* TabGroup::default_pager_= new MenuTabPager();

int TabGroupPager::available_width(TabGroup *g ) const {
    return g->w() - this->slope()-1;
}

//! setting the default pager_ from the built-in ones
void TabGroup::default_pager(int factory_pager_index) {
      switch (factory_pager_index) {
      case PAGER_MENU:
	  default_pager(new MenuTabPager);
	  break;
      case PAGER_SHRINK: // historically the first way to handle tabs in fltk2
	  default_pager(new ShrinkTabPager);
	  break;
      }
  }
////////////////////////////////////////////////////////////////////////////////////

// shrinking tab paging mode:
int ShrinkTabPager::update_positions(TabGroup* g, int numchildren, int& selected, 
				     int& cumulated_width, int& available_width , int* tab_pos, int* tab_width) {
    // uh oh, they are too big, we must move them:
    // special case when the selected tab itself is too big, make it fill cumulated_width:
    int i;
    if (tab_width[selected] >= available_width) {
	tab_width[selected] = available_width;
	for (i = 0; i <= selected; i++) tab_pos[i] = 0;
	for (i = selected+1; i <= numchildren; i++) tab_pos[i] = available_width;
	return selected ;
    }
    int w2[128];
    for (i = 0; i < numchildren; i++) w2[i] = tab_width[i];
    i = numchildren-1;
    int j = 0;
    int minsize = TABSLOPE;
    bool right = true;
    while (cumulated_width > available_width) {
	int n; // which one to shrink
	if (j < selected && (!right || i <= selected)) { // shrink a left one
	    n = j++;
	    right = true;
	} else if (i > selected) { // shrink a right one
	    n = i--;
	    right = false;
	} else { // no more space, start making them zero
	    minsize = 0;
	    i = numchildren-1;
	    j = 0;
	    right = true;
	    continue;
	}
	cumulated_width -= w2[n]-minsize;
	w2[n] = minsize;
	if (cumulated_width < available_width) {
	    w2[n] = available_width-cumulated_width+minsize;
	    cumulated_width = available_width;
	    break;
	}
    }
    // re-sum the positions:
    cumulated_width = 0;
    for (i = 0; i < numchildren; i++) {
	cumulated_width += w2[i];
	tab_pos[i+1] = cumulated_width;
    }
    return selected;
}

////////////////////////////////////////////////////////////////
int ShrinkTabPager::which(TabGroup* g, int event_x,int event_y) {
    int H = g->tab_height();
    if (!H) return -1;
    if (H < 0) {
	if (event_y > g->h() || event_y < g->h()+H) return -1;
    } else {
	if (event_y > H || event_y < 0) return -1;
    }
    if (event_x < 0) return -1;
    int p[128], w[128];
    int selected = g->tab_positions(p, w);
    int d = (event_y-(H>=0?0:g->h()))*slope()/H;
    for (int i=0; i<g->children(); i++) {
	if (event_x < p[i+1]+(i<selected ? slope() - d : d)) return i;
    }
    return -1;
    
}
////////////////////////////////////////////////////////////////
TabGroupPager* ShrinkTabPager::clone() const {
    return new ShrinkTabPager(*this);
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

TabGroupPager* MenuTabPager::clone() const {
    return new MenuTabPager(*this);
}

////////////////////////////////////////////////////////////////
int MenuTabPager::available_width(TabGroup *g ) const {
    return g->w() - this->slope()-1;
}

static inline int MAX(int a, int b) {return a>b ? a : b;}

////////////////////////////////////////////////////////////////
int MenuTabPager::update_positions(TabGroup* g, int numchildren, int& selected, 
				   int& cumulated_width, int& available_width , 
				   int* tab_pos, int* tab_width) {
    // width does not change, only position
    int i,j, offset;
    // calc real offset
    offset = tab_pos[shift()];
    if (!shift()) {
	for (i=g->children()-1; i; i--) {
	    if (tab_pos[i]>available_width) 
		{available_width-=BTN_WIDTH;break;}
	}
    } else {available_width-=BTN_WIDTH;}

    if (tab_pos[selected]+tab_width[selected]-offset > available_width) {
	// set selected to right 
	tab_pos[selected] = MAX(available_width-tab_width[selected],0);
	for (i=selected;i; i--) {
	    tab_pos[i-1] = tab_pos[i] - tab_width[i-1];
	}
	for (i=selected+1;i<numchildren; i++) {
	    tab_pos[i] = tab_pos[i-1]+tab_width[i-1];
	}
	// now correct left align offset and clip
	int start = 0;
	for (i=0; i<=selected && tab_pos[i]<start;i++);
	for (offset=tab_pos[i]-start, j=i; i<=numchildren-1; i++) tab_pos[i] -=offset;
	for (shift(0), j=0;j<i;j++) {
	    if (tab_pos[j] <start) tab_pos[j] =start;
	    else {
		if (!shift()) 
		    shift(j); // take the first non neg value as shift value
	    }
	}
    }
    else { // apply last position offsets
	for (i=0; i< numchildren; i++) tab_pos[i] -=offset;
	for (i=0; i<shift(); i++) if(tab_pos[i]<0) tab_pos[i] =0;
    }
    return selected;
}

////////////////////////////////////////////////////////////////
int MenuTabPager::which(TabGroup* g, int event_x,int event_y) {
    int H = g->tab_height();
    if (!H) return -1;
    if (H < 0) {
	if (event_y > g->h() || event_y < g->h()+H) return -1;
    } else {
	if (event_y > H || event_y < 0) return -1;
    }
    if (event_x < 0) return -1;
    int p[128], w[128];
    int selected = g->tab_positions(p, w);
    int d = (event_y-(H>=0?0:g->h()))*slope()/H;
    for (int i=0; i<g->children(); i++) {
	if (event_x < p[i+1]+(i<selected ? slope() - d : d)) return i;
    }
    return -1;
    
}
////////////////////////////////////////////////////////////////
#include <stdio.h>
bool MenuTabPager::draw_tabs(TabGroup* g, int selected, int* p, int* w) {
    int i;
    int H = g->tab_height();
    fltk::Widget *v = g->selected_child();
    int r = available_width(g);
    bool want_extension=false;
    

    if (shift()==0) {
	for (i=g->children()-1; i; i--) {
	    if (p[i]+w[i]>r) {r-=BTN_WIDTH;want_extension=true; break;}
	}
    } else {r-=BTN_WIDTH;want_extension=true;}
    
    for (i=0;i<g->children()-1; i++) if (p[i]+w[i]>r) break;
    //printf("%c last %d (p,w,p+w) = (%d,%d,%d),win width=%d, avail width = %d\n",want_extension ? 'E' : ' ',i, p[i],w[i],p[i]+w[i],g->w(),r);

    for (i=shift()-1; i>=0; i--) 
	// when resizing, thumbnails behind the shift may be drawable again
	// let's get them and update the shift consequently 
	if (p[i]>=0 && 	p[i]+w[i]<=p[shift()]) {
	    g->draw_tab(p[i], p[i]+w[i], w[i], H, g->child(i), TAB_LEFT);
	    shift(i);
	}
    for (i=this->shift(); i<selected; i++) 
	g->draw_tab(p[i], p[i]+w[i], w[i], H, g->child(i), TAB_LEFT);
    for (i=g->children()-1; i > selected; i--) 
	if (p[i]+w[i]<=r)
	    g->draw_tab(p[i], p[i]+w[i], w[i], H, g->child(i), TAB_RIGHT);
    
    if (v) {
	i = selected;
	if (p[i]+w[i]<=r)
	    g->draw_tab(p[i], p[i]+w[i], w[i], H, g->child(i), TAB_SELECTED);
    } else {
	// draw the edge when no selection:
	fltk::setcolor(H >= 0 ? GRAY99 : GRAY33);
	int b = H >= 0 ? H : g->h() + H;
	fltk::drawline(0, b, g->w(), b);
    }
    if (want_extension) {
	createExtMenu(g);
    } else {
      if (extension_ && extension_->visible()) {
        extension_->clear_visible();
        // g->redraw(); // I think it is already erased
      }
    }
    return true;
}

////////////////////////////////////////////////////////////////
static void btnCb(Widget * w, void * data) {
    Widget * wi = (Widget * ) ((Menu*) w)->get_item()->user_data();
    TabGroup * g =  (TabGroup*) wi->parent();
    //fltk::message("%s, %08p",wi->label(), wi);
    g->pager()->shift(0);
    g->selected_child(wi);
    g->redraw();
}

////////////////////////////////////////////////////////////////
void MenuTabPager::createExtMenu(TabGroup* g){
    if (!extension_) {
	Group *cur = Group::current();
	Group::current(0);
	extension_ = new PopupMenu(0,0, BTN_WIDTH,BTN_HEIGHT,"@>>");
	g->parent()->add(extension_);
	Group::current(cur);
	extension_->callback(btnCb);
	extension_->buttonbox(FLAT_BOX);
    }
    extension_->set_visible();
    extension_->clear();
    for (int i=0;i<g->children(); i++) {
	Widget  * c = g->child(i);
	if (c) {
	    Widget* wi = extension_->add(c->label());
	    wi->user_data((void*) c);
	}
    }
    int th = g->tab_height()+1;
    th = MAX((th-BTN_HEIGHT)/2,0)+g->y();
    extension_->resize(g->r()-BTN_WIDTH, th, BTN_WIDTH,BTN_HEIGHT);
    extension_->y(th);
    extension_->redraw();
}


// End of "$Id"
