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



#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#if defined(WIN32) || defined(WIN64)
#  include <winsock2.h>  // to avoid winsock issues
#  include <windows.h>
#endif

#include <GL/glew.h>


#if defined(WIN32) || defined(WIN64)
#  include <fltk/win32.h>
#  include <GL/wglew.h>
#elif defined(LINUX)
#  include <GL/glxew.h>
#endif

#include <GL/glut.h>
// #include <GL/gl.h>

#  define CHECK_GL(x) GLEngine::handle_gl_errors( N_( x ) )

#include <fltk/gl.h>
#include <fltk/Font.h>

#include <fltk/draw.h>

#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
#include "video/mrvGLEngine.h"
#include "video/mrvGLShape.h"

namespace {
const char* kModule = N_("shape");
}


namespace fltk {

#ifdef WIN32
  extern HINSTANCE	xdisplay;
#else
  extern Display*	xdisplay;
#endif

}


namespace mrv {

std::string GLPathShape::send() const
{
   std::string buf = "GLPathShape ";
   char tmp[256];
   sprintf( tmp, "%g %g %g %g %g %" PRId64, r, g, b, a,
            pen_size, frame );
   buf += tmp;
   GLPathShape::PointList::const_iterator i = pts.begin();
   GLPathShape::PointList::const_iterator e = pts.end();
   for ( ; i != e; ++i )
   {
      sprintf( tmp, " %g %g", (*i).x, (*i).y );
      buf += tmp;
   }
   return buf;
}

void GLPathShape::draw( double z )
{
   //Turn on Color Buffer and Depth Buffer
   glColorMask(true, true, true, true);

   //Only write to the Stencil Buffer where 1 is not set
   glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
   //Keep the content of the Stencil Buffer
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);



   glEnable( GL_BLEND );
   // So compositing works properly
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glColor4f( r, g, b, a );


   glLineWidth( pen_size * float(z) );
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


   if ( pts.size() == 1 || a >= 0.95f )
   {
       glPointSize( pen_size * float(z) );
       glDisable( GL_POINT_SMOOTH );
       glBegin( GL_POINTS );

      i = pts.begin();
      for ( ; i != e; ++i )
      {
         const mrv::Point& p = *i;
         glVertex2d( p.x, p.y );
      }

      glEnd();
   }

   glDisable( GL_BLEND );
}

std::string GLErasePathShape::send() const
{
   std::string buf = "GLErasePathShape ";
   char tmp[128];
   sprintf( tmp, "%g %" PRId64, pen_size, frame );

   buf += tmp;
   GLPathShape::PointList::const_iterator i = pts.begin();
   GLPathShape::PointList::const_iterator e = pts.end();
   for ( ; i != e; ++i )
   {
      sprintf( tmp, " %g %g", (*i).x, (*i).y );
      buf += tmp;
   }

   return buf;
}

void GLErasePathShape::draw( double z )
{
   glColorMask(false, false, false, false);

   //Set 1 into the stencil buffer
   glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
   glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

   glLineWidth( pen_size * float(z) );

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


GLTextShape::~GLTextShape()
{
}

std::string GLTextShape::send() const
{
   std::string buf = "GLTextShape ";

   fltk::Font* f = font();
   if (!f) return "";

   char tmp[512];
   sprintf( tmp, "\"%s\" ^%s^ %d %g %g %g %g %" PRId64, font()->name(),
            text().c_str(), size(), r, g, b, a, frame );
   buf += tmp;
   sprintf( tmp, " %g %g", pts[0].x, pts[0].y );
   buf += tmp;

   return buf;
}

void GLTextShape::draw( double z )
{
   //Turn on Color Buffer and Depth Buffer
   glColorMask(true, true, true, true);

   //Only write to the Stencil Buffer where 1 is not set
   glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);

   //Keep the content of the Stencil Buffer
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

   glDisable( GL_DEPTH_TEST );
   glDisable( GL_DITHER );
   glDisable( GL_LIGHTING );
   glEnable( GL_BLEND );

   // So compositing works properly
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glColor4f( r, g, b, a );

   if ( font() )
       fltk::glsetfont(font(), size()*float(z) );

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   std::string txt = text();
   std::size_t pos = txt.find('\n');
   double y = pts[0].y;
   for ( ; pos != std::string::npos; y -= size(), pos = txt.find('\n') )
   {
#if 0
       glRasterPos2i(0,0);
       glBitmap( 0, 0, 0.f, 0.f, GLfloat(pts[0].x), GLfloat(y), NULL );
#else
       glRasterPos2d( pts[0].x, y );
#endif
       std::string t;
       if (pos > 0 )
           t = txt.substr( 0, pos );
       fltk::gldrawtext(t.c_str());
       if ( txt.size() > pos+1 )
           txt = txt.substr( pos+1, txt.size() );
   }
   if ( txt.size() )
   {
       glRasterPos2d( pts[0].x, GLfloat(y) );
       fltk::gldrawtext(txt.c_str());
   }
}


} // namespace mrv
