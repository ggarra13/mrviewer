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



#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#if defined(_WIN32)
#  include <winsock2.h>  // to avoid winsock issues
#  include <windows.h>
#endif

#include <GL/glew.h>


#if defined(_WIN32)
#  include <FL/platform.H>
#elif defined(LINUX)
#  include <GL/glxew.h>
#endif

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/fl_draw.H>

#include "core/mrvColor.h"

#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
#include "video/mrvGLEngine.h"
#include "video/mrvGLShape.h"

#include "video/Polyline2D.h"

namespace {
const char* kModule = N_("shape");
}




namespace mrv {


void glDisk( const Point& p, const float diameter )
{
    const float radius = diameter / 2.0f;
    const GLint triangleAmount = 20;
    const GLdouble twoPi = M_PI * 2.0;

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

 void GLCircleShape::glCircle( const Point& p, const double radius,
                               double pen_size )
{
    const GLint triangleAmount = 40;
    const GLdouble twoPi = M_PI * 2.0;

    PointList verts;
    verts.reserve( triangleAmount+1 );
    for ( int i = 0; i < triangleAmount; ++i )
    {
        Point pt( p.x + (radius * cos( i* twoPi / triangleAmount )),
                  p.y + (radius * sin( i* twoPi / triangleAmount )) );
        verts.push_back( pt );
    }

    Point pt( p.x + radius, p.y );
    verts.push_back( pt );

    const PointList& draw =
        Polyline2D::create( verts, pen_size,
                            Polyline2D::JointStyle::MITER,
                            Polyline2D::EndCapStyle::ROUND,
                            false
            );

    glEnableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_EDGE_FLAG_ARRAY );
    glDisableClientState( GL_FOG_COORD_ARRAY );
    glDisableClientState( GL_INDEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glVertexPointer(2, GL_DOUBLE, 0, &draw[0]);
    glDrawArrays( GL_TRIANGLES, 0, draw.size() );


    glDisableClientState(GL_VERTEX_ARRAY);
}




std::string GLPathShape::send() const
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

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

    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );

    return buf;
}

void GLPathShape::draw( double z, double m )
{
    //Turn on Color Buffer
    glColorMask(true, true, true, true);

    //Only write to the Stencil Buffer where 1 is not set
    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
    //Keep the content of the Stencil Buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);



    glEnable( GL_BLEND );
    // So compositing works properly
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f( r, g, b, a );

    size_t num = pts.size();
    if ( num < 1 )
    {
        return;
    }


    const PointList& draw =
        Polyline2D::create( pts, pen_size,
                            Polyline2D::JointStyle::ROUND,
                            Polyline2D::EndCapStyle::ROUND,
                            false
            );

    glEnableClientState( GL_VERTEX_ARRAY );

    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_EDGE_FLAG_ARRAY );
    glDisableClientState( GL_FOG_COORD_ARRAY );
    glDisableClientState( GL_INDEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glVertexPointer(2, GL_DOUBLE, 0, &draw[0]);
    glDrawArrays( GL_TRIANGLES, 0, draw.size() );

    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable( GL_BLEND );
}

void GLArrowShape::draw( double z, double m )
{
    //Turn on Color Buffer
    glColorMask(true, true, true, true);

    //Only write to the Stencil Buffer where 1 is not set
    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
    //Keep the content of the Stencil Buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);



    glEnable( GL_BLEND );
    // So compositing works properly
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f( r, g, b, a );

    const PointList& draw =
        Polyline2D::create( pts, pen_size,
                            Polyline2D::JointStyle::ROUND,
                            Polyline2D::EndCapStyle::ROUND,
                            false
            );

    glEnableClientState( GL_VERTEX_ARRAY );

    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_EDGE_FLAG_ARRAY );
    glDisableClientState( GL_FOG_COORD_ARRAY );
    glDisableClientState( GL_INDEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glVertexPointer(2, GL_DOUBLE, 0, &draw[0]);
    glDrawArrays( GL_TRIANGLES, 0, draw.size() );

    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable( GL_BLEND );

}

std::string GLArrowShape::send() const
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    std::string buf = "GLArrowShape ";
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
    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );

    return buf;
}

void GLRectangleShape::draw( double z, double m )
{

    //Turn on Color Buffer
    glColorMask(true, true, true, true);

    //Only write to the Stencil Buffer where 1 is not set
    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
    //Keep the content of the Stencil Buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);



    glEnable( GL_BLEND );
    // So compositing works properly
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f( r, g, b, a );

    PointList verts;
    verts.resize(5);
    verts[0] = Point( pts[0].x, pts[0].y );
    verts[1] = Point( pts[1].x, pts[0].y );
    verts[2] = Point( pts[1].x, pts[1].y );
    verts[3] = Point( pts[0].x, pts[1].y );
    verts[4] = Point( pts[0].x, pts[0].y );

    const PointList& draw =
        Polyline2D::create( verts, pen_size,
                            Polyline2D::JointStyle::ROUND,
                            Polyline2D::EndCapStyle::ROUND,
                            false
            );

    glEnableClientState( GL_VERTEX_ARRAY );

    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_EDGE_FLAG_ARRAY );
    glDisableClientState( GL_FOG_COORD_ARRAY );
    glDisableClientState( GL_INDEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glVertexPointer(2, GL_DOUBLE, 0, &draw[0]);
    glDrawArrays( GL_TRIANGLES, 0, draw.size() );

    glDisableClientState(GL_VERTEX_ARRAY);


    glDisable( GL_BLEND );

}

std::string GLRectangleShape::send() const
{

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    std::string buf = "GLRectangleShape ";
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
    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );

    return buf;
}


void GLCircleShape::draw( double z, double m )
{
    //Turn on Color Buffer
    glColorMask(true, true, true, true);

    //Only write to the Stencil Buffer where 1 is not set
    glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
    //Keep the content of the Stencil Buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);



    glEnable( GL_BLEND );
    // So compositing works properly
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f( r, g, b, a );

    glCircle( center, radius, pen_size );

    glDisable( GL_BLEND );
}


std::string GLCircleShape::send() const
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    std::string buf = "GLCircleShape ";
    char tmp[128];
    sprintf( tmp, "%g %g %g %g %g %" PRId64, r, g, b, a, pen_size, frame );

    buf += tmp;

    sprintf( tmp, " %g %g %g", center.x, center.y, radius );
    buf += tmp;

    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );

    return buf;
}


std::string GLErasePathShape::send() const
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

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

    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );

    return buf;
}




void GLErasePathShape::draw( double z, double m )
{
    glColorMask(false, false, false, false);

    //Set 1 into the stencil buffer
    glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    if ( pts.size() < 1 )
    {
        return;
    }

    const PointList& draw =
        Polyline2D::create( pts, pen_size,
                            Polyline2D::JointStyle::ROUND,
                            Polyline2D::EndCapStyle::ROUND,
                            false
            );

    glEnableClientState( GL_VERTEX_ARRAY );

    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_EDGE_FLAG_ARRAY );
    glDisableClientState( GL_FOG_COORD_ARRAY );
    glDisableClientState( GL_INDEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_SECONDARY_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glVertexPointer(2, GL_DOUBLE, 0, &draw[0]);
    glDrawArrays( GL_TRIANGLES, 0, draw.size() );

    glDisableClientState(GL_VERTEX_ARRAY);
}


GLTextShape::~GLTextShape()
{
}

std::string GLTextShape::send() const
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    std::string buf = "GLTextShape ";

    char tmp[512];
    sprintf( tmp, "\"%s\" ^%s^ %d %g %g %g %g %" PRId64,
             Fl::get_font_name( font() ),
             text().c_str(), size(), r, g, b, a, frame );
    buf += tmp;
    sprintf( tmp, " %g %g", pts[0].x, pts[0].y );
    buf += tmp;

    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );
    return buf;
}

void GLTextShape::draw( double z, double m )
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

    glActiveTexture( GL_TEXTURE0 );

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // So compositing works properly
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glColor4f( r, g, b, a );


    int textsize = int( size() * z );
    if ( textsize < 1 ) return;


    gl_font(font(), textsize );

    double height = (gl_height() / z);

    std::string txt = text();

    GLboolean result;
    std::size_t pos = txt.find('\n');
    double x = double( pts[0].x );
    double y = double( pts[0].y );
    for ( ; pos != std::string::npos; y -= height, pos = txt.find('\n') )
    {
        glRasterPos2d( x, y );
        glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
        if ( result == GL_FALSE )
        {
            double xMove = gl_width( txt.c_str(), pos ) / z;
            double yMove = height;
            double bxMove = -xMove * m * z;
            double byMove = -yMove * m * z;
            glRasterPos2d( x + xMove, y + yMove );
            glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
            if ( result == GL_FALSE )
            {
                // Probably bottom right corner, don't offset x.
                bxMove = 0;
                glRasterPos2d( x, y + yMove );
                glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
            }
            glBitmap( 0, 0, 0, 0, bxMove, byMove, NULL );
        }
        if ( result == GL_TRUE )
            gl_draw(txt.c_str(), pos );
        if ( txt.size() > pos )
            txt = txt.substr( pos+1, txt.size() );
    }
    if ( !txt.empty() )
    {
        glRasterPos2d( x, y );
        glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
        if ( result == GL_FALSE )
        {
            double xMove = gl_width( txt.c_str() ) / z;
            double yMove = height;
            double bxMove = -xMove * m * z;
            double byMove = -yMove * m * z;
            glRasterPos2d( x + xMove, y + yMove );
            glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
            if ( result == GL_FALSE )
            {
                // Probably bottom right corner, don't offset x.
                bxMove = 0;
                glRasterPos2d( x, y + yMove );
                glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &result);
            }
            glBitmap( 0, 0, 0, 0, bxMove, byMove, NULL );
        }
        if ( result == GL_TRUE )
            gl_draw( txt.c_str() );
    }

}


} // namespace mrv
