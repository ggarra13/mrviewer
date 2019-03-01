/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
 * @file   mrvBrowser.cpp
 * @author gga
 * @date   Wed Jan 31 14:28:24 2007
 *
 * @brief
 *
 *
 */

#include <iostream>

#define assert0(x) if ( !(x) ) do { std::cerr << #x << " FAILED"; abort(); } while(0);

#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Item.H>
#include "mrvBrowser.h"


namespace mrv
{

Browser::Browser( int x, int y, int w, int h, const char* l ) :
Fl_Browser( x, y, w, h, l ),
_column_separator_color( Fl_Color(FL_BLACK) ),
_last_cursor( FL_CURSOR_DEFAULT ),
_column_separator( true ),
_dragging ( false ),
_auto_resize( false )
{
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    init_sizes();
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
}

// CHANGE CURSOR
//     Does nothing if cursor already set to value specified.
//
void Browser::change_cursor(Fl_Cursor newcursor) {
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    if ( newcursor != _last_cursor ) {
        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        fl_cursor(newcursor);
        _last_cursor = newcursor;
    }
}
// RETURN THE COLUMN MOUSE IS 'NEAR'
//     Returns -1 if none.
//
int Browser::which_col_near_mouse() {
    //     int X,Y,W,H;
    //     FL_Rectangle r( X, Y, W, H );
    //     box->inset(r);	// area inside browser's box()
    // EVENT NOT INSIDE BROWSER AREA? (eg. on a scrollbar)
    if ( ! Fl::event_inside(this) ) {
        return -1;
    }
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

    const int* widths = column_widths();
    if (!widths) return -1;

    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    int mousex = Fl::event_x() + xposition();
    int colx = 0;
    for ( int t=0; widths[t]; t++ ) {
        colx += widths[t];
        int diff = mousex - colx;
        // MOUSE 'NEAR' A COLUMN?
        //     Return column #
        //
        if ( diff >= -4 && diff <= 4 ) {
            return(t);
        }
    }
    return(-1);
}

// MANAGE EVENTS TO HANDLE COLUMN RESIZING
void Browser::set_column_start(int i, int w)
{
    int* cols = const_cast<int*>( column_widths() );
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    cols[i] = w;
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    column_widths( cols );
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    redraw();
}

// MANAGE EVENTS TO HANDLE COLUMN RESIZING
int Browser::handle(int e)
{
    // Not showing column separators? Use default Fl_Browser::handle() logic
    if ( ! column_separator() ) return(Fl_Browser::handle(e));
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    // Handle column resizing
    int ret = 0;
    switch ( e ) {
    case FL_ENTER: {
        ret = 1;
        break;
    }
    case FL_MOVE: {
        std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
        if ( which_col_near_mouse() >= 0 ) {
            change_cursor(FL_CURSOR_WE);
        } else {
            change_cursor(FL_CURSOR_DEFAULT);
        }
        ret = 1;
        break;
    }
    case FL_PUSH: {
        Fl::event_clicks(0);
        int whichcol = which_col_near_mouse();
        if ( whichcol >= 0 ) {
            std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
            // CLICKED ON RESIZER? START DRAGGING
            ret = 1;
            _dragging = true;
            _dragcol = whichcol + 1;
            change_cursor(FL_CURSOR_DEFAULT);
        }
        break;
    }
    case FL_DRAG: {
        if ( _dragging ) {
            // Sum up column widths to determine position
            int mousex = Fl::event_x() + xposition();
            int newwidth = mousex - x();

            if ( newwidth > 0 ) {
                std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                set_column_start( _dragcol, newwidth );
                std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
                //relayout();  // @TODO: fltk1.4
                redraw();
            }
        }
        break;
    }
    case FL_LEAVE:
    case FL_RELEASE: {
        _dragging = false;				// disable drag mode
        change_cursor(FL_CURSOR_DEFAULT);	// ensure normal cursor
        ret = 1;
        break;
    }
    }
    if ( _dragging ) return(1);	// dragging? don't pass event to FL_Browser
    return(Fl_Browser::handle(e) ? 1 : ret);
}

void Browser::layout()
{
    int nchildren = children();

    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

    if ( _auto_resize )
    {
        int hh = 24;
        int nchildren = children();
        for ( int i = 0; i < nchildren; ++i )
        {
            Fl_Widget* c = child(i);
            hh += c->h();
        }

        h( hh );

    }

    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

    // Fl_Browser::layout();

    const int* widths = column_widths();
    if (!widths) return;

    int num = 0;
    const int*  c;
    for ( c = widths; *c != 0; ++c, ++num )
    {
    }
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

    // If widget is set to resizable, resize all columns equally,
    // unless one column width is set to -1
    if ( resizable() == this )
    {
        int sum = 0;
        const int*  c;
        for ( c = widths; *c != 0; ++c )
        {
            if ( *c < 0 ) {
                sum = 0;
                break;
            }
            sum += *c;
        }

        if ( sum > 0 )
        {
            for ( c = widths; *c != 0; ++c )
            {
                int W = int( w() * ( (float) *c / (float) sum ) );
                int* t = (int*) c;
                *t = W;
            }
        }
    }


    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    for ( int i = 0; i < nchildren; ++i )
    {
        Fl_Widget* c = child(i);
        if (!c) continue;
        if ( ! c->as_group() ) continue;

        Fl_Group* g  = (Fl_Group*) c;

        int columns = g->children();
        int x = 0;
        assert0( columns <= num || widths[num-1] == 0 );
        for ( int j = 0; j < columns; ++j )
        {
            c = g->child(j);
            if (!c) continue;
            int W = widths[j];
            if ( W == -1 ) {
                W = w() - x;
            }
            if ( W == 0 ) break;
            c->resize( x, c->y(), W, c->h() );
            x += W;
        }
    }

}

void Browser::draw() {
    // DRAW BROWSER
    Fl_Browser::draw();
    if (!column_widths()) return;

    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

    if ( column_separator() ) {
        // DRAW COLUMN SEPARATORS
        int X = x() + Fl::box_dx(box());
        int Y = y() + Fl::box_dy(box());
        int W = w() - Fl::box_dw(box());
        int H = h() - Fl::box_dh(box());
        int colx = X - xposition();
        fl_color( column_separator_color() );
        const int* widths = column_widths();
        for ( int t=0; widths[t]; t++ ) {
            colx += widths[t];
            if ( colx > X && colx < (X+W) ) {
                fl_line(colx, Y, colx, Y+H-1);
            }
        }
    }
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
}

int Browser::absolute_item_index( Fl_Group* g )
{
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    int idx = 1;
    int num = g->children();
    for (int i = 0; i < num; ++i)
    {
        Fl_Widget* c = g->child(i);
        if ( c->as_group() )
            idx += absolute_item_index( (Fl_Group*) c );
        else
            ++idx;
    }
    return idx;
}

int Browser::absolute_item_index( bool& found,
                                  Fl_Widget* item,
                                  Fl_Widget* w )
{
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;

    if ( w == item  ) {
        found = true;
        return 0;
    }

    if ( !w->as_group() ) return 1;

    Fl_Group* g = (Fl_Group*) w;
    int idx = 0;
    int children = g->children();
    for ( int i = 0; i < children; ++i )
    {
        idx += absolute_item_index( found, item, w );
        if ( found ) break;
    }

    return idx;
}

int Browser::absolute_item_index()
{
    std::cerr << __FUNCTION__ << " " << __LINE__ << std::endl;
    int main_idx = value();
    if ( main_idx < 0 ) return main_idx;

    int idx = 0;
    for (int i = 0; i < main_idx; ++i)
    {
        Fl_Widget* c = child(i);

        if ( c->as_group() )
            idx += absolute_item_index( (Fl_Group*) c );
        else
            ++idx;
    }

    Fl_Widget* sel = (Fl_Widget*) item_at( value() );
    bool found = false;
    idx += absolute_item_index( found, sel, child(main_idx) );

    return idx;
}

} // namespace mrv
