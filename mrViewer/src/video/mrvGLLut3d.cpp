/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
 * @file   mrvGLLut3d.cpp
 * @author gga
 * @date   Sat Feb  9 14:13:57 2008
 *
 * @brief
 *
 *
 */

#include <fstream>
#include <limits>

#include <Iex.h>
#include <half.h>
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfStandardAttributes.h>
#include <ImfVecAttribute.h>

#include <FL/Enumerations.H>




#include "core/CMedia.h"
#include "gui/mrvIO.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvPreferences.h"

#include "mrViewer.h"
#include "mrvPreferencesUI.h"

#include "video/mrvGLLut3d.h"
#include "video/mrvGLEngine.h"

namespace
{
const char* kModule = N_("cmm");
}

#define OCIO_ERROR(x) do { \
    LOG_ERROR( "[ocio] " << x ); \
    ViewerUI::uiLog->uiMain->show(); \
    } while (0);

namespace mrv {


namespace {

inline void
indicesAndWeights (float r, int iMax, int &i, int &i1, float &u, float &u1)
{
    if (r >= 0)
    {
        if (r < iMax)
        {
            //
            // r is finite and in the interval [0, iMax]
            //

            i = int (r);
            i1 = i + 1;
            u  = r - (float) i;
        }
        else
        {
            //
            // r is greater than or equal to iMax
            //

            i = i1 = iMax;
            u = 1;
        }
    }
    else
    {
        //
        // r is either NaN or less than 0
        //

        i = i1 = 0;
        u = 1;
    }

    u1 = 1 - u;
}

} // namespace

using namespace Imath;

V3f
lookup3D
(const V3f table[],
 const int size,
 const V3f &p)
{
    int Max = size - 1;
    float r = (clamp (p.x, 0.0f, 1.0f) ) * (float) Max;

    int i, i1;
    float u, u1;
    indicesAndWeights (r, Max, i, i1, u, u1);

    float s = (clamp (p.y, 0.0f, 1.0f) ) * (float) Max;

    int j, j1;
    float v, v1;
    indicesAndWeights (s, Max, j, j1, v, v1);

    float t = (clamp (p.z, 0.0f, 1.0f) ) * (float) Max;

    int k, k1;
    float w, w1;
    indicesAndWeights (t, Max, k, k1, w, w1);

    const V3f &a = table[(k  * size + j ) * size + i ];
    const V3f &b = table[(k1 * size + j ) * size + i ];
    const V3f &c = table[(k  * size + j1) * size + i ];
    const V3f &d = table[(k1 * size + j1) * size + i ];
    const V3f &e = table[(k  * size + j ) * size + i1];
    const V3f &f = table[(k1 * size + j ) * size + i1];
    const V3f &g = table[(k  * size + j1) * size + i1];
    const V3f &h = table[(k1 * size + j1) * size + i1];

    V3f out3( w1 * (v1 * (u1 * a + u * b) + v * (u1 * c + u * d)) +
              w  * (v1 * (u1 * e + u * f) + v * (u1 * g + u * h)) );
    return out3;
}

V3f
lookup3D
(const V4f table[],
 const int size,
 const V3f &p)
{
    int Max = size - 1;
    float r = (clamp (p.x, 0.0f, 1.0f) ) * (float) Max;

    int i, i1;
    float u, u1;
    indicesAndWeights (r, Max, i, i1, u, u1);

    float s = (clamp (p.y, 0.0f, 1.0f) ) * (float) Max;

    int j, j1;
    float v, v1;
    indicesAndWeights (s, Max, j, j1, v, v1);

    float t = (clamp (p.z, 0.0f, 1.0f) ) * (float) Max;

    int k, k1;
    float w, w1;
    indicesAndWeights (t, Max, k, k1, w, w1);


    const V4f &a = table[(k  * size + j ) * size + i ];
    const V4f &b = table[(k1 * size + j ) * size + i ];
    const V4f &c = table[(k  * size + j1) * size + i ];
    const V4f &d = table[(k1 * size + j1) * size + i ];
    const V4f &e = table[(k  * size + j ) * size + i1];
    const V4f &f = table[(k1 * size + j ) * size + i1];
    const V4f &g = table[(k  * size + j1) * size + i1];
    const V4f &h = table[(k1 * size + j1) * size + i1];

    V4f out4( w1 * (v1 * (u1 * a + u * b) + v * (u1 * c + u * d)) +
              w  * (v1 * (u1 * e + u * f) + v * (u1 * g + u * h)) );
    return V3f( out4.x, out4.y, out4.z );
}

unsigned GLLut3d::NUM_STOPS = 8;
GLLut3d::LutsMap GLLut3d::_luts;


GLLut3d::GLLut3d( const ViewerUI* v, const unsigned N ) :
    lutMin( 0 ),
    lutMax( 0 ),
    lutM( 0 ),
    lutT( 0 ),
    lutF( 1 ),
    view( v ),
    texId( 0 ),
    _channels( 4 ),
    _lutN( N ),
    _inited( false )
{
    glGenTextures( 1, &texId );
}



GLLut3d::~GLLut3d()
{
    disable();
    glDeleteTextures( 1, &texId );
}



void GLLut3d::enable()
{
    if ( GLEngine::maxTexUnits() > 3 )
        glActiveTexture(GL_TEXTURE0 + 3);

    glBindTexture( GL_TEXTURE_3D, texId );
    glEnable( GL_TEXTURE_3D );
}



void GLLut3d::disable()
{
    if ( GLEngine::maxTexUnits() > 3 )
        glActiveTexture(GL_TEXTURE0 + 3);
    glDisable( GL_TEXTURE_3D );
}



//
// Initialize array of output pixel values to zero.
//
void GLLut3d::clear_lut()
{
    _inited = false;
    // @bug: CTL would hang or crash if lut_size is used.
    //       We pad it with 4 additional values and all seems fine.
    unsigned long num = lut_size() + 4;
    lut.resizeErase( num );
    memset( &lut[0], 0x00, num*sizeof(float) );
}



//
// Create opengl texture from the log of lut values
//
void GLLut3d::create_gl_texture()
{

    //
    // Take the logarithm of the output values that were
    // produced by the CTL transforms.
    //
    size_t num = lut_size();
    for ( size_t i = 0; i < num; ++i )
    {
        if (lut[i] >= HALF_MIN && lut[i] <= HALF_MAX)
        {
            //
            // lut[i] is finite and positive.
            //
            lut[i] = (float) logf(lut[i]);
        }
        else
        {
            //
            // lut[i] is zero, negative or not finite;
            // log (lut[i]) is undefined.
            //
            lut[i] = (float) logf( HALF_MIN );
        }
    }

    //
    // Convert the output values into a 3D texture.
    //
    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
    CHECK_GL;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    CHECK_GL;

    glActiveTexture( GL_TEXTURE3 );
    CHECK_GL;

    glBindTexture( GL_TEXTURE_3D, texId );
    CHECK_GL;

    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    CHECK_GL;
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    CHECK_GL;

    GLenum gl_clamp = GL_CLAMP;
    if ( GLEW_EXT_texture_edge_clamp )
        gl_clamp = GL_CLAMP_TO_EDGE;

    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, gl_clamp );
    CHECK_GL;
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, gl_clamp );
    CHECK_GL;
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, gl_clamp );
    CHECK_GL;


    if ( _channels == 3 )
    {
        GLenum internal = GL_RGB16F_ARB;
        if ( ! GLEngine::halfPixels() )
        {
            internal = GL_RGB32F_ARB;
            if ( ! GLEngine::floatTextures() ||
                 ! GLEngine::floatPixels() )
            {
                internal = GL_RGB;
            }
        }

        glTexImage3D( GL_TEXTURE_3D,
                      0,			// level
                      internal,           // internal format
                      _lutN, _lutN, _lutN,	// width, height, depth
                      0,			// border
                      GL_RGB,		// format
                      GL_FLOAT,	// type
                      (char *) &lut[0] );
    CHECK_GL;
    }
    else
    {
        GLenum internal = GL_RGBA16F_ARB;
        if ( ! GLEngine::halfPixels() )
        {
            internal = GL_RGBA32F_ARB;
            if ( ! GLEngine::floatTextures() ||
                 ! GLEngine::floatPixels() )
            {
                internal = GL_RGBA;
            }
        }

        glTexImage3D( GL_TEXTURE_3D,
                      0,			// level
                      internal,           // internal format
                      _lutN, _lutN, _lutN,	// width, height, depth
                      0,			// border
                      GL_RGBA,		// format
                      GL_FLOAT,	// type
                      (char *) &lut[0] );
    CHECK_GL;
    }
    FLUSH_GL_ERRORS;
}


void GLLut3d::evaluate( const Imath::V3f& rgb, Imath::V3f& out ) const
{
    using namespace Imath;
    float scale = ( (float) _lutN - 1.0f ) / (float) _lutN;
    float offset = 1.0f / ( 2.0f * _lutN );

    out.x = lutT + lutM * logf( Imath::clamp( rgb.x * scale + offset,
                                lutMin, lutMax ) );
    out.y = lutT + lutM * logf( Imath::clamp( rgb.y * scale + offset,
                                lutMin, lutMax ) );
    out.z = lutT + lutM * logf( Imath::clamp( rgb.z * scale + offset,
                                lutMin, lutMax ) );

    if (_channels == 3 )
        out = lookup3D( (V3f*)(&lut[0]), _lutN, out );
    else
        out = lookup3D( (V4f*)(&lut[0]), _lutN, out );

    out.x = expf( out.x );
    out.y = expf( out.y );
    out.z = expf( out.z );

}

void GLLut3d::calculate_range( const mrv::image_type_ptr& pic )
{
    // lutMin = std::numeric_limits<float>::max();
    // lutMax = std::numeric_limits<float>::min();

    // unsigned w = pic->width();
    // unsigned h = pic->height();

    // for ( unsigned y = 0; y < h; ++y )
    //     for ( unsigned x = 0; x < w; ++x )
    //     {
    //         const ImagePixel& p = pic->pixel( x, y );
    //         if ( p.r < lutMin && p.r > 0 ) lutMin = p.r;
    //         if ( p.g < lutMin && p.g > 0 ) lutMin = p.g;
    //         if ( p.b < lutMin && p.b > 0 ) lutMin = p.b;

    //         if ( p.r > lutMax ) lutMax = p.r;
    //         if ( p.g > lutMax ) lutMax = p.g;
    //         if ( p.b > lutMax ) lutMax = p.b;
    //     }
    //
    // std::cerr << "lutMin " << lutMin << " lutMax " << lutMax << std::endl;

}

void GLLut3d::init_pixel_values( Imf::Array< float >& pixelValues )
{
    //
    // Compute lutMin, lutMax, and scale and offset
    // values, lutM and lutT, so that
    //
    //	lutM * lutMin + lutT == 0
    //	lutM * lutMax + lutT == 1
    //


    static const float MIDDLE_GRAY = 0.18f;


    lutMin = MIDDLE_GRAY / (1 << NUM_STOPS);
    lutMax = MIDDLE_GRAY * (1 << NUM_STOPS);

    float logLutMin = logf (lutMin);
    float logLutMax = logf (lutMax);

    lutM = 1.0f / (logLutMax - logLutMin);
    lutT = -lutM * logLutMin;

    for (size_t ib = 0; ib < _lutN; ++ib)
    {
        float b = float(ib) / float(_lutN - 1.0);
        float B = expf((b - lutT) / lutM);

        for (size_t ig = 0; ig < _lutN; ++ig)
        {
            float g = float(ig) / float(_lutN - 1.0);
            float G = expf ((g - lutT) / lutM);

            for (size_t ir = 0; ir < _lutN; ++ir)
            {
                float r = float(ir) / float(_lutN - 1.0);
                float R = expf ((r - lutT) / lutM);

                size_t i = (ib * _lutN * _lutN + ig * _lutN + ir) * _channels;
                pixelValues[i + 0] = R;
                pixelValues[i + 1] = G;
                pixelValues[i + 2] = B;
                if ( _channels == 4 ) {
                    pixelValues[i + 3] = 1.0f;
                }
            }
        }
    }
}


bool GLLut3d::calculate_ocio( const CMedia* img )
{
    //
    // We build a 3D color lookup table by running a set of color
    // samples through a series of OCIO transforms.
    //
    // The 3D lookup table covers a range from lutMin to lutMax or
    // NUM_STOPS f-stops above and below 0.18 or MIDDLE_GRAY.  The
    // size of the table is _lutN by _lutN by _lutN samples.
    //
    // In order make the distribution of the samples in the table
    // approximately perceptually uniform, the Cg shaders that use
    // the table perform lookups in "log space":
    // In a Cg shader, the lookup table is represented as a 3D texture.
    // In order to apply the table to a pixel value, the Cg shader takes
    // the logarithm of the pixel value and scales and offsets the result
    // so that lutMin and lutMax map to 0 and 1 respectively.  The scaled
    // value is used to perform a texture lookup and the shader computes
    // e raised to the power of the result of the texture lookup.
    //
    //
    if ( !_inited )
    {
        _channels = 3;

        //
        // Init lut table to 0
        //
        clear_lut();

        //
        // Init table of pixel values
        //
        init_pixel_values( lut );
    }

    try
    {
        OCIO::ConstConfigRcPtr config = mrv::Preferences::OCIOConfig();
        const std::string& display = mrv::Preferences::OCIO_Display;
        const std::string& view = mrv::Preferences::OCIO_View;

#if OCIO_VERSION_HEX >= 0x02000000
        OCIO::DisplayViewTransformRcPtr transform =
          OCIO::DisplayViewTransform::Create();
#else
        OCIO::DisplayTransformRcPtr transform =
          OCIO::DisplayTransform::Create();
#endif

        std::string ics = img->ocio_input_color_space();
        if  ( ics.empty() )
        {
            OCIO::ConstColorSpaceRcPtr defaultcs = config->getColorSpace(OCIO::ROLE_SCENE_LINEAR);
            if(!defaultcs)
                throw std::runtime_error( _("ROLE_SCENE_LINEAR not defined." ));
            ics = defaultcs->getName();
        }


#if OCIO_VERSION_HEX >= 0x02000000
        transform->setSrc( ics.c_str() );
#else
        transform->setInputColorSpaceName( ics.c_str() );
#endif
        transform->setDisplay( display.c_str() );
        transform->setView( view.c_str() );

        OCIO::ConstProcessorRcPtr processor =
            config->getProcessor( transform );


#if OCIO_VERSION_HEX >= 0x02000000
        OCIO::ConstCPUProcessorRcPtr cpu =
            processor->getOptimizedCPUProcessor(OCIO::BIT_DEPTH_F32,
                                                OCIO::BIT_DEPTH_F32,
                                                OCIO::OPTIMIZATION_DEFAULT);
#endif

        OCIO::PackedImageDesc img(&lut[0],
                                  /* width */ lut_size()/_channels,
                                  /*height*/ 1,
                                  /*channels*/ _channels);

#if OCIO_VERSION_HEX >= 0x02000000
        cpu->apply( img );
#else
        processor->apply( img );
#endif
        DBG;

        // std::ostringstream os;
        // os << processor->getGpuShaderText(shaderDesc) << std::endl;
        // std::cerr << os.str() << std::endl;

        _inited = true;
    }
    catch( const OCIO::Exception& e)
    {
        OCIO_ERROR( e.what() );
        return false;
    }
    catch( const std::exception& e )
    {
        LOG_ERROR( e.what() );
        return false;
    }
    catch( ... )
    {
        LOG_ERROR( _("Unknown error returned from OCIO processor") );
        return false;
    }
    return true;
}













GLLut3d::GLLut3d_ptr GLLut3d::factory( const ViewerUI* view,
                                       const CMedia* img )
{
    const PreferencesUI* uiPrefs = view->uiPrefs;
    std::string path, fullpath;

    static const CMedia* lastImg = NULL;

    std::string ics = img->ocio_input_color_space();
    if ( ics.empty() ) {
        ics = OCIO::ROLE_SCENE_LINEAR;

        const char* var = uiPrefs->uiPrefsOCIOConfig->value();
        if ( var )
        {
            std::string ocio = var;
            if ( img->depth() == image_type::kByte ||
                 img->depth() == image_type::kShort ) {
                if ( ocio.rfind( "nuke-default" ) != std::string::npos )
                    ics = "sRGB";
                else
                    ics = "Output - sRGB";
            }
        }
        CMedia* c = const_cast< CMedia* >( img );
        c->ocio_input_color_space( ics );
    }

    OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();
    fullpath = config->getCacheID();
    fullpath += " -> ";

    path = ics;
    path += " -> " + Preferences::OCIO_Display;
    path += " -> " + Preferences::OCIO_View;

    fullpath += path;
    //
    // Check if this lut path was already calculated by some other
    // image.
    //
    {
        LutsMap::const_iterator i = _luts.find( fullpath );
        if ( i != _luts.end() && i->second->inited() )
        {
            // this lut was already created, return it.
            if ( lastImg != img )
            {
                lastImg = img;
                LOG_INFO( _("3D Lut for ") << img->name()
                          << _(" already created: " ) );
                LOG_INFO( path );
            }
            return i->second;
        }
    }

    if ( Preferences::use_ocio )
    {
        if ( img->ocio_input_color_space().empty() )
        {
            std::string msg = _( "Image input color space is undefined for ") +
                              img->name() + ".";
            mrv::PopupMenu* uiICS = view->uiICS;
            const char* const lbl = "scene_linear";
            for ( unsigned i = 0; i < uiICS->children(); ++i )
            {
                std::string name = uiICS->child(i)->label();
                if ( name == lbl )
                {
                    msg += _("  Choosing ") + name + ".";
                    CMedia* c = const_cast< CMedia* >( img );

                    char buf[1024];
                    sprintf( buf, "ICS \"%s\"", name.c_str() );
                    view->uiView->send_network( buf );

                    c->ocio_input_color_space( name );
                    uiICS->copy_label( lbl );
                    uiICS->value(i);
                    uiICS->redraw();
                    break;
                }
            }
            LOG_INFO( msg );
        }
        else
        {
            mrv::PopupMenu* uiICS = view->uiICS;
            const std::string& lbl = img->ocio_input_color_space();
            LOG_INFO( _("Input color space for ") << img->name() << _(" is ")
                      << lbl );
            for ( unsigned i = 0; i < uiICS->children(); ++i )
            {
                const char* name = uiICS->child(i)->label();
                if ( name && name == lbl )
                {
                    uiICS->copy_label( lbl.c_str() );
                    uiICS->value(i);
                    uiICS->redraw();
                    break;
                }
            }
        }
    }

    unsigned size = 64;
    unsigned lut_type = uiPrefs->uiLUT_quality->value();
    switch( lut_type )
    {
    case kLut32:
        size = 32;
        break;
    default:
    case kLut64:
        size = 64;
        break;
    case kLut96:
        size = 96;
        break;
    case kLut128:
        size = 128;
        break;
    case kLut192:
        size = 192;
        break;
    case kLut256:
        size = 256;
        break;
    case kNoBake:
        break;
    }

    //
    // Log information about lut path
    //
    LOG_INFO( _("3D Lut for ") << img->name() << N_(":") );
    LOG_INFO( path );

    GLLut3d_ptr lut( new GLLut3d(view, size) );


    char* oldloc = av_strdup( setlocale(LC_NUMERIC, NULL ) );
    setlocale(LC_NUMERIC, "C" );
    if ( ! lut->calculate_ocio( img ) )
        LOG_ERROR( _("Could not calculate OCIO Lut") );
    setlocale(LC_NUMERIC, oldloc );
    av_free( oldloc );

    lut->create_gl_texture();

    if ( _luts.find( fullpath ) != _luts.end() ) _luts.erase( path );
    _luts.insert( std::make_pair( fullpath, lut ) );
    return lut;
}


void GLLut3d::clear()
{
    _luts.clear();
}

} // namespace mrv
