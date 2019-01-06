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

// v0 and v1 are normalized
// t can vary between 0 and 1
// http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/
Point slerp2d( const Point& v0, const Point& v1, float t )
{
    float dot = v0.dot(v1);
    if( dot < -1.0f ) dot = -1.0f;
    if( dot > 1.0f ) dot = 1.0f;

    float theta_0 = acos( dot );
    float theta = theta_0 * t;

    Point v2( -v0.y, v0.x );

    return ( v0*cos(theta) + v2*sin(theta) );
}

void glCircle( const Point& p, const double radius )
{
    GLint triangleAmount = 20;
    GLdouble twoPi = M_PI * 2.0;

    glBegin( GL_TRIANGLE_FAN );
    glVertex2d( p.x, p.y );
    for ( int i = 0; i <= triangleAmount; ++i )
    {
        glVertex2d( p.x + (radius * cos( i* twoPi / triangleAmount )),
                    p.y + (radius * sin( i* twoPi / triangleAmount ))
                  );
    }
    glEnd();
}

void glPolyline( const vector<mrv::Point>& polyline, float width )
{
    if( polyline.size() < 2 ) return;
    float w = width / 2.0f;

    glBegin(GL_TRIANGLES);
    for( size_t i = 0; i < polyline.size()-1; ++i )
    {
        const Point& cur = polyline[ i ];
        const Point& nxt = polyline[i+1];

        Point b = (nxt - cur).normalized();
        Point b_perp( -b.y, b.x );

        Point p0( cur + b_perp*w );
        Point p1( cur - b_perp*w );
        Point p2( nxt + b_perp*w );
        Point p3( nxt - b_perp*w );

        // first triangle
        glVertex2dv( &p0.x );
        glVertex2dv( &p1.x );
        glVertex2dv( &p2.x );
        // second triangle
        glVertex2dv( &p2.x );
        glVertex2dv( &p1.x );
        glVertex2dv( &p3.x );

        // only do joins when we have a prv
        if( i == 0 ) continue;

        const Point& prv = polyline[i-1];
        Point a = (prv - cur).normalized();
        Point a_perp( a.y, -a.x );

        float det = a.x*b.y - b.x*a.y;
        if( det > 0 )
        {
            a_perp.x = -a_perp.x;
            a_perp.y = -a_perp.y;
            b_perp.x = -b_perp.x;
            b_perp.y = -b_perp.y;
        }

        // TODO: do inner miter calculation

        // flip around normals and calculate round join points
        a_perp.x = -a_perp.x;
        a_perp.y = -a_perp.y;
        b_perp.x = -b_perp.x;
        b_perp.y = -b_perp.y;

        size_t num_pts = 4;
        vector< Point > round( 1 + num_pts + 1 );
        for( size_t j = 0; j <= num_pts+1; ++j )
        {
            float t = (float)j/(float)(num_pts+1);
            if( det > 0 )
                round[j] = cur + (slerp2d( b_perp, a_perp, 1.0f-t ) * w);
            else
                round[j] = cur + (slerp2d( a_perp, b_perp, t ) * w);
        }

        for( size_t j = 0; j < round.size()-1; ++j )
        {
            glVertex2dv( &cur.x );
            if( det > 0 )
            {
                glVertex2dv( &(round[j+1].x) );
                glVertex2dv( &(round[j+0].x) );
            }
            else
            {
                glVertex2dv( &(round[j+0].x) );
                glVertex2dv( &(round[j+1].x) );
            }
        }
    }
    glEnd();
}


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


    glCircle( pts[0], pen_size / 2.0 );
    glPolyline( pts, pen_size );
    glCircle( pts[pts.size()-1], pen_size / 2.0 );

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

    glCircle( pts[0], pen_size / 2.0 );
    glPolyline( pts, pen_size );
    glCircle( pts[pts.size()-1], pen_size / 2.0 );
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

    glActiveTexture( GL_TEXTURE0 );

    glColor4f( r, g, b, a );

    fltk::glsetfont(font(), size()*float(z) );


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
        glRasterPos2d( pts[0].x, GLdouble(y) );
        fltk::gldrawtext(txt.c_str());
    }
}


} // namespace mrv
