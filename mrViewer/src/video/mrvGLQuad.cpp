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
/**
 * @file   mrvGLQuad.cpp
 * @author gga
 * @date   Fri Feb  8 10:14:11 2008
 *
 * @brief  Handle and draw an OpenGL Quad with shaders
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

#include <fltk/Cursor.h>

#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"

#include "mrViewer.h"  // yuck

#include "video/mrvGLShader.h"
#include "video/mrvGLEngine.h"
#include "video/mrvGLQuad.h"
#include "video/mrvGLLut3d.h"


//#define TEST_NO_QUAD         // test not using textures
//#define TEST_NO_PBO_TEXTURES // test not using pbo textures
#define NVIDIA_PBO_BUG     // with pbo textures, my nvidia card has problems
                           // with GL_BGR formats and high resolutions



// PBO macro (see spec for details)
#define BUFFER_OFFSET(i) ((char *)NULL + (i))


namespace
{
  const char* kModule = "glquad";
}


namespace mrv {

  const char* GLQuad::debug_internal_format( const GLenum format )
  {
    switch( format )
      {
      case GL_LUMINANCE8:
        return "GL_LUMINANCE8";
      case GL_LUMINANCE16F_ARB:
        return "GL_LUMINANCE16F_ARB";
      case GL_LUMINANCE32F_ARB:
        return "GL_LUMINANCE32F_ARB";
      case GL_RGB:
      case GL_RGB8:
        return "GL_RGB8";
      case GL_RGB16F_ARB:
        return "GL_RGB16F_ARB";
      case GL_RGB32F_ARB:
        return "GL_RGB32F_ARB";
      case GL_RGBA:
      case GL_RGBA8:
        return "GL_RGBA8";
      case GL_RGBA16F_ARB:
        return "GL_RGBA16F_ARB";
      case GL_RGBA32F_ARB:
        return "GL_RGBA32F_ARB";
      default:
        return "(unknown GL internal format)";
      }
  }


  const char* GLQuad::debug_format( const GLenum format )
  {
    switch( format )
      {
      case GL_LUMINANCE:
        return "GL_LUMINANCE";
      case GL_BGR:
        return "GL_BGR";
      case GL_BGRA:
        return "GL_BGRA";
      case GL_RGB:
        return "GL_RGB";
      case GL_RGBA:
        return "GL_RGBA";
      default:
        return "(unknown GL format)";
      }
  }

  const char* GLQuad::debug_pixel_type( const GLenum pixel_type )
  {
    switch( pixel_type )
      {
      case GL_UNSIGNED_BYTE:
        return "GL_UNSIGNED_BYTE";
      case GL_UNSIGNED_SHORT:
        return "GL_UNSIGNED_SHORT";
      case GL_UNSIGNED_INT:
        return "GL_UNSIGNED_INT";
      case GL_BYTE:
        return "GL_BYTE";
      case GL_SHORT:
        return "GL_SHORT";
      case GL_INT:
        return "GL_INT";
      case GL_HALF_FLOAT_ARB:
        return "GL_HALF_FLOAT_ARB";
      case GL_FLOAT:
        return "GL_FLOAT";
      case GL_DOUBLE:
        return "GL_DOUBLE";
      default:
        return "(unknown GL pixel type)";
      }
  }


  GLenum GLQuad::gl_pixel_type( const image_type::PixelType type )
  {
      static bool bad_format = false;
    switch( type )
      {
      case image_type::kByte:
        return GL_UNSIGNED_BYTE;
      case image_type::kShort:
        return GL_UNSIGNED_SHORT;
      case image_type::kInt:
        return GL_UNSIGNED_INT;
      case image_type::kHalf:
        return GL_HALF_FLOAT_ARB;
      case image_type::kFloat:
        return GL_FLOAT;
      default:
          if ( !bad_format )
          {
              bad_format = true;
              LOG_ERROR( "Unknown mrv::Frame pixel type: " << type );
          }
          return GL_UNSIGNED_BYTE;
          break;
      }
  }

  void GLQuad::clear_lut()
  {
      if (_image)
      {
          static ACES::ASC_CDL old;
          if ( old != _image->asc_cdl() )
          {
              if ( _lut ) _lut->clear_lut();
              old = _image->asc_cdl();
          }
      }
      _lut = NULL; // lut is not deleted here
      _image = NULL;
      _lut_attempt = 0;
  }


  GLenum GLQuad::gl_format( const image_type::Format format )
  {
      static bool bad_format = false;
      switch( format )
      {
      case image_type::kRGB:
          return GL_RGB;
      case image_type::kRGBA:
          return GL_RGBA;
      case image_type::kBGRA:
          return GL_BGRA;
      case image_type::kBGR:
          return GL_BGR;
      case image_type::kLumma:
      case image_type::kYUV:
      case image_type::kITU_709_YCbCr444A:
      case image_type::kITU_601_YCbCr444A:
      case image_type::kITU_709_YCbCr420A:
      case image_type::kITU_601_YCbCr420A:
      case image_type::kITU_709_YCbCr410A:
      case image_type::kITU_601_YCbCr410A:
      case image_type::kITU_709_YCbCr444:
      case image_type::kITU_601_YCbCr444:
      case image_type::kITU_709_YCbCr420:
      case image_type::kITU_601_YCbCr420:
      case image_type::kITU_709_YCbCr410:
      case image_type::kITU_601_YCbCr410:
          return GL_LUMINANCE;
      default:
          if ( !bad_format )
          {
              bad_format = true;
              LOG_ERROR( _("Invalid mrv::Frame format: ") << format );
          }
          return GL_LUMINANCE;
          break;
      }
  }

  unsigned int GLQuad::calculate_pow2( unsigned int w )
  {
    unsigned int r = 64;  // smaller textures may not work
    while ( r < w )
    {
      r = r << 1;
    }
    return r;
  }


  void GLQuad::init_texture()
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    if ( GLEW_EXT_texture_edge_clamp )
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECK_GL;
      }
    else
      {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        CHECK_GL;
      }


    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    CHECK_GL;
  }

  GLQuad::GLQuad( const ImageView* view ) :
    _view( view ),
    _shader( NULL ),
    _lut( NULL ),
    _image( NULL ),
    _lut_attempt( 0 ),
    _gamma( 1.f ),
    _right( false ),
    _blend( true ),
    _blend_mode( GL_ALPHA ),
    _num_textures( 1 ),
    _glformat( 0 ),
    _internalFormat( 0 ),
    _pixel_type( GL_BYTE ),
    _format( image_type::kLumma ),
    _width( 0 ),
    _height( 0 ),
    _mask( 0 ),
    _mask_value( -10 ),
    _channels( 0 ),
    _normMin( 0.f ),
    _normMax( 1.f )
  {
    _yw[0] = 0.2126f;
    _yw[1] = 0.7152f;
    _yw[2] = 0.0722f;

    if ( GLEngine::supports_yuv() )  _num_textures = 4;

    if ( GLEngine::pboTextures() ) {
       glGenBuffers( _num_textures, _pbo );
       CHECK_GL;
    }

    glGenTextures( _num_textures, _texId );
    CHECK_GL;


    for ( unsigned i = 0; i < _num_textures; ++i )
      {
        if ( GLEngine::maxTexUnits() > 3 )
        {
          glActiveTexture(GL_TEXTURE0 + i);
          CHECK_GL;
        }

        glEnable( GL_TEXTURE_2D );
        CHECK_GL;


        glBindTexture(GL_TEXTURE_2D, _texId[i] );
        CHECK_GL;

        init_texture();
      }
  }

  GLQuad::~GLQuad()
  {
      if ( GLEngine::pboTextures() )
      {
          glDeleteBuffers( _num_textures, _pbo );
          CHECK_GL;
      }
      glDeleteTextures( _num_textures, _texId );
      CHECK_GL;

    // NOTE: we don't delete neither the shader nor the lut, as those
    //       are not managed by us.
  }

  void GLQuad::update_texsub( unsigned int idx,
                              unsigned int rx, unsigned int ry,
                              unsigned int rw, unsigned int rh,
                              unsigned int tw, unsigned int th,
                              GLenum format, GLenum pixel_type,
                              unsigned short  channels,
                              unsigned short  pixel_size,
                              boost::uint8_t* pixels )
  {
    assert( pixels != NULL );
    assert( rx <  tw );
    assert( ry <  th );
    assert( rw > 0 );
    assert( rh > 0 );
    assert( rx+rw <= tw );
    assert( ry+rh <= th );


    if ( _view->field() == ImageView::kFrameDisplay )
      {
// #define TEST_NO_PBO_TEXTURES
#ifndef TEST_NO_PBO_TEXTURES

#ifdef NVIDIA_PBO_BUG
        if ( GLEngine::pboTextures() &&
             (format == GL_LUMINANCE || channels == 4) )
#else
        if ( GLEngine::pboTextures() )
#endif
            {

              glPixelStorei( GL_UNPACK_ROW_LENGTH, tw );
              glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

              // bind pixel buffer
              glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, _pbo[idx]);
              CHECK_GL;

              //
              // This avoids a potential stall.
              //
              // glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
              //           rw*rh*pixel_size, NULL, GL_STREAM_DRAW);
              glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
                           tw*th*pixel_size*channels, NULL, GL_STREAM_DRAW);
              CHECK_GL;

              // Acquire a pointer to the first data item in this buffer object
              // by mapping it to the so-called unpack buffer target. The unpack
              // buffer refers to a location in memory that gets "unpacked"
              // from CPU (main) memory into GPU memory, hence the somewhat
              // confusing language.
              // Important: This is a pointer to a chunk of memory in what I
              // like to call "driver-controlled memory". Depending on the
              // driver, this might be PCIe/AGP memory, or ideally onboard
              // memory.
              // GL_WRITE_ONLY tells the GL that while we have control of the
              // memory, we will access it write-only. This allows for internal
              // optimisations and increases our chances to get good performance.
              // If we mapped this buffer read/write, it would almost always be
              // located in system memory, from which reading is much faster.
              //
              char* ioMem = (char*)glMapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB,
                                                GL_WRITE_ONLY );
              CHECK_GL;
              if ( !ioMem )
                {
                  LOG_ERROR("Could not map pixel buffer #" << idx );
                  return; // we are in trouble
                }

              //
              // "memcpy" the double precision array into the driver memory,
              // doing the explicit conversion to single precision.
              //
              unsigned offset = ( ry * tw + rx ) * pixel_size * channels;
              unsigned size   = tw * rh * pixel_size * channels;

              memcpy( ioMem + offset, pixels, size );

              //
              // release memory, i.e. give control back to the driver
              //
              if ( ! glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB) )
                {
                  LOG_ERROR("Could not unmap pixel buffer #" << idx );
                  return;
                }


              //
              // Since the buffer object is still bound, this call populates our
              // texture by sourcing the buffer object. In effect, this means
              // that ideally no actual memcpy takes place (if the driver
              // decided to map the buffer in onboard memory previously), or at
              // most one memcpy inside driver-controlled memory which is much
              // faster since the previous copy of our data into driver-
              // controlled memory already resulted in laying out the data for
              // optimal performance.
              // In other words: This call is at most a real DMA transfer,
              // without any (expensive) interference by the CPU.
              //
              glTexSubImage2D(GL_TEXTURE_2D, 0, rx, ry, rw, rh, format,
                              pixel_type, BUFFER_OFFSET(0) );
              CHECK_GL;
              //
              // Unbind buffer object by binding it to zero.
              // This call is crucial, as doing the following computations
              // while the buffer object is still bound will result in all
              // sorts of weird side effects, eventually even delivering wrong
              // results.
              // Refer to the specification to learn more about which
              // GL calls act differently while a buffer is bound.
              glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
              CHECK_GL;
            }
          else
#endif // TEST_NO_PBO_TEXTURES
            {
              glPixelStorei( GL_UNPACK_ROW_LENGTH, tw );

              //
              // Handle copying FRAME AREA to 2D texture
              //
              glTexSubImage2D( GL_TEXTURE_2D,
                               0,             // level
                               rx, ry,        // offset
                               rw, rh,
                               format,        // format
                               pixel_type,
                               pixels );
              CHECK_GL;
            }
        CHECK_GL;
      }
    else
      {
        //
        // Handle copying FIELDS AREA to 2D texture
        //
        int off = 1;
        if ( _view->field() == ImageView::kBottomField ) off = -1;

        ry += 1 * ( off == -1 );
        unsigned step = tw * pixel_size * channels;

        if ( off == -1 ) pixels += step;
        step *= 2;

        unsigned int yo = ry + rh;
        for ( ; ry < yo; ry += 2, pixels += step )
        {
            glTexSubImage2D( GL_TEXTURE_2D, 0, rx, ry, rw, 1, format,
                             pixel_type, pixels );
            CHECK_GL;
            glTexSubImage2D( GL_TEXTURE_2D, 0, rx, ry+off, rw, 1, format,
                             pixel_type, pixels );
            CHECK_GL;
        }
      }

  }

  void GLQuad::bind_texture_yuv( const image_type_ptr pic,
                                 const unsigned int poww,
                                 const unsigned int powh )
  {
    unsigned dw = pic->width();
    unsigned dh = pic->height();


    unsigned int   channels = pic->channels();
    GLenum         pixel_type = gl_pixel_type( pic->pixel_type() );
    unsigned short pixel_size = pic->pixel_size();
    _pixels = pic->data();
    if ( !_pixels ) return;

    unsigned offset = 0;

    //
    // Choose best internal format based on gfx card capabilities
    //
    GLenum internalFormat = GL_LUMINANCE8;
    if ( pixel_type == GL_UNSIGNED_SHORT )
    {
        internalFormat = GL_LUMINANCE16;
    }
    else if ( pixel_type == GL_HALF_FLOAT_ARB || pixel_type == GL_FLOAT )
      {
        if ( GLEngine::halfTextures() )
           internalFormat = GL_LUMINANCE16F_ARB;
        else if ( GLEngine::floatTextures() )
           internalFormat = GL_LUMINANCE32F_ARB;
      }


    for ( unsigned i = 0; i < channels; ++i )
      {
        // Load & bind the texture

        // As GL_TEXTURE3 is the 3D Lut, any alpha gets bound on GL_TEXTURE4
        int idx = (i == 3 ? 4 : i);

        glActiveTexture(GL_TEXTURE0 + idx);
        CHECK_GL;

        glEnable( GL_TEXTURE_2D );
        CHECK_GL;

        glBindTexture(GL_TEXTURE_2D, _texId[i] );
        CHECK_GL;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        CHECK_GL;


        unsigned int tw = dw;
        unsigned int th = dh;

        unsigned int ow = dw;
        unsigned int oh = dh;

        if ( GLEngine::pow2Textures() )
          {
            tw = poww;
            th = powh;
          }

        if ( i % 3 != 0 && i != 4 )
        {
           switch( pic->format() )
           {
              case image_type::kITU_601_YCbCr410:
              case image_type::kITU_601_YCbCr410A:
              case image_type::kITU_709_YCbCr410:
              case image_type::kITU_709_YCbCr410A:
              case image_type::kYByRy410:
              case image_type::kYByRy410A:
                 ow = tw;
                 oh = th;
                 break;
              case image_type::kYUV:
              case image_type::kYUVA:
              case image_type::kITU_601_YCbCr420:
              case image_type::kITU_601_YCbCr420A:
              case image_type::kITU_709_YCbCr420:
              case image_type::kITU_709_YCbCr420A:
              case image_type::kYByRy420:
              case image_type::kYByRy420A:
                 tw = (tw+1) / 2;
                 th = (th+1) / 2;
                 ow = (dw+1) / 2;
                 oh = (dh+1) / 2;
                 break;
              case image_type::kITU_601_YCbCr422A:
              case image_type::kITU_601_YCbCr422:
              case image_type::kITU_709_YCbCr422A:
              case image_type::kITU_709_YCbCr422:
              case image_type::kYByRy422:
              case image_type::kYByRy422A:
                 tw = (tw+1) / 2;
                 ow = (dw+1) / 2;
                 break;
              case image_type::kRGB:
              case image_type::kRGBA:
              case image_type::kBGR:
              case image_type::kBGRA:
              case image_type::kITU_601_YCbCr444:
              case image_type::kITU_601_YCbCr444A:
              case image_type::kITU_709_YCbCr444:
              case image_type::kITU_709_YCbCr444A:
                 break;
              default:
                 LOG_ERROR("Wrong pixel format " << pic->format()
                           << " for yuv");
                 break;
           }
        }


        if ( _width      != dw ||
             _height     != dh ||
             _pixel_type != pixel_type ||
             _channels   != channels ||
             _glformat       != GL_LUMINANCE ||
             _internalFormat != internalFormat )
          {
            if ( i == channels-1 )
              {
                _width = dw;
                _height = dh;
                _pixel_type = pixel_type;
                _channels = channels;
                _format = pic->format();
                // _shader = ( pic->format() >= image_type::kYByRy420 ?
                //            GLEngine::YByRyShader() : GLEngine::YCbCrShader() );
                _glformat = GL_LUMINANCE;
                _internalFormat = internalFormat;
              }

#ifndef TEST_NO_PBO_TEXTURES
            if ( GLEngine::pboTextures() )
              {
                // Define texture level zero (without an image); notice the
                // explicit bind to the zero pixel unpack buffer object so that
                // pass NULL for the image data leaves the texture image
                // unspecified.
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
                CHECK_GL;
              }
#endif

            unsigned htw = tw;
            unsigned hth = th;


            if ( _view->stereo_input() &
                 CMedia::kTopBottomStereoInput )
            {
                hth /= 2;
            }
            else if ( _view->stereo_input() &
                      CMedia::kLeftRightStereoInput )
            {
                htw /= 2;
            }

            glTexImage2D( GL_TEXTURE_2D,
                          0,              // level
                          internalFormat,  // internalFormat
                          htw, hth,
                          0,             // border
                          GL_LUMINANCE,  // texture data format
                          pixel_type,         // texture pixel type
                          NULL );        // texture pixel data

            CHECK_GL;
          }

          unsigned off = 0;
          if ( _right )
          {
              if ( _view->stereo_input() & CMedia::kTopBottomStereoInput )
              {
                  unsigned dy = oh / 2;
                  off = ( dy * ow ) * pixel_size;
              }
              else if ( _view->stereo_input() & CMedia::kLeftRightStereoInput )
              {
                  unsigned dx = ow / 2;
                  off = dx * pixel_size;
              }
          }

          boost::uint8_t* p = (boost::uint8_t*)_pixels.get() + offset + off;
          offset += ow * oh * pixel_size;

          if ( _view->stereo_input() & CMedia::kTopBottomStereoInput )
          {
              oh /= 2;
          }
          else if ( _view->stereo_input() & CMedia::kLeftRightStereoInput )
          {
              ow /= 2;
          }

          update_texsub( i, 0, 0, ow, oh, tw, th, GL_LUMINANCE, pixel_type,
                         1, pixel_size, p );
      }
  }

  void GLQuad::bind_texture_quad( const image_type_ptr pic,
                                  const unsigned poww, const unsigned int powh )
  {
      if ( pic->format() >= image_type::kYUV )
        return bind_texture_yuv( pic, poww, powh );


    unsigned dw = pic->width();
    unsigned dh = pic->height();

    unsigned int channels = pic->channels();
    GLenum pixel_type = gl_pixel_type( pic->pixel_type() );
    unsigned short pixel_size = pic->pixel_size();
    GLenum glformat   = gl_format( pic->format() );
    _pixels = pic->data();
    if (!_pixels) return;

    GLenum internalFormat = GL_RGB8;

    if ( pic->has_alpha() )
      {
        internalFormat = GL_RGBA8;

        //
        // Choose best internal format based on gfx card capabilities
        //
        if ( pixel_type == GL_HALF_FLOAT_ARB || pixel_type == GL_FLOAT )
          {
            if ( GLEngine::halfTextures() )
              internalFormat = GL_RGBA16F_ARB;
            else if ( GLEngine::floatTextures() )
              internalFormat = GL_RGBA32F_ARB;
          }

      }
    else
      {
        //
        // Choose best internal format based on gfx card capabilities
        //
        if ( pixel_type == GL_HALF_FLOAT_ARB || pixel_type == GL_FLOAT )
          {
            if ( GLEngine::halfTextures() )
              internalFormat = GL_RGB16F_ARB;
            else if ( GLEngine::floatTextures() )
              internalFormat = GL_RGB32F_ARB;
          }
      }


    // Load & bind the texture
    if ( GLEW_ARB_multitexture )
    {
        CHECK_GL;
        glActiveTexture( GL_TEXTURE0 );
        CHECK_GL;
    }

    glBindTexture( GL_TEXTURE_2D, _texId[0] );
    CHECK_GL;

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    CHECK_GL;


    unsigned tw = dw;
    unsigned th = dh;
    if ( GLEngine::pow2Textures() )
      {
        tw = poww;
        th = powh;
      }

    if ( _view->stereo_input() & CMedia::kTopBottomStereoInput )
    {
        dh /= 2;
    }
    else if ( _view->stereo_input() & CMedia::kLeftRightStereoInput )
    {
        dw /= 2;
    }


    // If texture size is different than old frame/image,
    // allocate new quad texture
    if ( _width          != dw ||
         _height         != dh ||
         _channels       != channels ||
         _pixel_type     != pixel_type ||
         _glformat       != glformat   ||
         _internalFormat != internalFormat )
      {
        _width          = dw;
        _height         = dh;
        _channels       = channels;
        _pixel_type     = pixel_type;
        _glformat       = glformat;
        _format         = pic->format();
        _internalFormat = internalFormat;
        DBG( "GLEngine::shader_type() " << GLEngine::shader_type() << " "
             << GLEngine::shader_type_name() );
        if ( GLEngine::shader_type() ) _shader = GLEngine::rgbaShader();
        else                           _shader = NULL;

#ifndef TEST_NO_PBO_TEXTURES
        if ( GLEngine::pboTextures() )
          {
            // Define texture level zero (without an image); notice the
            // explicit bind to the zero pixel unpack buffer object so that
            // pass NULL for the image data leaves the texture image
            // unspecified.
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            CHECK_GL;
          }
#endif

        unsigned htw = tw;
        unsigned hth = th;


        if ( _view->stereo_input() &
             CMedia::kTopBottomStereoInput )
        {
            hth /= 2;
            th = hth;
        }
        else if ( _view->stereo_input() &
                  CMedia::kLeftRightStereoInput )
        {
            htw /= 2;
        }
        glTexImage2D( GL_TEXTURE_2D,
                      0,               // level
                      _internalFormat,  // internalFormat
                      htw, hth,
                      0,               // border
                      _glformat,
                      _pixel_type,
                      NULL );

        CHECK_GL;

      }


    // Upload converted rectangle
    unsigned off = 0;
    if ( _right )
    {
        unsigned psize = pixel_size * channels;
        if ( _view->stereo_input() & CMedia::kTopBottomStereoInput )
        {
            unsigned dy = dh;
            off = ( dy * tw ) * psize;
        }
        else if ( _view->stereo_input() & CMedia::kLeftRightStereoInput )
        {
            unsigned dx = dw;
            off = dx * psize;
        }
    }
    boost::uint8_t* p = (boost::uint8_t*)_pixels.get() + off;

    update_texsub( 0, 0, 0, dw, dh, tw, th, _glformat, _pixel_type,
                   short(_channels), short(pixel_size), p );
  }

  /// Prepare a texture for opengl
  void GLQuad::bind_texture_pixels( const mrv::image_type_ptr pic )
  {
    unsigned dw = pic->width();
    unsigned dh = pic->height();

    unsigned int channels = pic->channels();
    GLenum pixel_type = gl_pixel_type( pic->pixel_type() );
    GLenum glformat = gl_format( pic->format() );
    _pixels = pic->data();
    if ( !_pixels ) return;

    if ( _width  != dw ||
         _height != dh ||
         _pixel_type != pixel_type ||
         _channels != channels ||
         _glformat != glformat )
      {
          static bool warn = false;

          if ( !warn )
          {
              warn = true;
              LOG_WARNING( "Image bigger than "
                           << GLEngine::maxTexWidth() << " x "
                           << GLEngine::maxTexHeight() << ".  "
                           "Will use scanline drawing which is buggy." );
          }
      }

    _width = dw;
    _height = dh;
    _channels = channels;
    _pixel_type = pixel_type;
    _glformat = glformat;
    _format   = pic->format();
    _uvMax.u = _uvMax.v = 0.0f;
  }


  void GLQuad::bind( const image_type_ptr pic )
  {
      CHECK_GL;
    if ( ! pic ) {
        LOG_ERROR( _("Not a picture to be bound") );
        return;
    }
    unsigned dw = pic->width();
    unsigned dh = pic->height();
    if ( dw <= 0 || dh <= 0 ) {
        LOG_ERROR( _("Not a valid picture to be bound") );
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


#ifndef TEST_NO_QUAD
    if ( poww <= GLEngine::maxTexWidth() && powh <= GLEngine::maxTexHeight() )
      {
        bind_texture_quad( pic, poww, powh );
      }
    else
#endif
      {
        bind_texture_pixels( pic );
      }
  }


  void GLQuad::draw_quad( const unsigned dw, const unsigned dh ) const
  {
    glColor4f(1.0f,1.0f,1.0f,1.0f);

    // If in presentation mode (fullscreen), we use a linear texture filter.
    // Otherwise, use a nearest one to clearly show pixels.
    bool presentation = _view->in_presentation();
    GLenum filter = presentation ? GL_LINEAR : GL_NEAREST;

    if ( _shader && _shader != GLEngine::rgbaShader() )
      {
          short i = short(_channels - 1);
        for ( ; i >= 0 ; --i )
          {
              short idx = (i == 3 ? (short) 4 : i );
              glActiveTexture(GL_TEXTURE0 + idx);
              CHECK_GL;
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
            if ( _shader == GLEngine::YByRyAShader() ||
                 _shader == GLEngine::YCbCrAShader() )
                _shader->setTextureUnit( "AImage", 4 );
          }
        else
          {
            _shader->setTextureUnit( "fgImage", 0 );
          }

        _shader->setTextureUnit( "lut", 3 );
        CHECK_GL;

        _shader->setUniform( "mask", _mask );
        CHECK_GL;
        _shader->setUniform( "mask_value", _mask_value );
        CHECK_GL;
        _shader->setUniform( "width", _width );
        CHECK_GL;
        _shader->setUniform( "height", _height );
        CHECK_GL;

        _shader->setUniform( "gain",  _view->gain() );
        CHECK_GL;
        _shader->setUniform( "gamma", 1.0f/_gamma );
        CHECK_GL;

        _shader->setUniform( "channel", _view->channel_type() );
        CHECK_GL;

        if ( use_lut && _lut )
          {
            _shader->setUniform( "enableLut", 1 );
            CHECK_GL;
            _shader->setUniform( "lutMin", _lut->lutMin );
            CHECK_GL;
            _shader->setUniform( "lutMax", _lut->lutMax );
            CHECK_GL;
            _shader->setUniform( "lutM", _lut->lutM );
            CHECK_GL;
            _shader->setUniform( "lutT", _lut->lutT );
            CHECK_GL;
            _shader->setUniform( "lutF", false );


            unsigned len = lut()->edge_len();
            float scale = ( (float) len - 1.0f ) / (float) len;
            float offset = 1.0f / ( 2.0f * len );

            _shader->setUniform( "scale", scale );
            _shader->setUniform( "offset", offset );
          }
        else
          {
            _shader->setUniform( "enableLut", 0 );
	    CHECK_GL;
          }

        const mrv::ViewerUI* v = _view->main();
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
        CHECK_GL;
        if ( _shader == GLEngine::rgbaShader() )
            _shader->setUniform( "unpremult", unpremult );
        CHECK_GL;

        if ( _view->normalize() )
          {
            _shader->setUniform( "enableNormalization", 1 );
        CHECK_GL;
            _shader->setUniform( "normMin",  _normMin);
        CHECK_GL;
            _shader->setUniform( "normSpan", _normMax - _normMin );
        CHECK_GL;
          }
        else
          {
            _shader->setUniform( "enableNormalization", 0 );
        CHECK_GL;
          }

        if ( _shader == GLEngine::YByRyShader() ||
             _shader == GLEngine::YByRyAShader() )
          {
            _shader->setUniform( "yw", _yw[0], _yw[1], _yw[2] );
        CHECK_GL;
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
                  CHECK_GL;
                  // HDTV  YCbCr coefficients,
                  _shader->setUniform( "Koff", -0.0625f, -0.5f, -0.5f );
                  CHECK_GL;
                  _shader->setUniform( "Kr", 1.16438356f, 0.f, 1.79274107f );
                  CHECK_GL;
                  _shader->setUniform( "Kg", 1.16438356f, -0.21324861,
                                       -0.53290933 );
                  CHECK_GL;
                  _shader->setUniform( "Kb", 1.16438356f, 2.11240179f, 0.0f );
                  CHECK_GL
              }
              else if ( colorspace == "BT470BG" ||
                        colorspace == "SMPTE170M" )
              {
                  _shader->setUniform( "coeffs", 1 );
        CHECK_GL;
                  // STV  YCbCr coefficients
                  _shader->setUniform( "Koff", -16.0f/255.0f, -0.5f, -0.5f );
        CHECK_GL;
                  _shader->setUniform( "Kr", 1.0f, 0.0f, 1.59602715f );
        CHECK_GL;
                  _shader->setUniform( "Kg", 1.0f, -0.39465f, -0.58060f );
        CHECK_GL;
                  _shader->setUniform( "Kb", 1.0f, 2.03211f, 0.0f );
        CHECK_GL;
              }
              else if ( colorspace == "BT2020_NCL" ||
                        colorspace == "BT2020_CL" )
              {
                  // ULTRA HDTV  YCbCr coefficients (BT2020) - Not HDR
                _shader->setUniform( "coeffs", 1 );

                _shader->setUniform( "Koff",  0.0f, -0.5f, -0.5f );
                CHECK_GL;

                // // Full gamut
                // _shader->setUniform( "Kr", 1.164384f, 0.0f, 1.792741f );
                // CHECK_GL;
                // _shader->setUniform( "Kg", 1.164384f, -0.213249f, -0.532909f );
                // CHECK_GL;
                // _shader->setUniform( "Kb", 1.164384f, 2.112402f, 0.0f );

                // // Full gamut
                _shader->setUniform( "Kr", 1.164384f, 0.0f, 1.678674 );
                CHECK_GL;
                _shader->setUniform( "Kg", 1.164384f, -0.187326, -0.650424f );
                CHECK_GL;
                _shader->setUniform( "Kb", 1.164384f, 2.141772, 0.0f );

            }
              else if ( colorspace == "YCOCG" )
              {
                  _shader->setUniform( "coeffs", 1 );
        CHECK_GL;
                  _shader->setUniform( "Koff", 0.f, 0.f, 0.f );
        CHECK_GL;
                  _shader->setUniform( "Kr", 1.0f, -1.0f, 1.0f );  // Y
        CHECK_GL;
                  _shader->setUniform( "Kg", 1.0f,  1.0f, 0.0f );  // Cg
        CHECK_GL;
                  _shader->setUniform( "Kb", 1.0f, -1.0f, -1.0f ); // Co
        CHECK_GL;
              }
              else
              {
                  _shader->setUniform( "coeffs", 0 );
        CHECK_GL;
              }
          }

        CHECK_GL;
      }



    glBegin( GL_QUADS );
    {
      glTexCoord2f( 0.0f, _uvMax.v );
      glVertex3d( -0.5, -0.5, 0.0 );

      glTexCoord2f( _uvMax.u, _uvMax.v );
      glVertex3d(  0.5, -0.5, 0.0 );

      glTexCoord2f( _uvMax.u, 0.0f );
      glVertex3d(  0.5, 0.5, 0.0 );

      glTexCoord2f( 0.0f, 0.0f );
      glVertex3d( -0.5, 0.5, 0.0 );
    }
    glEnd();


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


void GLQuad::lut( const CMedia* img )
{

    // Check if already calculated
    if ( _lut && img == _image ) {
        return;
    }
    // Not calculated.  Count lut attempt
    if ( _lut == NULL && _image )
    {
        ++_lut_attempt;

        // If we failed twice or more, quick exit
        if ( _lut_attempt >= 2 ) {
            _lut_attempt = 1;
            return;
        }
    }

    _view->window()->cursor( fltk::CURSOR_WAIT );

    _lut   = mrv::GLLut3d::factory( _view->main(), img );
    _image = img;

    _view->window()->cursor( fltk::CURSOR_DEFAULT );
}



  int GLQuad::calculate_gl_step( const GLenum format,
                                 const GLenum pixel_type ) const
  {
    int step;
    switch( format )
    {
       case GL_RGBA:
       case GL_BGRA:
          step = 4; break;
       case GL_RGB:
       case GL_BGR:
          step = 3; break;
       case GL_LUMINANCE:
          step = 1; break;
       default:
          step = 1; break;
    }

    switch( pixel_type )
    {
       case GL_UNSIGNED_BYTE:
       case GL_BYTE:
          step *= (int)sizeof(char); break;
       case GL_SHORT:
       case GL_UNSIGNED_SHORT:
           step *= (int)sizeof(short); break;
       case GL_INT:
       case GL_UNSIGNED_INT:
          step *= (int)sizeof(int); break;
       case GL_HALF_FLOAT_ARB:
          step *= (int)sizeof(float)/2; break;
       case GL_FLOAT:
          step *= (int)sizeof(float); break;
       case GL_DOUBLE:
          step *= (int)sizeof(double); break;
       default:
          LOG_ERROR("Unknown OpenGL pixel type " << pixel_type );
          step *= sizeof(char);
    }

    return step;
  }

  void GLQuad::draw_pixels( const unsigned dw, const unsigned dh ) const
  {
    unsigned dh1 = dh;
    if ( _view->main()->uiPixelRatio->value() )
      dh1 = (unsigned)(dh1 / _view->pixel_ratio());

    if ( _view->field() == ImageView::kFrameDisplay || (dh % 2 != 0) )
      draw_frame( dw, dh1 );
    else
      draw_field( dw, dh1 );
  }

  /**
   * Draw an 8-bit image onto the viewport using opengl's glDrawPixels.
   *
   * @param dw     image's width
   * @param dh     image's height
   */
  void GLQuad::draw_frame( const unsigned dw, const unsigned dh ) const
  {

    assert( dw > 0 && dh > 0 );



    double p = _view->zoom();
    assert( p > 0.0 );

#if 0
    double sh = ((double)_view->h() + (double) dh*p ) / 2.0;
    double dy = (_view->offset_y() * p - sh );
#else
    double dy = ((_view->offset_y() - dh) * p );
#endif

    double dx = (_view->offset_x() * p );


    int step = calculate_gl_step( _glformat, _pixel_type );

    // Check zoom fraction
    int z = (int) _view->zoom();
    float zoom_fraction =  _view->zoom() - (float) z;


    unsigned H = dh - 1;
    double yp = dy - _view->zoom();

    ///// Draw Buffer
    glPixelZoom( _view->zoom(), _view->zoom() );

    const boost::uint8_t* base = (const boost::uint8_t*)_pixels.get();

    for (unsigned int y = 0; y <= H; ++y)
      {
          yp += _view->zoom();
          // if ( yp < -_view->zoom() || yp >= _view->h() ) continue;

          // To avoid opengl raster clipping issues, instead of:
          // glRasterPos2f( dx, yp );
          // we do:
          glRasterPos2i(0, 0);
          glBitmap( 0, 0, 0, 0, float(dx), float(yp), NULL );
          //

          unsigned int r = (H-y) * dw;
          const boost::uint8_t* ptr = base + r * step;
          glDrawPixels (dw,                     // width
                        1,                      // height
                        _glformat,	// format
                        _pixel_type,	// type
                        ptr );
      }


    if ( zoom_fraction < 1e-5 ) return;

    // To avoid floating point errors, we send two lines
    // when zoom is not an integer.
    yp = dy - _view->zoom() + 0.499f;
    for (unsigned int y = 0; y <= H; ++y)
      {
        yp += _view->zoom();
        //if ( yp < -_view->zoom() || yp >= _view->h() ) continue;

        // To avoid opengl raster clipping issues, instead of:
        //glRasterPos2f(dx, yp);
        // we do:
        glRasterPos2i(0, 0);
        glBitmap( 0, 0, 0, 0, float(dx), float(yp), NULL );
        //

        unsigned int r = (H-y) * dw;
        const boost::uint8_t* ptr = base + r * step;
        glDrawPixels (dw,                       // width
                      1,                        // height
                      _glformat,	// format
                      _pixel_type,	// type
                      ptr );
      }
  }

  /**
   * Draw an 8-bit image onto the viewport using opengl.
   *
   * @param format      GL_RGB, GL_BGR, GL_RGBA, GL_BGRA
   * @param pixel_type  GL_UNSIGNED_BYTE or GL_FLOAT
   * @param dw     image's width
   * @param dh     image's height
   * @param data   image 8-bit data in RGBA order.
   */
  void GLQuad::draw_field( const unsigned int dw, const unsigned int dh ) const
  {

      double p = _view->zoom();

      double sw = ((double)_view->w() - (double) dw * p) / 2.0f;
      double sh = ((double)_view->h() + (double) dh * p) / 2.0f;

      double dx = (_view->offset_x() * p);
      double dy = (_view->offset_y() * p - sh);

    int step = calculate_gl_step( _glformat, _pixel_type );

    // Check zoom fraction
    int z = (int) _view->zoom();
    float zoom_fraction =  _view->zoom() - (float) z;


    mrv::ImageView::FieldDisplay field = _view->field();


    unsigned H = dh - 1;
    double yp = dy - _view->zoom();

    ///// Draw Buffer
    glPixelZoom( _view->zoom(), _view->zoom() );

    const boost::uint8_t* base = (const boost::uint8_t*)_pixels.get();

    for (unsigned int y = 0; y <= H; ++y)
      {
        yp += _view->zoom();
        if ( yp < -_view->zoom() || yp >= _view->h() ) continue;

        // To avoid opengl raster clipping issues, instead of:
        //    glRasterPos2f(dx, yp);
        // we do:
        glRasterPos2i(0, 0);
        glBitmap( 0, 0, 0, 0, float(dx), float(yp), NULL );
        //
        unsigned int r = H - (y / 2)*2;
        if ( field == ImageView::kTopField )
          {
            if ( r > 1 ) r -= 1;
          }

        r *= dw;
        const boost::uint8_t* ptr = base + r * step;
        glDrawPixels (dw,		// width
                      1,		// height
                      _glformat,	// format
                      _pixel_type,	// type
                      ptr );
      }

    CHECK_GL;
    if ( zoom_fraction < 1e-5 ) return;

    // To avoid floating point errors, we send two lines
    // when zoom is not an integer.
    yp = double(dy) - _view->zoom();
    for (unsigned int y = 0; y <= H; ++y)
      {
        yp += _view->zoom();
        if ( yp < -_view->zoom() || yp >= _view->h() ) continue;

        // To avoid opengl raster clipping issues, instead of:
        //    glRasterPos2f(dx, yp-0.499f);
        // we do:
        glRasterPos2i(0, 0);
        glBitmap( 0, 0, 0, 0, float(dx), float(yp-0.499), NULL );
        //

        unsigned int r = H - (y / 2)*2;
        if ( field == ImageView::kTopField )
          {
            if ( r > 1 ) r -= 1;
          }
        r *= dw;

        const boost::uint8_t* ptr = base + r * step;
        glDrawPixels (dw,                       // width
                      1,                        // height
                      _glformat,	// format
                      _pixel_type,	// type
                      ptr );
      }
    CHECK_GL;
  }


  void GLQuad::draw( const unsigned dw, const unsigned dh ) const
  {
      if ( _uvMax.u > 0.0f )
          draw_quad( dw, dh );
      else
          draw_pixels( dw, dh );
  }

}
