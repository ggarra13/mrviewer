/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvCollapsableGroup.cpp
 * @author gga
 * @date   Tue Aug  7 04:52:02 2007
 * 
 * @brief  
 * 
 * 
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <iostream>
using namespace std;

#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

#include "mrvCollapsableGroup.h"

#define BUTTON_H        20
#define GROUP_MARGIN    8               // compensates for FL_ROUND_BUTTON (?)

namespace mrv {

  // Change button label based on open/closed state of pack
  void CollapsableGroup::relabel_button() {
    int open = _contents->visible() ? 1 : 0;
    char buf[256];
    // Draw arrow in label
    //    Text to the right of arrow based on parent's label()
    //
    if ( open ) sprintf( buf, "@2>  %s", label() ? label() : "(no label)");
    else        sprintf( buf, "@>  %s",  label() ? label() : "(no label)");
    _button->copy_label( buf );
  }

  // Enforce layout
  void CollapsableGroup::layout() {

    // Size self based on visible() of pack
    int cvis = _contents->visible();                                    // content visible?
    int ch   = cvis ? (parent()->h()-BUTTON_H-(GROUP_MARGIN*2)) : 0;    // content height
    int gh   = ch + BUTTON_H + GROUP_MARGIN*2;                          // group's height
    Fl_Group::resize(x(), y(), w(), gh);

    // Manage size/position of children
    //    No need to call init_sizes(): resizable(0), 
    //    and we child positions ourself.
    //
    _button->resize(x()+GROUP_MARGIN,                   // x always inset (leaves room for ROUND box)
                    y()+GROUP_MARGIN,                   // y always inset ("")
                    w()-(GROUP_MARGIN*2),               // width tracks group's w()
                    BUTTON_H);                          // height fixed

    _contents->resize(_button->x(),                     // x always same as button
                      y()+_button->y()+_button->h(),    // y always "below button"
                      _button->w(),                     // width tracks button
                      ch);                              // derived from parent's height

    /** DEBUG
    printf("-----------------\n");
    printf(" grp: %d,%d,%d,%d\n", x(),y(),w(),h());
    printf(" but: %d,%d,%d,%d\n", _button->x(), _button->y(), _button->w(), _button->h());
    printf("pack: %d,%d,%d,%d\n", _contents->x(), _contents->y(), _contents->w(), _contents->h());
    **/
  }

  void CollapsableGroup::toggle_tab_cb(Fl_Button* w, void *data) {
    mrv::CollapsableGroup* g = (mrv::CollapsableGroup*) data;
    g->toggle_tab(w);
  }

  // Collapse the widget or open it to show its contents
  void CollapsableGroup::toggle_tab( Fl_Button* b) {
    //DEBUG std::cerr << "toggle tab collapsable group" << std::endl; 
    if ( _contents->visible() )
      _contents->hide();
    else
      _contents->show();

    // Relabel button
    relabel_button();

    layout();   // layout changed
    window()->redraw();         // force redraw (_contents->hide()/show() doesn't)
  }

  // CTOR
  CollapsableGroup::CollapsableGroup( const int x, const int y, 
                                      const int w, const int h, 
                                      const char* l ) : Fl_Group( x, y, w, h, l ) {
    box( FL_ROUNDED_BOX );
    Fl_Group::begin();
    {
      // Button 
      _button = new Fl_Button(x+GROUP_MARGIN,           // margin leaves room for FL_ROUND_BOX
                              y+GROUP_MARGIN,           // margin leaves room for FL_ROUND_BOX
                              w-(GROUP_MARGIN*2),       // width same as group within margin
                              BUTTON_H);                // button height fixed size
      _button->align( FL_ALIGN_LEFT| FL_ALIGN_INSIDE );
      _button->labelsize( 16 );
      _button->box( FL_FLAT_BOX );
      _button->callback( (Fl_Callback*)toggle_tab_cb, this );

      _contents = new Fl_Pack(_button->x(),                     // lines up with button on x
                              y+_button->y()+_button->h(),      // just below button
                              w-(GROUP_MARGIN*2),               // width same as group within margin
                              10);                              // changes when child add()ed
      _contents->end();

      relabel_button();         // relabel button once pack created
    } 
    Fl_Group::end();
    resizable(0);               // prevent FLTK auto-sizing -- we handle children ourself
  }

  CollapsableGroup::~CollapsableGroup() {
    _contents->clear();
    Fl_Group::clear();  // delete _button and _contents
  }

  void CollapsableGroup::spacing( int x ) {
    _contents->spacing( x );
    redraw();
  }

  void CollapsableGroup::clear() {
    _contents->clear();
    redraw();
  }

  /** DEBUG
  // We don't really need this other than for debugging..
  void CollapsableGroup::draw() {
     fl_push_clip(x(), y(), w(), h());  // enforce clipping
     Fl_Group::draw();                  // let group draw itself and children
     fl_pop_clip();                     // enforce clipping

     fl_color(FL_RED);   fl_rect(x(), y(), w(), h());   // red line around group's xywh
     fl_color(FL_GREEN); fl_rect(_contents->x(),        // grn line around pack's xywh 
                                 _contents->y(),
                                 _contents->w(),
                                 _contents->h());
  }
 **/

  void CollapsableGroup::add( Fl_Widget* w ) {
    _contents->add( w );
    _contents->redraw();
  }

  void CollapsableGroup::resize(int X,int Y,int W,int H) {
    Fl_Group::resize(X,Y,W,H);  // let group resize
    layout();                   // let layout() handle child pos/sizes
  }

} // namespace mrv
