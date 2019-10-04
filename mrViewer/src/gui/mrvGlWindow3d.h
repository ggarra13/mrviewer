/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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

#ifndef mrvGlWindow3d_h
#define mrvGlWindow3d_h

//----------------------------------------------------------------------------
//
//        class GlWindow3d -- reconstructs deep image in a 3D OpenGl window
//
//----------------------------------------------------------------------------

#include <boost/thread/recursive_mutex.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

#include <ImfRgba.h>
#include <ImfArray.h>
#include <ImfNamespace.h>


namespace mrv {

const float kFPS = 1.0f / 24.0f;

class GlWindow3d : public Fl_Gl_Window
{
public:
    typedef boost::recursive_mutex Mutex;

    GlWindow3d (int x, int y, int w, int h, const char *l = 0);
    GlWindow3d (int w, int h, const char* l = 0 );
    ~GlWindow3d ();

    void load_data( int zsize,
                    float* dataZ[],
                    unsigned int sampleCount[],
                    int dx, int dy, // data window
                    float zmin, float zmax,
                    float farPlane  // zfar plane in Deep 3D window
                  );

    void Perspective (double focal, double aspect,
                      double zNear, double zFar);
    void ReshapeViewport();
    void GlInit();
    void draw();
    int handle (int event);

protected:
    void init();

    float**                              _dataZ;
    unsigned int *                       _sampleCount;
    int                                  _dx;
    int                                  _dy;
    float                                _zmax;
    float                                _zmin;
    float                                _farPlane;

private:
    double      _zoom;
    double      _translateX;
    double      _translateY;
    double      _scaleZ;
    double      _fitTran;
    double      _fitScale;
    double      _elevation;     // for rotation
    double      _azimuth;       // for rotation
    int         _mouseX;
    int         _mouseY;
    int         _mouseStartX;
    int         _mouseStartY;
    int         _inverted;      // for rotation
    int         _displayFactor; // for display pixel samples

    Mutex       _mutex;  // mutex to avoid clearing data being drawn

};

} // namespace mrv

#endif  // mrvGlWindow3d_h
