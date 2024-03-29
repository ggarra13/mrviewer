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
/**
 * @file   mrvGLSphere.cpp
 * @author gga
 * @date   Fri Feb  8 10:14:11 2008
 *
 * @brief  Handle and draw an OpenGL Sphere with shaders
 *
 *
 */



#if defined(WIN32) || defined(WIN64)
#  include <winsock2.h>  // to avoid winsock issues
#  include <windows.h>
#  undef min
#  undef max
#endif

#include <GL/glew.h>

#if defined(WIN32) || defined(WIN64)
#  include <GL/wglew.h>
#elif defined(LINUX)
#  include <GL/glxew.h>
#endif


#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"

#include "mrViewer.h"  // yuck
#include "mrvPreferencesUI.h"  // yuck

#include "mrvGLShader.h"
#include "mrvGLEngine.h"
#include "mrvGLSphere.h"
#include "mrvGLLut3d.h"

#include "ImathMatrix.h"

//#define TEST_NO_QUAD         // test not using textures
//#define TEST_NO_PBO_TEXTURES // test not using pbo textures
#define NVIDIA_PBO_BUG     // with pbo textures, my nvidia card has problems
// with GL_BGR formats and high resolutions


// PBO macro (see spec for details)
#define BUFFER_OFFSET(i) ((char *)NULL + (i))


namespace
{
const char* kModule = "glsphr";
}


namespace mrv {


GLSphere::GLSphere( const ImageView* view ) :
    GLQuad( view ),
    qObj( NULL ),
    _rotX( 0.0 ),
    _rotY( 0.0 )
{
    if (!qObj) qObj = gluNewQuadric();
    gluQuadricDrawStyle( qObj, GLU_FILL );
    gluQuadricOrientation( qObj, GLU_INSIDE );
    gluQuadricNormals( qObj, GLU_SMOOTH );
    gluQuadricTexture( qObj, GL_TRUE );
}

GLSphere::~GLSphere()
{
    gluDeleteQuadric( qObj );
}

void GLSphere::bind( const image_type_ptr pic )
{
    CHECK_GL;
    unsigned dw = pic->width();
    unsigned dh = pic->height();
    if ( ! pic || dw <= 0 || dh <= 0 ) {
        LOG_ERROR( _("Not a picture to be bound") );
        return;
    }

    _width = _height = 0;
    _uvMax.u = _uvMax.v = 1.0f;


    //
    // Use power of two textures if card does not support arbitrary resolutions
    //
    unsigned poww = dw, powh = dh;
    if ( GLEngine::pow2Textures() )
    {
        poww = calculate_pow2( dw );
        powh = calculate_pow2( dh );

        _uvMax.u  = (float) dw / (float) poww;
        _uvMax.v  = (float) dh / (float) powh;
    }


    if ( poww <= GLEngine::maxTexWidth() && powh <= GLEngine::maxTexHeight() )
    {
        bind_texture_quad( pic, poww, powh );
    }
}


void GLSphere::draw_sphere( const unsigned dw, const unsigned dh ) const
{
    glColor4f(1.0f,1.0f,1.0f,1.0f);

    // We use a linear texture filter always on the sphere
    GLenum filter = GL_LINEAR;

    if ( _shader && _shader != GLEngine::rgbaShader() )
    {
        short i = short(_channels - 1);
        for ( ; i >= 0 ; --i )
        {
            short idx = (i == 3 ? 4 : i );
            glActiveTexture(GL_TEXTURE0 + idx);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, _texId[i] );
            CHECK_GL;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        }
    }
    else
    {
        if ( GLEW_ARB_multitexture )
            glActiveTexture(GL_TEXTURE0);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, _texId[0] );
        CHECK_GL;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    }

    CHECK_GL;

    bool use_lut = _view->use_lut() && _lut && GLEW_EXT_texture3D;
    if ( use_lut && _lut )
    {
        _lut->enable();
    }


    if ( _shader )
    {
        _shader->enable();
        _shader->bind();

        if ( _shader != GLEngine::rgbaShader() )
        {
            _shader->setTextureUnit( "YImage", 0 );
            _shader->setTextureUnit( "UImage", 1 );
            _shader->setTextureUnit( "VImage", 2 );
        }
        else
        {
            _shader->setTextureUnit( "fgImage", 0 );
        }

        _shader->setTextureUnit( "lut", 3 );

        _shader->setUniform( "mask", (int) _mask );
        _shader->setUniform( "mask_value", (int) _mask_value );
        _shader->setUniform( "height", (int) _height );
        _shader->setUniform( "width", (int) _width );

        _shader->setUniform( "gain",  _view->gain() );
        _shader->setUniform( "gamma", 1.0f/_gamma );

        _shader->setUniform( "channel", _view->channel_type() );

        if ( use_lut && _lut )
        {
            _shader->setUniform( "enableLut", 1 );
            _shader->setUniform( "lutMin", _lut->lutMin );
            _shader->setUniform( "lutMax", _lut->lutMax );
            _shader->setUniform( "lutM", _lut->lutM );
            _shader->setUniform( "lutT", _lut->lutT );
            _shader->setUniform( "lutF", false );
        }
        else
        {
            _shader->setUniform( "enableLut", 0 );
        }

        const ViewerUI* v = _view->main();
        if (!v) return;

        int premult = 0, unpremult = 0;

        switch ( v->uiPrefs->uiPrefsBlendMode->value() )
        {
        case ImageView::kBlendPremultNonGamma:
            unpremult = premult = 1;
            break;
        case ImageView::kBlendTraditionalNonGamma:
            unpremult = premult = 1;
            break;
        default:
            unpremult = premult = 0;
            break;
        }

        _shader->setUniform( "premult", premult );
        if ( _shader == GLEngine::rgbaShader() )
            _shader->setUniform( "unpremult", unpremult );

        if ( _view->normalize() )
        {
            _shader->setUniform( "enableNormalization", 1 );
            _shader->setUniform( "normMin",  _normMin);
            _shader->setUniform( "normSpan", _normMax - _normMin );
        }
        else
        {
            _shader->setUniform( "enableNormalization", 0 );
        }

        if ( _shader == GLEngine::YByRyShader() ||
                _shader == GLEngine::YByRyAShader() )
        {
            _shader->setUniform( "yw", _yw[0], _yw[1], _yw[2] );
        }
        else if ( _shader == GLEngine::YCbCrShader() ||
                  _shader == GLEngine::YCbCrAShader() )
        {
            std::string colorspace = _("Unspecified");
            if ( _image && _image->colorspace() )
                colorspace = _image->colorspace();

            if ( colorspace == "BT709" )
            {
                _shader->setUniform( "coeffs", 1 );
                // HDTV  YCbCr coefficients
                _shader->setUniform( "Koff", 0.0f, -0.5f, -0.5f );
                _shader->setUniform( "Kr", 1.0f, 0.0f, 1.28033f );
                _shader->setUniform( "Kg", 1.0f, -0.21482f, -0.38059f );
                _shader->setUniform( "Kb", 1.0f, 2.12798f, 0.0f );
            }
            else if ( colorspace == "BT470BG" ||
                      colorspace == "SMPTE170M" )
            {
                _shader->setUniform( "coeffs", 1 );
                // STV  YCbCr coefficients
                _shader->setUniform( "Koff", -16.0f/255.0f, -0.5f, -0.5f );
                _shader->setUniform( "Kr", 1.0f, 0.0f, 1.59602715f );
                _shader->setUniform( "Kg", 1.0f, -0.39465f, -0.58060f );
                _shader->setUniform( "Kb", 1.0f, 2.03211f, 0.0f );
            }
            else if ( colorspace == "YCOCG" )
            {
                _shader->setUniform( "coeffs", 1 );
                _shader->setUniform( "Koff", 0.f, 0.f, 0.f );
                _shader->setUniform( "Kr", 1.0f, -1.0f, 1.0f );  // Y
                _shader->setUniform( "Kg", 1.0f,  1.0f, 0.0f );  // Cg
                _shader->setUniform( "Kb", 1.0f, -1.0f, -1.0f ); // Co
            }
            else
            {
                _shader->setUniform( "coeffs", 0 );
            }
        }

        CHECK_GL;
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    {
        mrv::media fg = _view->foreground();
        if ( fg )
        {
            CMedia* img = fg->image();
            float x = 1.0f, y = 1.0f;
            if ( img->flipX() )   x = -1.0f;
            if ( img->flipY() ) y = -1.0f;
            glScalef( x, y, 1.0f );
        }
    }

    glRotated( _rotX, 1, 0, 0 );
    glRotated( _rotY, 0, 1, 0 );
    glRotated( 90.0, 1, 0, 0 );


    gluSphere( qObj, 1.0f, 30, 30 );

    if ( _shader )
    {
        _shader->disable();
    }

    if ( _shader && _shader != GLEngine::rgbaShader() )
    {
        short i = short( _channels - 1 );
        for ( ; i >= 0 ; --i )
        {
            short idx = (i == 3 ? (short) 4 : i );
            glActiveTexture(GL_TEXTURE0 + idx);
            glDisable( GL_TEXTURE_2D );
        }
    }
    else
    {
        if ( GLEngine::maxTexUnits() > 3 )
            glActiveTexture( GL_TEXTURE0 );
        glDisable( GL_TEXTURE_2D );
    }

    if ( use_lut && _lut )
    {
        _lut->disable();
    }

    CHECK_GL;
}


void GLSphere::draw( const unsigned dw, const unsigned dh ) const
{
    draw_sphere( dw, dh );
}

}
