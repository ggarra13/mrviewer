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

#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"

#include "mrViewer.h"  // yuck

#include "mrvGLShader.h"
#include "mrvGLEngine.h"
#include "mrvGLQuad.h"
#include "mrvGLLut3d.h"


// #define TEST_NO_QUAD         // test not using textures
// #define TEST_NO_PBO_TEXTURES // test not using pbo textures
#define NVIDIA_PBO_BUG     // with pbo textures, my nvidia card has problems
                           // with GL_BGR formats and high resolutions


#ifdef DEBUG
#  define CHECK_GL(x) GLEngine::handle_gl_errors(x)
#else
#  define CHECK_GL(x)
#endif

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
              return GL_LUMINANCE;
          default:
              if ( !bad_format )
              {
                  bad_format = true;
                  LOG_ERROR( "Invalid mrv::Frame format: " << format );
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
	CHECK_GL( "init_texture glTexParameter wrap clamp to edge" );
      }
    else
      {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	CHECK_GL( "init_texture glTexParameter wrap clamp" );
      }

    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    CHECK_GL( "init_texture glTexEnv mode replace" );
  }

  GLQuad::GLQuad( const ImageView* view ) :
    _view( view ),
    _shader( NULL ),
    _lut( NULL ),
    _image( NULL ),
    _gamma( 1.f ),
    _blend( true ),
    _blend_mode( GL_ALPHA ),
    _num_textures( 1 ),
    _glformat( 0 ),
    _internalFormat( 0 ),
    _pixel_type( GL_BYTE ),
    _format( image_type::kLumma ),
    _width( 0 ),
    _height( 0 ),
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
    }

    glGenTextures( _num_textures, _texId );


    for ( unsigned i = 0; i < _num_textures; ++i )
      {
	if ( GLEngine::maxTexUnits() > 3 )
	{
	  glActiveTexture(GL_TEXTURE0 + i);
	  CHECK_GL( "init_textures glActiveTexture" );
	}

	glEnable( GL_TEXTURE_2D );
	CHECK_GL( "init_textures glEnable" );
	

	glBindTexture(GL_TEXTURE_2D, _texId[i] );
	CHECK_GL( "init_textures glBindTexture" );

	init_texture();
      }
  }

  GLQuad::~GLQuad()
  {
      if ( GLEngine::pboTextures() )
      {
          glDeleteBuffers( _num_textures, _pbo );
          CHECK_GL( "glDeleteBuffers" );
      }
      glDeleteTextures( _num_textures, _texId );
      CHECK_GL( "glDeleteTextures" );

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
#ifndef TEST_NO_PBO_TEXTURES

#ifdef NVIDIA_PBO_BUG 
	if ( GLEngine::pboTextures() && 
	     (format == GL_LUMINANCE || channels == 4) )
#else
	if ( GLEngine::pboTextures() )
#endif
	    {

	      // bind pixel buffer
	      glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, _pbo[idx]);
              CHECK_GL( "glBindBuffer" );

	      //
	      // This avoids a potential stall.
	      //
	      glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
			   tw*th*pixel_size*channels, NULL, GL_STREAM_DRAW);
              CHECK_GL( "glBufferData" );

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
              CHECK_GL( "glMapBuffer" );
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
	      unsigned size   = rw * rh * pixel_size * channels;

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
              CHECK_GL( "glTexSubImage2D" );
	      //
	      // Unbind buffer object by binding it to zero.
	      // This call is crucial, as doing the following computations while 
	      // the buffer object is still bound will result in all sorts of
	      // weird side effects, eventually even delivering wrong results.
	      // Refer to the specification to learn more about which 
	      // GL calls act differently while a buffer is bound.
	      glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
              CHECK_GL( "glBindBuffer" );
	    }
	  else
#endif // TEST_NO_PBO_TEXTURES
	    {
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
              CHECK_GL( "no PBO glTexSubImage2D" );
	    }
	CHECK_GL( "bind_texture glTexSubImage2D" );
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
	    CHECK_GL( "bind_texture glTexSubImage2D" );
	    glTexSubImage2D( GL_TEXTURE_2D, 0, rx, ry+off, rw, 1, format,
			     pixel_type, pixels );
	    CHECK_GL( "bind_texture glTexSubImage2D" );
	  }
      }

  }

  void GLQuad::bind_texture_yuv( const image_type_ptr& pic,
				 const unsigned int poww, 
				 const unsigned int powh )
  {
    unsigned dw = pic->width();
    unsigned dh = pic->height();

    unsigned offset = 0;

    unsigned int   channels = pic->channels();
    GLenum         pixel_type = gl_pixel_type( pic->pixel_type() );
    unsigned short pixel_size = pic->pixel_size();
    _pixels = pic->data();

    //
    // Choose best internal format based on gfx card capabilities
    //
    GLenum internalFormat = GL_LUMINANCE8;
    if ( pixel_type == GL_HALF_FLOAT_ARB || pixel_type == GL_FLOAT )
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
	CHECK_GL( "bind_texture_yuv glActiveTexture" );

	glEnable( GL_TEXTURE_2D );
	CHECK_GL( "bind_texture_yuv glEnable" );

	glBindTexture(GL_TEXTURE_2D, _texId[i] );
	CHECK_GL( "bind_texture_yuv glBindTexture" );

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	CHECK_GL( "bind_texture_yuv glPixelStore" );


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
		_shader = ( pic->format() >= image_type::kYByRy420 ? 
			   GLEngine::YByRyShader() : GLEngine::YCbCrShader() );
		_glformat = GL_LUMINANCE;
		_internalFormat = internalFormat;
	      }

	    if ( GLEngine::pboTextures() )
	      {
		// Define texture level zero (without an image); notice the
		// explicit bind to the zero pixel unpack buffer object so that
		// pass NULL for the image data leaves the texture image
		// unspecified.
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	      }

	    glTexImage2D( GL_TEXTURE_2D,
			  0,              // level
			  internalFormat,  // internalFormat
			  tw, th, 
			  0,             // border
			  GL_LUMINANCE,  // texture data format
			  pixel_type,         // texture pixel type
			  NULL );        // texture pixel data


	    CHECK_GL( "bind_texture_yuv glTexImage2D" );
	  }    


	boost::uint8_t* p = (boost::uint8_t*)_pixels.get() + offset;
	offset += ow * oh * pixel_size;

	update_texsub( i, 0, 0, ow, oh, tw, th, GL_LUMINANCE, pixel_type, 
		       1, pixel_size, p );

      }
  }

  void GLQuad::bind_texture_quad( const image_type_ptr& pic,
				  const unsigned poww, const unsigned int powh )
  {
    if ( pic->format() >= image_type::kITU_601_YCbCr420 )
      return bind_texture_yuv( pic, poww, powh );

    unsigned dw = pic->width();
    unsigned dh = pic->height();

    unsigned int channels = pic->channels();
    GLenum pixel_type = gl_pixel_type( pic->pixel_type() );
    unsigned short pixel_size = pic->pixel_size();
    GLenum glformat   = gl_format( pic->format() );
    _pixels = pic->data();
  

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
        CHECK_GL( "Before glActiveTexture" );
      glActiveTexture( GL_TEXTURE0 );
      CHECK_GL( "glActiveTexture" );
    }

    glBindTexture( GL_TEXTURE_2D, _texId[0] );
    CHECK_GL( "!bind_texture glBindTexture" );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    CHECK_GL( "bind_texture glPixelStore" );


    unsigned tw = dw;
    unsigned th = dh;
    if ( GLEngine::pow2Textures() )
      {
	tw = poww;
	th = powh;
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
	if ( GLEngine::shader_type() ) _shader = GLEngine::rgbaShader();
	else                           _shader = NULL;


	if ( GLEngine::pboTextures() )
	  {
	    // Define texture level zero (without an image); notice the
	    // explicit bind to the zero pixel unpack buffer object so that
	    // pass NULL for the image data leaves the texture image
	    // unspecified.
	    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
            CHECK_GL( "glBindBuffer" );
	  }


	glTexImage2D( GL_TEXTURE_2D, 
		      0,               // level
		      _internalFormat,  // internalFormat
		      tw, th, 
		      0,               // border
		      _glformat, 
		      _pixel_type, 
		      NULL );

	CHECK_GL( "bind_texture glTexImage2D" );

      }


    // Upload converted rectangle

#if 0
    const mrv::Recti& r = img->damage_rectangle();

    unsigned int rx = r.x();
    unsigned int ry = r.y();
    unsigned int rw = r.w();

    boost::uint8_t* p = ( (boost::uint8_t*)_pixels.get() + 
			       (ry * dw + rx) * _channels * pixel_size );

    update_texsub( 0, rx, ry, rw, r.h(), tw, th, _glformat, _pixel_type, 
		   short(_channels), short(pixel_size), p );

#else
    boost::uint8_t* p = (boost::uint8_t*)_pixels.get();
    update_texsub( 0, 0, 0, dw, dh, tw, th, _glformat, _pixel_type, 
		   short(_channels), short(pixel_size), p );
#endif

  }

  /// Prepare a texture for opengl
  void GLQuad::bind_texture_pixels( const mrv::image_type_ptr& pic )
  {
    unsigned dw = pic->width();
    unsigned dh = pic->height();

    unsigned int channels = pic->channels();
    GLenum pixel_type = gl_pixel_type( pic->pixel_type() );
    GLenum glformat = gl_format( pic->format() );
    _pixels = pic->data();


    if ( _width  != dw || 
	 _height != dh || 
	 _pixel_type != pixel_type || 
	 _channels != channels ||
	 _glformat != glformat )
      {
	// @todo
	// LOG_WARNING( /* img->name() << */ ": bigger than " 
	// 	     << GLEngine::maxTexWidth() << " x "
	// 	     << GLEngine::maxTexHeight() << ".  "
	// 	     "Cannot use your gfx card for fast texture mapping." );
      }

    _width = dw;
    _height = dh;
    _channels = channels; 
    _pixel_type = pixel_type;
    _glformat = glformat;
    _format   = pic->format();
    _uvMax.u = _uvMax.v = 0.0f;
  }


  void GLQuad::bind( const image_type_ptr& pic )
  {
      CHECK_GL( "bind" );
    unsigned dw = pic->width();
    unsigned dh = pic->height();
    if ( dw <= 0 || dh <= 0 ) return;

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
    bool fullscreen = !_view->fltk_main()->border();
    GLenum filter = fullscreen ? GL_LINEAR : GL_NEAREST;

    if ( _shader && _shader != GLEngine::rgbaShader() )
      {
          short i = short(_channels - 1);
	for ( ; i >= 0 ; --i )
	  {
              short idx = (i == 3 ? (short) 4 : i ); 
	    glActiveTexture(GL_TEXTURE0 + idx);
	    glEnable(GL_TEXTURE_2D);
	    glBindTexture(GL_TEXTURE_2D, _texId[i] );
            CHECK_GL( "shader bind_texture glBindTexture" );
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	  }
      }
    else
      {
	if ( GLEW_ARB_multitexture )
	  glActiveTexture(GL_TEXTURE0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, _texId[0] );
        CHECK_GL( "no shader bind_texture glBindTexture" );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
      }


    CHECK_GL( "draw_quad enable 2D textures" );

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
	    // 	  _shader->setUniform( "lutF", _lut->lutF );
	  }
	else
	  {
	    _shader->setUniform( "enableLut", 0 );
	  }

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
	    if ( _format >= image_type::kITU_709_YCbCr420 )
	      {
		// HDTV  YCbCr
		_shader->setUniform( "Koff", 0.0f, -0.5f, -0.5f );
		_shader->setUniform( "Kr", 1.0f, 0.0f, 1.402f );
		_shader->setUniform( "Kg", 1.0f, -0.344136f, -0.714136f );
		_shader->setUniform( "Kb", 1.0f, 1.772f, 0.0f );
	      }
	    else
	      {
		// STV  YCbCr
		_shader->setUniform( "Koff", 
				    -16/255.0f, -128/255.f, -128/255.f );
		_shader->setUniform( "Kr", 1.16438355f, 0.0f, 1.59602715f );
		_shader->setUniform( "Kg", 1.16438355f, -0.3917616f, 
				    -0.81296805f );
		_shader->setUniform( "Kb", 1.16438355f, 2.01723105f, 0.0f );
	      }
	  }

	CHECK_GL( "draw_quad shader parameters" );
      }



    glBegin( GL_POLYGON );
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

    CHECK_GL( "draw_quad" );
  }


  void GLQuad::lut( const CMedia* img )
  {
    if ( _lut && img == _image ) return;

    _lut   = mrv::GLLut3d::factory( _view->main()->uiPrefs, img );
    _image = img;
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

    double sw = ((double)_view->w() - (double) dw * p) / 2.0;
    double sh = ((double)_view->h() + (double) dh * p ) / 2.0;

    double dx = (_view->offset_x() * p );
    double dy = (_view->offset_y() * p - sh );

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
          //  glRasterPos2f( dx, yp );
          // we do:
          glRasterPos2i(0, 0);
          glBitmap( 0, 0, 0, 0, float(dx), float(yp), NULL );
          //

          unsigned int r = (H-y) * dw;
          const boost::uint8_t* ptr = base + r * step;
          glDrawPixels (dw,		        // width
                        1,		        // height
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
	glDrawPixels (dw,		        // width
		      1,		        // height
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

    CHECK_GL("drawField");
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
	glDrawPixels (dw,		        // width
		      1,		        // height
		      _glformat,	// format
		      _pixel_type,	// type
		      ptr );
      }
  }


  void GLQuad::draw( const unsigned dw, const unsigned dh ) const
  {
    if ( _uvMax.u > 0.0f )
      draw_quad( dw, dh );
    else
      draw_pixels( dw, dh );
  }

}
