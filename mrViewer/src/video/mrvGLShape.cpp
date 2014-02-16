


#if defined(WIN32) || defined(WIN64)
#  include <winsock2.h>  // to avoid winsock issues
#  include <windows.h>
#endif

#include <GL/glew.h>


#if defined(WIN32) || defined(WIN64)
#  include <GL/wglew.h>
#elif defined(LINUX)
#  include <GL/glxew.h>
#endif

#include <GL/glut.h>
// #include <GL/gl.h>


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

void GLTextShape::init()
{
  unsigned numChars = 255;

#ifdef WIN32
    HDC   hDC = fltk::getDC();
    HGLRC hRC = wglGetCurrentContext();
    if (hRC == NULL ) hRC = wglCreateContext( hDC );

    LOGFONT     lf;
    memset(&lf,0,sizeof(LOGFONT));
    lf.lfHeight               =   -_fontsize ;
    lf.lfWeight               =   FW_NORMAL ;
    lf.lfCharSet              =   ANSI_CHARSET ;
    lf.lfOutPrecision         =   OUT_RASTER_PRECIS ;
    lf.lfClipPrecision        =   CLIP_DEFAULT_PRECIS ;
    lf.lfQuality              =   DRAFT_QUALITY ;
    lf.lfPitchAndFamily       =   FF_DONTCARE|DEFAULT_PITCH;
    lstrcpy (lf.lfFaceName, _font.c_str() ) ;


    HFONT    fid = CreateFontIndirect(&lf);
    HFONT oldFid = (HFONT)SelectObject(hDC, fid);

    _charset = glGenLists( numChars );

    wglMakeCurrent( hDC, hRC );
    wglUseFontBitmaps(hDC, 0, numChars-1, sCharset);

    SelectObject(hDC, oldFid);
#else
    // Find Window's default font
    Display* gdc = fltk::xdisplay;

    // Load XFont to user's specs
    char font_name[256];
    sprintf( font_name, N_("-*-%s-*-r-normal--%d-0-0-0-c-0-iso8859-1"),
	     _font.c_str(), _fontsize );
    XFontStruct* hfont = XLoadQueryFont( gdc, font_name );
    if (!hfont) {
       LOG_ERROR( _("Could not open ") << _font << _(" of size ") << _fontsize);
       return;
    }

    // Create GL lists out of XFont
    _charset = glGenLists( numChars );
    glXUseXFont(hfont->fid, 0, numChars-1, _charset);

    // Free font and struct
    XFreeFont( gdc, hfont );
#endif
}

void GLTextShape::draw()
{
  if ( !_charset )
  {
     init();
  }

  glLoadIdentity();
  glRasterPos2s( p.x, p.y );

  glPushAttrib(GL_LIST_BIT);

  glListBase( _charset );
  glCallLists( GLsizei( _text.size() ), GL_UNSIGNED_BYTE, 
               _text.c_str() );

  glPopAttrib();
}


} // namespace mrv
