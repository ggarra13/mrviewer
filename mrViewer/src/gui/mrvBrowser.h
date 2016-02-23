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
 * @file   mrvBrowser.h
 * @author gga
 * @date   Wed Jan 31 14:26:28 2007
 * 
 * @brief  A browser that can contain widgets and collapse itself.
 * 
 * 
 */

#ifndef mrvBrowser_h
#define mrvBrowser_h

#include <FL/Fl_Tree.H>
// #include <FL/Fl_Cursor.H>
// #include <FL/Fl_Color.H>


namespace mrv
{

class Browser : public Fl_Tree
{
public:
  Browser( int x, int y, int w, int h, const char* l = 0 );

  int handle( int e );
  void layout();
  void draw();

    void column_labels( const char** labels );

    void add( const char* text, Fl_Tree_Item* item = 0 )
    {
        Fl_Tree::add( text, item );
    }

    void insert( Fl_Widget& w, int idx );
    void replace( int idx, Fl_Widget& w );

    void remove( int idx );

    void add( Fl_Widget* w );
    void add( Fl_Group* g );
    void value( int x );
    int value();

  bool column_separator() const    { return _column_separator; }
  void column_separator(bool t) { _column_separator = t; }

  void column_separator_color( Fl_Color c ) { _column_separator_color = c; }
  Fl_Color column_separator_color() const { 
    return _column_separator_color; 
  }

  //
  // Returns the absolute (selected) item index by adding all children.
    int absolute_item_index();

  void auto_resize( bool t ) { _auto_resize = t; }

    void column_headers( const char* c ) { _column_headers = c; }
    void column_widths( const int* w ) { _column_widths = (int*)w; }

    const char* column_headers() const { return _column_headers; }
    const int* column_widths() const { return _column_widths; }

protected:

  // int absolute_item_index( const Fl_Group* g );
  // int absolute_item_index( bool& found, 
  //       		   const Fl_Widget* item,
  //       		   const Fl_Widget* child );

  void change_cursor( Fl_Cursor cursor );
  int which_col_near_mouse();

protected:
    int       _value;
    int*      _column_widths;
    const char* _column_headers;
  Fl_Color  _column_separator_color;	// color of column separator lines 
  Fl_Cursor _last_cursor;	// saved cursor state info
  int       _dragcol;		// col# user is currently dragging
  bool      _column_separator;	// flag to enable drawing column separators
  bool      _dragging;	// true if user dragging a column
  bool      _auto_resize;
};

} // namespace mrv

#endif // mrvBrowser_h
