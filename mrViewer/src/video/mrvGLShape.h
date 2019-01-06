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

#include <float.h>
#include <limits.h>
#include <vector>
#include <iostream>

#include "core/mrvPacketQueue.h" // For MRV_NOPTS_VALUE

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

    inline Point operator-() const
    {
        return Point( -x, -y );
    }

    inline Point operator*( const double t ) const
    {
        return Point( x*t, y*t );
    }

    inline Point	operator+(const Point &v) const
    {
        return Point( x + v.x, y + v.y );
    }

    inline Point	operator-(const Point &v) const
    {
        return Point( x - v.x, y - v.y );
    }

    inline double		dot (const Point &v) const
    {
        return x * v.x + y * v.y;
    }

    double lengthTiny () const
    {
        double absX = (x >= double(0))? x: -x;
        double absY = (y >= double(0))? y: -y;

        double max = absX;

        if (max < absY)
            max = absY;

        if (max == double(0))
            return double(0);

        //
        // Do not replace the divisions by max with multiplications by 1/max.
        // Computing 1/max can overflow but the divisions below will always
        // produce results less than or equal to 1.
        //

        absX /= max;
        absY /= max;

        return max * sqrt (absX * absX + absY * absY);
    }

    inline double length () const
    {
        double length2 = dot (*this);

        if (length2 < 2 * DBL_MIN )
            return lengthTiny();

        return sqrt (length2);
    }

    inline Point normalized() const
    {
        double l = length();
        if ( l == 0 )
            return Point();

        return Point( x / l, y / l );
    }

    double x, y;
};

inline std::ostream& operator<<( std::ostream& o, const Point& p )
{
    return o << p.x << "," << p.y;
}

void glCircle( const Point& p, const double radius );

class GLShape
{
public:
    GLShape() : r(0.0), g(1.0), b(0.0), a(1.0), pen_size(5),
        //  previous( 5 ), next( 5 ),
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
    //short previous, next;
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

    void position( int x, int y ) {
        pts[0].x = x;
        pts[0].y = y;
    }

    inline void text( std::string t ) {
        _text = t;
    }
    inline std::string text() const   {
        return _text;
    }

    inline void font( fltk::Font* f ) {
        _font = f;
    }
    inline fltk::Font* font() const   {
        return _font;
    }

    inline void size( unsigned f ) {
        _fontsize = f;
    }
    inline unsigned size() const   {
        return _fontsize;
    }

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
