


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

#include <fltk/gl.h>
#include <fltk/Font.h>

#include <fltk/draw.h>

#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
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

void GLPathShape::draw( float z )
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

   glDisable( GL_BLEND );

   if ( pts.size() == 1 || a >= 0.95f )
   {
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
}



void GLErasePathShape::draw( float z )
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

void GLTextShape::init()
{
  unsigned numChars = 256;

#ifdef _WIN32
    // HDC   hDC = fltk::getDC();

  HDC hDC = wglGetCurrentDC();

    // HGLRC hRC = wglGetCurrentContext();
    // if (hRC == NULL ) hRC = wglCreateContext( hDC );

    LOGFONT     lf;
    memset(&lf,0,sizeof(LOGFONT));
    lf.lfHeight               =   size() * _zoom;
    lf.lfWeight               =   FW_NORMAL ;
    lf.lfCharSet              =   ANSI_CHARSET ;
    lf.lfOutPrecision         =   OUT_RASTER_PRECIS ;
    lf.lfClipPrecision        =   CLIP_DEFAULT_PRECIS ;
    lf.lfQuality              =   DRAFT_QUALITY ;
    lf.lfPitchAndFamily       =   FF_DONTCARE|DEFAULT_PITCH;
    lstrcpy (lf.lfFaceName, font()->name() ) ;


    HFONT    fid = CreateFontIndirect(&lf);
    HFONT oldFid = (HFONT)SelectObject(hDC, fid);

    // wglMakeCurrent( hDC, hRC );

    _charset = glGenLists( numChars );


    bool succeeded = FALSE;
    const int MAX_TRIES = 5;
    for(int i=0; i<MAX_TRIES && !succeeded; ++i)
    {
       succeeded = wglUseFontBitmaps(hDC, 0, numChars-1, _charset ) != FALSE;
    }

    if (!succeeded)
    {
       fprintf( stderr, "wglUseFontBitmap error\n" );
    }

    // BOOL ok = wglUseFontBitmaps(hDC, 0, numChars-1, _charset);
    // if ( ok == FALSE )
    // {
    //    fprintf( stderr, "wglUseFontBitmap error\n" );
    // }

    SelectObject(hDC, oldFid);
#endif
}

GLTextShape::~GLTextShape()
{
}

void GLTextShape::draw( float z )
{

#ifdef _WIN32
   if ( !_charset || z != _zoom )
   {
      if ( _charset )
	 glDeleteLists( _charset, 256 );
      _zoom = z;
      init();
   }
#endif

   //Turn on Color Buffer and Depth Buffer
   glColorMask(true, true, true, true);

   //Only write to the Stencil Buffer where 1 is not set
   glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
   //Keep the content of the Stencil Buffer
   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

   glDisable( GL_DEPTH_TEST );
   glDisable( GL_DITHER );
   glDisable( GL_LIGHTING );
   glBegin( GL_BLEND );

   glColor4f( r, g, b, a );


#ifdef _WIN32
   // glRasterPos2d( pts[0].x, pts[0].y );

   glRasterPos2i(0,0);
   glBitmap( 0, 0, 0.f, 0.f, GLfloat(pts[0].x*z), GLfloat(pts[0].y*z), NULL );

   glListBase(_charset);
   glCallLists( text().size(), GL_UNSIGNED_BYTE, text().c_str() );
#else
   if ( font() )
      fltk::glsetfont(font(), size()*z );

   glRasterPos2i(0,0);
   glBitmap( 0, 0, 0.f, 0.f, GLfloat(pts[0].x*z), GLfloat(pts[0].y*z), NULL );

   fltk::gldrawtext(text().c_str());
#endif

}


} // namespace mrv
