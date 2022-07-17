/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
#include <cmath>
#include <vector>
#include <iostream>

#include <FL/fl_draw.H>

#include "core/mrvPacketQueue.h" // For MRV_NOPTS_VALUE

#include <boost/shared_ptr.hpp>

#include <ImathVec.h>

namespace mrv
{

class ImageView;

    class Point : public Imath::V2d
    {
    public:
        Point() : Imath::V2d()
        {
        }

        Point( double xx, double yy ) :
            Imath::V2d( xx, yy )
        {
        }

        Point( const Point& b ) :
            Imath::V2d( b.x, b.y )
        {
        }

        Point( const Imath::V2d& b ) :
            Imath::V2d( b.x, b.y )
        {
        }

        inline Point& operator=( const Imath::V2d& b )
        {
            x = b.x; y = b.y;
            return *this;
        }

        inline Point& operator=( const Point& b )
        {
            x = b.x; y = b.y;
            return *this;
        }

        inline double angle( const Point& b )
        {
            return std::acos( dot( b ) ) / (length() * b.length());
        }
};


void glDisk( const Point& p, const float diameter );

class GLShape
{
public:
    GLShape() : r(0.0), g(1.0), b(0.0), a(1.0), pen_size(5),
    //  previous( 5 ), next( 5 ),
    frame( MRV_NOPTS_VALUE )
    {
    };

    virtual ~GLShape() {};

    virtual std::string send() const = 0;
    virtual void draw( double z, double m ) = 0;

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
    typedef std::vector< Point > PointList;
};

class GLCircleShape : public GLShape
{
public:
    GLCircleShape() : GLShape(), radius(1.0)  {};
    virtual ~GLCircleShape() {};
    virtual void draw( double z, double m );
    virtual std::string send() const;

    void glCircle( const Point& p, const double radius, double pen_size );
    Point center;
    double radius;
};

class GLPathShape : public GLShape
{
public:

    GLPathShape() : GLShape()  {};
    virtual ~GLPathShape() {};
    virtual void draw( double z, double m );
    virtual std::string send() const;

    PointList pts;
};

class GLArrowShape : public GLPathShape
{
public:

    GLArrowShape() : GLPathShape()  {};
    virtual ~GLArrowShape() {};
    virtual void draw( double z, double m );
    virtual std::string send() const;
};

class GLRectangleShape : public GLPathShape
{
public:

    GLRectangleShape() : GLPathShape()  {};
    virtual ~GLRectangleShape() {};
    virtual void draw( double z, double m );
    virtual std::string send() const;
};

class GLErasePathShape : public GLPathShape
{
public:

    GLErasePathShape() : GLPathShape()  {};
    virtual ~GLErasePathShape() {};
    virtual void draw( double z, double m );
    virtual std::string send() const;
};

class GLTextShape : public GLPathShape
{
public:
    GLTextShape() :
    GLPathShape(),
    _font(0), _zoom(0), _fontsize(8), _charset(0)
    {};
    virtual ~GLTextShape();

    void position( double x, double y ) {
        pts[0].x = x;
        pts[0].y = y;
    }

    inline void text( std::string t ) {
        _text = t;
    }
    inline std::string text() const   {
        return _text;
    }

    inline void font( Fl_Font f ) {
        _font = f;
    }
    inline Fl_Font font() const   {
        return _font;
    }

    inline void size( unsigned f ) {
        _fontsize = f;
    }
    inline unsigned size() const   {
        return _fontsize;
    }

    virtual void draw( double z, double m );
    virtual std::string send() const;

protected:
    Fl_Font _font;
    std::string _text, _encoding;
    float    _zoom;
    int      _fontsize;
    unsigned _charset;   //!< display list for characters
};


typedef boost::shared_ptr< GLShape > shape_type_ptr;
typedef std::vector< shape_type_ptr > GLShapeList;

}


#endif // mrvGLShape_h
