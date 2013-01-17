
#if defined(WIN32) || defined(WIN64)
#  include <winsock2.h>  // to avoid winsock issues
#  include <windows.h>
#endif

#include <GL/gl.h>

#include <fltk/draw.h>

#include "gui/mrvImageView.h"
#include "video/mrvGLShape.h"

namespace mrv {

void GLPathShape::draw()
{
   //Turn on Color Buffer and Depth Buffer
   glColorMask(true, true, true, true);

   //Only write to the Stencil Buffer where 1 is not set
   glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
   //Keep the content of the Stencil Buffer
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

   glColor4f( r, g, b, a );


   glBegin( GL_BLEND );

   // So compositing works properly
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glLineWidth( pen_size );
   glEnable( GL_LINE_SMOOTH );

   PointList::const_iterator i = pts.begin();
   PointList::const_iterator e = pts.end();

   if ( pts.size() > 1 )
   {
      glBegin( GL_LINE_STRIP );

      for ( ; i != e; ++i )
      {
	 const mrv::Point& p = *i;
	 glVertex2d( p.x, p.y );
      }
      
      glEnd();
   }

   glPointSize( pen_size );
   glEnable( GL_POINT_SMOOTH );
   glBegin( GL_POINTS );

   i = pts.begin();
   for ( ; i != e; ++i )
   {
      const mrv::Point& p = *i;
      glVertex2d( p.x, p.y );
   }

   glEnd();

}


void GLPathShape::send( mrv::ImageView* v )
{
   std::string str = "GLPathShape ";
   	 
   PointList::iterator i = pts.begin();
   PointList::iterator e = pts.end();

   std::ostringstream stream;
   for ( ; i != e; ++i )
   {
      stream << (*i) << " ";
   }
   stream << std::endl;
   
   str += stream.str();

   v->send( str );
}

void GLErasePathShape::draw()
{
   glColorMask(false, false, false, false);

   //Set 1 into the stencil buffer
   glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
   glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

   glLineWidth( pen_size );

   glBegin( GL_LINE_STRIP );

   PointList::const_iterator i = pts.begin();
   PointList::const_iterator e = pts.end();
   for ( ; i != e; ++i )
   {
      const mrv::Point& p = *i;
      glVertex2d( p.x, p.y );
   }

   glEnd();

}


} // namespace mrv
