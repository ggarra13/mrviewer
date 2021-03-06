/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
 * @file   mrvTable.h
 * @author gga
 * @date   Wed Jan 31 14:26:28 2007
 *
 * @brief  A table that can contain widgets.
 *
 *
 */

#ifndef mrvTable_h
#define mrvTable_h

#include <FL/Fl_Table.H>


namespace mrv {

class Table : public Fl_Table
{
  public:
    Table( int x, int y, int w, int h, const char* l = 0 );
    virtual ~Table();

    virtual void draw_cell(TableContext context, int R=0, int C=0, 
                           int X=0, int Y=0, int W=0, int H=0);

    inline void column_labels( const char** h ) { headers = h; }
    // inline void column_widths( const int* w ) { widths = w; }   // REMOVED: use Fl_Table::col_width(int) to get, and col_width(int,int) to set
    
    inline void column_separator(bool t = true) { _column_separator = t; }
    inline void auto_resize( bool t = true ) { _auto_resize = t; }

    virtual int handle( int event );
    
    void add( Fl_Widget* w );
    void layout();
    
  protected:
    void DrawHeader(const char *s, int X, int Y, int W, int H);

    Fl_Color get_title_color()  { return FL_WHITE; }
    Fl_Color get_widget_color() { return FL_BLACK; }
    
    int line_height() const { return 24; }

    int X, Y;
    
    const char** headers;
    bool _column_separator;
    bool _auto_resize;
    
};

}


#endif
