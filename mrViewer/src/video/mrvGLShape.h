
#ifndef mrvGLShape_h
#define mrvGLShape_h

#include <vector>
#include <iostream>

#include <boost/shared_ptr.hpp>


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

     Point( int xx, int yy ) :
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
     GLShape() : r(0.0), g(1.0), b(0.0), a(1.0), pen_size(5) {};
     ~GLShape() {};
     virtual void draw() = 0;
     void color( float ri, float gi, float bi, float ai = 1.0 ) {
	r = ri;
	g = gi;
	b = bi;
	a = ai;
     }
     float r, g, b, a;
     float pen_size;
};

class GLPathShape : public GLShape
{
   public:

     GLPathShape() : GLShape()  {};
     ~GLPathShape() {};
     virtual void draw();

     virtual void send( mrv::ImageView* v );

     typedef std::vector< Point > PointList;
     PointList pts;
};

class GLErasePathShape : public GLPathShape
{
   public:

     GLErasePathShape() : GLPathShape()  {};
     ~GLErasePathShape() {};
     virtual void draw();
};


typedef boost::shared_ptr< GLShape > shape_type_ptr;
typedef std::vector< shape_type_ptr > GLShapeList;

}


#endif // mrvGLShape_h
