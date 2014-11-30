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

#ifndef mrvGLShape_h
#define mrvGLShape_h

#include <vector>
#include <iostream>

#include "mrvPacketQueue.h" // For MRV_NOPTS_VALUE

#include <boost/shared_ptr.hpp>

namespace fltk
{
struct Font;
}

namespace mrv
{

class ImageView;

class Point
{
   public:
     Point() :
     x( 0 ), y( 0 )
     {
     }

     Point( double xx, double yy ) :
     x( xx ), y( yy )
     {
     }

     Point( const Point& b ) :
     x( b.x ), y( b.y )
     {
     }

     double x, y;
};

inline std::ostream& operator<<( std::ostream& o, const Point& p )
{
   return o << p.x << "," << p.y;
}

class GLShape
{
   public:
     GLShape() : r(0.0), g(1.0), b(0.0), a(1.0), pen_size(5),
		 frame( MRV_NOPTS_VALUE )
     {
     };

     ~GLShape() {};

     virtual std::string send() const = 0;
     virtual void draw( double z ) = 0;

     void color( float ri, float gi, float bi, float ai = 1.0 ) {
	r = ri;
	g = gi;
	b = bi;
	a = ai;
     }

   public:
     float r, g, b, a;
     float pen_size;
     boost::int64_t frame;
};

class GLPathShape : public GLShape
{
   public:

     GLPathShape() : GLShape()  {};
     ~GLPathShape() {};
     virtual void draw( double z );
     virtual std::string send() const;

     typedef std::vector< Point > PointList;
     PointList pts;
};

class GLErasePathShape : public GLPathShape
{
   public:

     GLErasePathShape() : GLPathShape()  {};
     ~GLErasePathShape() {};
     virtual void draw( double z );
     virtual std::string send() const;
};

class GLTextShape : public GLPathShape
{
   public:
     GLTextShape() : _font(NULL), _zoom(0), _fontsize(8), _charset(0), 
                     GLPathShape() {};
     ~GLTextShape();

     void position( int x, int y ) { pts[0].x = x; pts[0].y = y; }

     void text( std::string t ) { _text = t; }
     std::string text() const   { return _text; }

     void font( fltk::Font* f ) { _font = f; }
     fltk::Font* font() const   { return _font; }

     void size( unsigned f ) { _fontsize = f; }
     unsigned size() const   { return _fontsize; }

     virtual void draw( double z );
     virtual std::string send() const;

   protected:
     fltk::Font* _font;
     std::string _text, _encoding;
     float    _zoom;
     int      _fontsize;
     unsigned _charset;   //!< display list for characters
};


typedef boost::shared_ptr< GLShape > shape_type_ptr;
typedef std::vector< shape_type_ptr > GLShapeList;

}


#endif // mrvGLShape_h
