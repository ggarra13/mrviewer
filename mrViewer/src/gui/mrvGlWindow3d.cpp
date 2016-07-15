/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2016  Gonzalo Garramuño

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
///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////



//----------------------------------------------------------------------------
//
//        class GlWindow3d -- reconstructs deep image in a 3D OpenGl window
//
//----------------------------------------------------------------------------
#include <cassert>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include <fltk/Cursor.h>
#include <fltk/events.h>
#include <fltk/draw.h>


#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <algorithm>

#include "mrvThread.h"

#include "gui/mrvGlWindow3d.h"
#include "gui/mrvHotkey.h"

using std::max;
using std::min;
using std::cout;
using std::endl;
using std::cerr;

namespace mrv {


GlWindow3d::GlWindow3d (int x,int y, int w,int h, const char *l ) 
: fltk::GlWindow (x,y,w,h,l)
{
    init();
}

GlWindow3d::GlWindow3d (int w,int h, const char *l ) :
fltk::GlWindow( w, h, l )
{
    init();
}

void
GlWindow3d::init()
{
    _dataZ = NULL;
    _sampleCount = NULL;
    _dx = 100;
    _dy = 80;
    _zmax = 100.0f;
    _zmin = 1.0f;
    _farPlane = 20.0f;
    _fitTran = -(_zmax + _zmin) / 2.0;
    _fitScale = 1.0;

    if (_zmax != _zmin)
        _fitScale = 1.0 / (_zmax - _zmin);
}

void
GlWindow3d::load_data( int zsize,
                       float* dataZ[],
                       unsigned int sampleCount[],
                       int dx, int dy,
                       float zmin, float zmax,
                       float farPlane )
{
    SCOPED_LOCK( _mutex );
    if ( zsize != dx * dy) {
        _dataZ = NULL;
        _sampleCount = NULL;
        return;
    }

    _dataZ = dataZ;

    assert( dataZ != NULL );

    _sampleCount = sampleCount;

    assert( sampleCount != NULL );


    _dx = dx;
    _dy = dy;
    _zmax = zmax;
    _zmin = zmin;
    _farPlane = farPlane;

    _fitTran = -(_zmax + _zmin) / 2.0;
    _fitScale = 1.0;

    if (_zmax != _zmin)
        _fitScale = 1.0 / (_zmax - _zmin);

}

GlWindow3d::~GlWindow3d ()
{
    // if ( _dataZ )
    // {
    //     for (int y = 0; y < _dy; y++)
    //     {
    //         for (int x = 0; x < _dx; x++)
    //         {
    //             delete [] _dataZ[y * _dx + x];
    //         }
    //     }
    //     delete [] _dataZ;
    //     _dataZ = NULL;
    // }

    // if ( _sampleCount )
    // {
    //     delete [] _sampleCount;
    //     _sampleCount = NULL;
    // }
}

void
GlWindow3d::Perspective (double focal, double aspect,
                         double zNear, double zFar)
{
    GLdouble xmin, xmax, ymin, ymax;
    ymax = zNear * tan (focal * M_PI / 360.0);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;
    glFrustum (xmin, xmax, ymin, ymax, zNear, zFar);
}

void
GlWindow3d::ReshapeViewport()
{
    glViewport (0, 0, w(), h());

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    GLfloat ratio = float(w()) / float(h());
    Perspective(min (max (30.0 + _zoom, 1.0), 179.0),
                1.0 * ratio, 1.0, _farPlane);
    glTranslatef(0.0, 0.0, -8.0);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}

void
GlWindow3d::GlInit()
{
    glShadeModel (GL_FLAT);

    _zoom = 0;
    _translateX = 0;
    _translateY = 0;
    _scaleZ = 1.0;
    _elevation = 0;
    _azimuth = 0;
    _inverted = 0;
    _displayFactor = 1;
}

void
drawRefPlan()
{
    glBegin (GL_LINES);
    glColor3f (0.6f, 0.6f, 0.6f);
    for (int i = 0; i <= 10; ++i)
    {
        glVertex3f (1.0f - 0.2f * float(i), 0.0f, 1.0f);
        glVertex3f (1.0f - 0.2f * float(i), 0.0f, -1.0f);
        glVertex3f (1.0f, 0.0f, 1.0f - 0.2f * float(i));
        glVertex3f (-1.0f, 0.0f, 1.0f - 0.2f * float(i));
    }
    glEnd();

    glBegin (GL_LINES);
    glColor3f (0.3f, 0.3f, 0.3f);
    glVertex3f (0.0, 0.0, 1.0);
    glVertex3f (0.0, 0.0, -1.0);
    glVertex3f (1.0, 0.0, 0.0);
    glVertex3f (-1.0, 0.0, 0.0);
    glEnd();
}

void
drawCoord()
{
    glBegin (GL_LINES);
    glColor3f (0.0f, 0.0f, 1.0f);
    glVertex3f (-1.0f, 0.0f, 1.0f);
    glVertex3f (-1.0f, 0.0f, 0.8f);

    glColor3f (1.0f, 0.0f, 0.0f);
    glVertex3f (-1.0f, 0.0f, 1.0f);
    glVertex3f (-0.8f, 0.0f, 1.0f);

    glColor3f (0.0f, 1.0f, 0.0f);
    glVertex3f (-1.0f, 0.0f, 1.0f);
    glVertex3f (-1.0f, 0.2f, 1.0f);
    glEnd();

    glPointSize(6);
    glBegin (GL_POINTS);
    glColor3f (1.0f, 1.0f, 0.0f);
    glVertex3f (-1.0f, 0.0f, 1.0f);

    glColor3f (0.0f, 0.0f, 1.0f);
    glVertex3f (-1.0f, 0.0f, 0.8f);

    glColor3f (1.0f, 0.0f, 0.0f);
    glVertex3f (-0.8f, 0.0f, 1.0f);

    glColor3f (0.0f, 1.0f, 0.0f);
    glVertex3f (-1.0f, 0.2f, 1.0f);
    glEnd();
}

void
drawOutLine(float dx, float dy, float z)
{
    glBegin (GL_LINE_LOOP);
    glColor3f (0.6f, 0.0f, 0.6f);
    glVertex3f (0, 0, z);
    glVertex3f (0, dy, z);
    glVertex3f (dx, dy, z);
    glVertex3f (dx, 0, z);
    glEnd();
}

void
GlWindow3d::draw()
{
    if ( !valid() )
    {
        GlInit();
        valid (1);
    }
    ReshapeViewport();
    glClearColor (.5,.5,.5, 0.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslated (_translateX, 0.0, 0.0);
    glTranslated (0.0, -_translateY, 0.0);

    glRotated (_elevation, 1.0, 0.0, 0.0);
    glRotated (_azimuth, 0.0, 1.0, 0.0);

    // draw the reference plane
    drawRefPlan();

    // draw the coordinate
    drawCoord();

    // scale z value
    glScaled (1.0, 1.0, _scaleZ);

    // display the objects
    // move the objects to the center of display
    glScalef (1.0f / float(_dx), 1.0f / float(_dx), 1.0f / 2.0f);
    glTranslatef (float(-_dx) / 2.0f, float(-_dy) / 2.0f, 0.0f);

    glScaled (1.0, 1.0, _fitScale);
    glTranslated (0.0, 0.0, -_fitTran);

    // loop dataZ to draw points
    glPointSize (2);

    glBegin (GL_POINTS);
    glColor3f (0.0, 1.0, 1.0);

    if ( _dataZ && _sampleCount )
    {
        SCOPED_LOCK( _mutex );
        for (int y = 0; y < _dy; y++)
        {
            for (int x = 0; x < _dx; x++)
            {
                if( x % _displayFactor == 0 && y % _displayFactor == 0)
                {
                    float* z = _dataZ[y * _dx + x];
                    unsigned int count = _sampleCount[y * _dx + x];

                    for (unsigned int i = 0; i < count; i++)
                    {
                        float val = z[i];
                        glVertex3f (float(x), float(_dy) - float(y) -1, -val);
                    }
                }
            }
        }
    }

    glEnd();

    // draw the display window OutLine
    drawOutLine (float(_dx), float(_dy), -(_zmax + _zmin) / 2.0f);

    // Check gl errors
    GLenum err = glGetError();
    if ( err != GL_NO_ERROR )
    {
        cerr << "GLGETERROR = " << (int)err << endl;
    }
}

int
GlWindow3d::handle (int event)
{
    if ( event == fltk::ENTER )
    {
        focus(this);
        return 1;
    }

    if (event == fltk::FOCUS ) {
        return 1;
    }

    if (fltk::event_button() == fltk::LeftButton )
    {
        switch (event)
        {
            case fltk::PUSH:
                _mouseStartX = fltk::event_x();
                _mouseStartY = fltk::event_y();

                if (fabs(_elevation) > 90.0)
                {
                    _inverted = 1;
                }
                else
                {
                    _inverted = 0;
                }
                return 1;
                break;
            case fltk::DRAG:
            case fltk::RELEASE:
            {
                int x = fltk::event_x();
                int y = fltk::event_y();

                if (_inverted)
                {
                    _azimuth -= (double)(x - _mouseStartX) * 0.2;
                }
                else
                {
                    _azimuth += (double)(x - _mouseStartX) * 0.2;
                }
                _elevation += (double)(y - _mouseStartY) * 0.2;

                while (_elevation < -180.0)
                    _elevation += 360.0;
                while (_elevation > 180.0)
                    _elevation -= 360.0;

                _mouseStartX = x;
                _mouseStartY = y;
                redraw();
                return 1;
                break;
            }
            default:
                break;
        }
    }

    if ( fltk::event_button() == fltk::MiddleButton )
    {
        switch (event)
        {
            case fltk::PUSH:
                fltk::cursor (fltk::CURSOR_MOVE);
                return 1;
                break;
            case fltk::RELEASE:
                fltk::cursor (fltk::CURSOR_DEFAULT);
                return 1;
                break;
            case fltk::DRAG:
                int x = fltk::event_x();
                int y = fltk::event_y();
                _translateX += (x - _mouseX) * 0.01;
                _translateY += (y - _mouseY) * 0.01;
                redraw();
                break;
        }
    }

    if ( event == fltk::MOUSEWHEEL )
    {
        float delta = (float) fltk::event_dy();
        _zoom += delta * 2.0f;
        redraw();
        return 1;
    }

    if ( fltk::event_button() == fltk::RightButton )
    {
        if ( event == fltk::PUSH ) return 1;

        if ( event == fltk::DRAG )
        {
            int x = fltk::event_x();
            int dx = x - _mouseX;
            int delta = -dx;
            _zoom += delta * 0.2;
            redraw();
        }
    }

    _mouseX = fltk::event_x();
    _mouseY = fltk::event_y();



    if (event == fltk::KEY)
    {
        unsigned rawkey = fltk::event_key();
        
        if ( kZDepthUp.match( rawkey ) ) //scale up
        {
            _scaleZ *= 1.2;
            redraw();
            return 1;
        }
        if ( kZDepthDown.match( rawkey ) ) //scale down
        {
            _scaleZ /= 1.2;
            redraw();
            return 1;
        }
        if ( kFitScreen.match(rawkey) ) //fit
        {
            GlInit();
            redraw();
            return 1;
        }
        if ( kDensityDown.match(rawkey) ) //decrease pixel samples
        {
            _displayFactor *= 2;
            if (_displayFactor > _dx || _displayFactor > _dy )
                _displayFactor /= 2;
            redraw();
            return 1;
        }
        if ( kDensityUp.match(rawkey) ) //increase pixel samples
        {
            _displayFactor /= 2;
            if (_displayFactor < 1)
                _displayFactor = 1;
            redraw();
            return 1;
        }
    }

    return fltk::GlWindow::handle( event );
}

} // namespace mrv
