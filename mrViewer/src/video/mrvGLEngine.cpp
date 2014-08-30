/**
 * @file   mrvGLEngine.cpp
 * @author gga
 * @date   Fri Jul 13 23:47:14 2007
 * 
 * @brief  OpenGL engine
 *
 */

//
// Set these for testing some features as if using a lower gfx card
//
// #define TEST_NO_SHADERS  // test without hardware shaders
// #define TEST_NO_YUV      // test in rgba mode only

#define USE_NV_SHADERS
#define USE_OPENGL2_SHADERS
#define USE_ARBFP1_SHADERS

#include <vector>
#include <iostream>
#include <sstream>


#if defined(WIN32) || defined(WIN64)
#  include <winsock2.h>  // to avoid winsock issues
#  include <windows.h>
#  undef min
#  undef max
#endif

#if defined(WIN32) || defined(WIN64)
#  include <fltk/win32.h>   // for fltk::getDC()
#endif

#include <GL/glew.h>

#if defined(WIN32) || defined(WIN64)
#  include <GL/wglew.h>
#elif defined(LINUX)
#  include <GL/glxew.h>
#endif

#include <GL/glu.h>
#include <GL/glut.h>




#include <half.h>
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImfStringAttribute.h>
#include <Iex.h>
#include <CtlExc.h>
#include <halfLimits.h>

#include "core/ctlToLut.h"
#include "core/mrvThread.h"
#include "core/mrvRectangle.h"

#include "gui/mrvIO.h"
#include "gui/mrvImageView.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvPreferences.h"
#include "mrViewer.h"

#include "video/mrvGLShape.h"
#include "video/mrvGLShader.h"
#include "video/mrvGLEngine.h"
#include "video/mrvGLQuad.h"
#include "video/mrvGLLut3d.h"


namespace 
{
  const char* kModule = N_("opengl");
}


namespace fltk {

#ifdef WIN32
  extern HINSTANCE	xdisplay;
#else
  extern Display*	xdisplay;
#endif

}



#ifdef DEBUG
#  define CHECK_GL(x) handle_gl_errors( N_( x ) )
#else
#  define CHECK_GL(x)
#endif

namespace mrv {

  typedef CMedia::Mutex Mutex;

  GLShader* GLEngine::_rgba   = NULL;
  GLShader* GLEngine::_YCbCr  = NULL;
  GLShader* GLEngine::_YCbCrA = NULL;
  GLShader* GLEngine::_YByRy  = NULL;
  GLShader* GLEngine::_YByRyA = NULL;

  GLint  GLEngine::_maxTexUnits     = 1;
  bool   GLEngine::_floatTextures   = false;
  bool   GLEngine::_halfTextures    = false;
  bool   GLEngine::_pow2Textures    = true;
  bool   GLEngine::_pboTextures     = false;
  bool   GLEngine::_sdiOutput       = false;
  bool   GLEngine::_fboRenderBuffer = false;

  GLuint GLEngine::sCharset = 0;   // display list for characters
  unsigned int GLEngine::_maxTexWidth;
  unsigned int GLEngine::_maxTexHeight;


  //
  // Check for opengl errors and print function name where it happened.
  //
  void GLEngine::handle_gl_errors(const char* where)
  {
      GLenum error = glGetError();
      if ( error == GL_NO_ERROR ) return;

      while (error != GL_NO_ERROR)
      {
          LOG_ERROR( where << _(": Error ") << error << " " <<
                     gluErrorString(error) );
          error = glGetError();
      }

      exit(1);
  }



  std::string GLEngine::options()
  {
    using std::endl;
    std::ostringstream o;	

    char* vendorString = (char*)glGetString(GL_VENDOR);
    if ( !vendorString ) vendorString = (char*)_("Unknown");

    char* rendererString = (char*)glGetString(GL_RENDERER);
    if ( !rendererString ) rendererString = (char*)_("Unknown");

    char* versionString = (char*)glGetString(GL_VERSION);
    if ( !versionString ) versionString = (char*)_("Unknown");

    o << _("Vendor:\t") << vendorString << endl
      << _("Renderer:\t") << rendererString << endl
      << _("Version:\t")  << versionString << endl
      << _("Hardware Shaders:\t") << shader_type_name() << endl
      << _("PBO Textures:\t") << (_pboTextures   ? _("Yes") : _("No")) << endl
      << _("Float Textures:\t") << (_floatTextures ? _("Yes") : _("No")) << endl
      << _("Half Textures:\t") << (_halfTextures  ? _("Yes") : _("No")) << endl
      << _("Non-POT Textures:\t") 
      << (_pow2Textures  ? _("No")  : _("Yes")) << endl
      << _("Max. Texture Size:\t") 
      << _maxTexWidth << N_(" x ") << _maxTexHeight << endl
      << _("Texture Units:\t") << _maxTexUnits << endl
      << _("YUV  Support:\t") << (supports_yuv() ? _("Yes") : _("No")) << endl
      << _("YUVA Support:\t") << (supports_yuva() ? _("Yes") : _("No")) << endl
      << _("SDI Output:\t") << (_sdiOutput ? _("Yes") : _("No")) << endl;
    return o.str();
  }



void GLEngine::init_charset()
{
  unsigned numChars = 255;
  int fontsize = 16;

#ifdef WIN32
    HDC   hDC = fltk::getDC();
    HGLRC hRC = wglGetCurrentContext();
    if (hRC == NULL ) hRC = wglCreateContext( hDC );

    LOGFONT     lf;
    memset(&lf,0,sizeof(LOGFONT));
    lf.lfHeight               =   -fontsize ;
    lf.lfWeight               =   FW_NORMAL ;
    lf.lfCharSet              =   ANSI_CHARSET ;
    lf.lfOutPrecision         =   OUT_RASTER_PRECIS ;
    lf.lfClipPrecision        =   CLIP_DEFAULT_PRECIS ;
    lf.lfQuality              =   DRAFT_QUALITY ;
    lf.lfPitchAndFamily       =   FF_DONTCARE|DEFAULT_PITCH;
    lstrcpy (lf.lfFaceName, N_("Helvetica") ) ;


    HFONT    fid = CreateFontIndirect(&lf);
    HFONT oldFid = (HFONT)SelectObject(hDC, fid);

    sCharset = glGenLists( numChars );

    wglMakeCurrent( hDC, hRC );
    wglUseFontBitmaps(hDC, 0, numChars-1, sCharset);

    SelectObject(hDC, oldFid);
#else
    // Find Window's default font
    Display* gdc = fltk::xdisplay;

    // Load XFont to user's specs
    char font_name[256];
    sprintf( font_name, N_("-*-fixed-*-r-normal--%d-0-0-0-c-0-iso8859-1"),
	     fontsize );
    XFontStruct* hfont = XLoadQueryFont( gdc, font_name );
    if (!hfont) {
       LOG_ERROR( _("Could not open any font of size ") << fontsize);
       return;
    }

    // Create GL lists out of XFont
    sCharset = glGenLists( numChars );
    glXUseXFont(hfont->fid, 0, numChars-1, sCharset);

    // Free font and struct
    XFreeFont( gdc, hfont );
#endif

  CHECK_GL("init_charset");
}



/** 
 * Initialize opengl textures
 *
 */
void GLEngine::init_textures()
{
  // Get maximum texture resolution for gfx card
  GLint glMaxTexDim;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);
  CHECK_GL("init_textures get max texture size");

#ifndef TEST_NO_PBO_TEXTURES // test not using pbo textures
  _pboTextures = ( GLEW_ARB_pixel_buffer_object != GL_FALSE );
#endif

  _has_yuv = false;

  _maxTexUnits = 1;
  if ( GLEW_ARB_multitexture )
    {
#ifndef TEST_NO_YUV
      glGetIntegerv(GL_MAX_TEXTURE_UNITS, &_maxTexUnits);
      CHECK_GL("init_textures get max tex units");

      if ( _maxTexUnits >= 4 )  _has_yuv = true;
#endif
    }

  _maxTexWidth = _maxTexHeight = glMaxTexDim;

}

/**
 * Initialize GLEW features
 *
 */
void GLEngine::init_GLEW()
{
  GLenum err = glewInit();
  if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        LOG_ERROR( _("GLEW Initialize Error: ") << glewGetErrorString(err) );
        exit(1);
    }

// #if defined(WIN32) || defined(WIN64)
//   err = wglewInit();
//   if (GLEW_OK != err)
//     {
//       /* Problem: wglewInit failed, something is seriously wrong. */
//       std::cerr << "WGLEW Initialize Error: " 
// 		<< glewGetErrorString(err) << std::endl;
//       exit(1);
//     }
// #else
//   err = glxewInit();
//   if (GLEW_OK != err)
//     {
//       /* Problem: glxewInit failed, something is seriously wrong. */
//       std::cerr << "GLXEW Initialize Error: " 
// 		<< glewGetErrorString(err) << std::endl;
//       exit(1);
//     }
// #endif
}


/** 
 * Initialize OpenGL context, textures, and get opengl features for
 * this gfx card.
 * 
 */
void GLEngine::initialize()
{
  init_GLEW();

  init_charset();

  init_textures();

  int argc = 1;
  static char* args[] = { (char*)"GlEngine", NULL };
  glutInit( &argc, args );

// #if defined(WIN32) || defined(WIN64)
//   if ( WGLEW_WGL_swap_control )
//     {
//       std::cerr << "swap control vsync? "
// 		<< wglGetSwapIntervalEXT() << std::endl;
//     }
// #else
//   if ( GLX_SGI_video_sync )
//     {
//     }
// #endif


#if defined(WIN32) || defined(WIN64)
  if ( wglewIsSupported( N_("WGL_NV_video_out") ) )
    {
      _sdiOutput = true;
    }
#else
  if ( glxewIsSupported( N_("GLX_NV_video_out") ) ||
       glxewIsSupported( N_("GLX_NV_video_output") ) )
    {
      _sdiOutput = true;
    }
#endif

  if ( _hardwareShaders == kAuto )
    {
      _hardwareShaders = kNone;
#ifndef TEST_NO_SHADERS

#ifdef USE_ARBFP1_SHADERS
      if ( GLEW_ARB_fragment_program ) 
	_hardwareShaders = kARBFP1;
#endif

#ifdef USE_OPENGL2_SHADERS
      if ( GLEW_VERSION_2_0 ) 
	_hardwareShaders = kGLSL;
#endif

#ifdef USE_NV_SHADERS
      if ( GLEW_NV_fragment_program )
	_hardwareShaders = kNV30;
#endif

      LOG_INFO( _("Using hardware shader profile: ") << shader_type_name() );
#endif

    }

  if ( _hardwareShaders != kNone )
    {
      std::string directory;

      if ( _has_yuv )
	{
	  _has_yuva = false;
	  if ( _maxTexUnits > 4 )
	    {
	      _has_yuva = true;
	    }
	}


      const char* env = getenv( N_("MRV_SHADER_PATH") );
      if ( !env )
	{
	  env = getenv( N_("MRV_ROOT") );
	  if ( env )
	    {
	      directory = env;
	      directory += N_("/shaders");
	    }
	}
      else
	{
	  directory = env;
	}

      if ( !directory.empty() )
	{
	  char shaderFile[256];

	  try 
	    {
	      const char* ext = NULL;
	      switch( _hardwareShaders )
		{
		case kNV30:
		  ext = N_("fp30"); break;
		case kGLSL:
		  ext = N_("glsl"); break;
		case kARBFP1:
		  ext = N_("arbfp1"); break;
		default:
		  break;
		}

	      const char* dir = directory.c_str();

	      sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("rgba"), ext );
	      _rgba = new GLShader( shaderFile );


	      if ( _has_yuv )
		{
		  sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("YCbCr"), ext );
		  _YCbCr = new GLShader( shaderFile );
		      
		  sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("YByRy"), ext );
		  _YByRy = new GLShader( shaderFile );
		}

	      if ( _has_yuva )
		{
		  sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("YCbCrA"), ext );
		  _YCbCrA = new GLShader( shaderFile );

		  sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("YByRyA"), ext );
		  _YByRyA = new GLShader( shaderFile );
		}

	    }
	  catch ( const std::exception& e )
	    {
	      LOG_ERROR( e.what() );
	      directory.clear();
	      _has_yuv  = false;
	      _has_yuva = false;
	    }
	}
      else
	{
	  LOG_WARNING( _("Environment variable MRV_SHADER_PATH not found, "
			  "using built-in shader.") );
	}

      if ( directory.empty() )
	{
	  directory = N_(".");
	  loadBuiltinFragShader();
	}

    }
  else
    {
      LOG_INFO( _("Hardware shaders not available.") );
      _has_yuv  = false;
      _has_yuva = false;
    }

  if ( _has_yuv )
    {
      if ( _has_yuva )
	{
	  LOG_INFO( _("mrViewer supports YUVA images through shaders.") );
	}
      else
	{
	  LOG_INFO( _("mrViewer supports YUV images through shaders.") );
	}
    }
  else
    {
      LOG_INFO( _("mrViewer does not support YUV images.") );
    }

  _floatTextures     = ( GLEW_ARB_color_buffer_float != GL_FALSE );
  _halfTextures      = ( GLEW_ARB_half_float_pixel != GL_FALSE );
  _pow2Textures      = !GLEW_ARB_texture_non_power_of_two;
  _fboRenderBuffer   = ( GLEW_ARB_framebuffer_object != GL_FALSE );

  alloc_quads( 4 );

  CHECK_GL("initGL");
}


/** 
 * Resets the view matrix and sets the projection to match the window's viewport
 * 
 */
void GLEngine::reset_view_matrix()
{
  glMatrixMode(GL_PROJECTION);

  ImageView* view = const_cast< ImageView* >( _view );
  view->ortho();
  
  // Makes gl a tad faster
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
}

void GLEngine::refresh_luts()
{
  QuadList::iterator q = _quads.begin();
  QuadList::iterator e = _quads.end();
  for ( ; q != e; ++q )
    {
      (*q)->clear_lut();
    }
}

/** 
 * Clears the opengl canvas to a certain color
 * 
 * @param r red component
 * @param g green component
 * @param b blue component
 * @param a alpha component
 */
void GLEngine::clear_canvas( float r, float g, float b, float a )
{
  glClearColor(r, g, b, a );
  glClear( GL_COLOR_BUFFER_BIT );
  glShadeModel( GL_FLAT );
  CHECK_GL( "Clear canvas" );
}

void GLEngine::set_blend_function( int source, int dest )
{
  // So compositing works properly
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc( (GLenum) source, (GLenum) dest );
  CHECK_GL( "glBlendFunc" );
}

void GLEngine::color( uchar r, uchar g, uchar b, uchar a = 255 )
{
  glColor4ub( r, g, b, a );
}

void GLEngine::color( float r, float g, float b, float a = 1.0 )
{
  glColor4f( r, g, b, a );
}

bool GLEngine::init_fbo( ImageList& images )
{
   if ( ! _fboRenderBuffer ) return false;

   glGenTextures(1, &textureId);
   glBindTexture(GL_TEXTURE_2D, textureId);
   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   GLenum internalFormat = GL_RGBA32F_ARB;
   GLenum dataFormat = GL_RGBA;
   GLenum pixelType = GL_FLOAT;

   Image_ptr img = images.back();

   mrv::image_type_ptr pic = img->hires();

   unsigned w = pic->width();
   unsigned h = pic->height();

   glTexImage2D(GL_TEXTURE_2D, 
		0, // level
		internalFormat, // internal format
		w, h, 
		0, // border
		dataFormat,  // texture data format
		pixelType, // texture pixel type 
		NULL);    // texture pixel data
   CHECK_GL( "glTexImage2D" );

   glGenFramebuffers(1, &id);
   glBindFramebuffer(GL_FRAMEBUFFER, id);
   CHECK_GL( "glBindFramebuffer" );

   glGenRenderbuffers(1, &rid);
   glBindRenderbuffer( GL_RENDERBUFFER, rid );
   CHECK_GL( "glBindRenderbuffer" );



   if ( w > GL_MAX_RENDERBUFFER_SIZE ) return false;
   if ( h > GL_MAX_RENDERBUFFER_SIZE ) return false;

   glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_STENCIL, 
			  w, h );
   glBindRenderbuffer( GL_RENDERBUFFER, 0 );
   CHECK_GL( "glBindRenderbuffer" );
 
   // attach a texture to FBO color attachement point
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
    			  GL_TEXTURE_2D, textureId, 0);
   CHECK_GL( "glFramebufferTexture2D" );

   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
			     GL_RENDERBUFFER, rid);
   CHECK_GL( "glFramebufferRenderbuffer depth" );

   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 
			     GL_RENDERBUFFER, rid);
   CHECK_GL( "glFramebufferRenderbuffer stencil" );

   GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
   if ( status != GL_FRAMEBUFFER_COMPLETE )
   {
      switch( status )
      {
	 case GL_FRAMEBUFFER_UNSUPPORTED:
	    LOG_ERROR( _("Unsupported internal format") );
	    return false;
	 case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	    LOG_ERROR( _("Framebuffer incomplete: Attachment is NOT complete.") );
	    return false;
	    
	 case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	    LOG_ERROR( _("Framebuffer incomplete: No image is attached to FBO.") );
	    return false;
	 case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
	    LOG_ERROR( _("Framebuffer incomplete: Draw buffer." ) );
	    return false;

	 case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
	    LOG_ERROR( _("Framebuffer incomplete: Read buffer.") );
	    return false;
      }
   }
   return true;
}

void GLEngine::end_fbo( ImageList& images )
{
   if ( ! _fboRenderBuffer ) return;

   glBindTexture(GL_TEXTURE_2D, textureId);
   CHECK_GL( "end_fbo glBindTexture" );

   Image_ptr img = images.back();
   mrv::image_type_ptr pic = img->hires();

   unsigned w = pic->width();
   unsigned h = pic->height();
   glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
   CHECK_GL( "glCopyTexSubImage2D" );
   glBindTexture(GL_TEXTURE_2D, 0);
   CHECK_GL( "glBindTexture 0" );

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   CHECK_GL( "glBindFramebuffer" );
   glDeleteFramebuffers(1, &id);
   CHECK_GL( "glDeleteFramebuffers" );
   glDeleteRenderbuffers(1, &rid);
   CHECK_GL( "glDeleteRenderbuffers" );

}

void GLEngine::draw_title( const float size,
			   const int y, const char* text )
{
  if ( !text ) return;

  void* font = GLUT_STROKE_MONO_ROMAN;

  glPushMatrix();
  glLoadIdentity();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  
  glLineWidth(4.0);

  float sum = 0.0f;
  for (const char* p = text; *p; ++p)
    sum += glutStrokeWidth( font, *p );

  float x = ( _view->w() - sum * size ) / 2.0f;

  float rgb[4];
  glGetFloatv( GL_CURRENT_COLOR, rgb );

  glColor4f( 0.f, 0.f, 0.f, 1.0f );
  glLoadIdentity();
  glTranslatef( x, GLfloat( y ), 0 );
  glScalef( size, size, 1.0 );
  for (const char* p = text; *p; ++p)
    glutStrokeCharacter( font, *p );

  glColor4f( rgb[0], rgb[1], rgb[2], rgb[3] );
  glLoadIdentity();
  glTranslatef( x-2, float(y+2), 0 );
  glScalef( size, size, 1.0 );
  for (const char* p = text; *p; ++p)
    glutStrokeCharacter( font, *p );

  glPopMatrix();

  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);
  glLineWidth(1.0);
}

/** 
 * Draw a line of text at a raster position
 * 
 * @param x raster x pos
 * @param y raster y pos
 * @param s line of text to write
 */
void GLEngine::draw_text( const int x, const int y, const char* s )
{
  glLoadIdentity();
  glRasterPos2s( x, y );

  glPushAttrib(GL_LIST_BIT);
  if ( sCharset )
  {
     glListBase(sCharset);
     glCallLists( GLsizei( strlen(s) ), GL_UNSIGNED_BYTE, s);
  }
  glPopAttrib();
}

void GLEngine::draw_cursor( const double x, const double y )
{ 
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();
   
   double pr = 1.0;
   if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();

   double zoomX = _view->zoom();
   double zoomY = _view->zoom();

   double tw = texWidth  / 2.0f;
   double th = texHeight / 2.0f;

   double sw = ((double)_view->w() - texWidth  * zoomX) / 2;
   double sh = ((double)_view->h() - texHeight * zoomY) / 2;

   glTranslated(_view->offset_x() * zoomX + sw, 
		_view->offset_y() * zoomY + sh, 0);
   glTranslated(tw * zoomX, th * zoomY, 0);
   
   glScaled(zoomX, zoomY * pr, 1.0);

   glColor4f( 1, 0, 0, 1 );

   glPointSize( float(_view->main()->uiPaint->uiPenSize->value()) );

   glBegin( GL_POINTS );
   glVertex2d( x, y );
   glEnd();
}

void GLEngine::draw_square_stencil( const int x, const int y, 
                                    const int x2, const int y2)
{

  glClear( GL_STENCIL_BUFFER_BIT );
  CHECK_GL( "glClear STENCIL_BUFFER" );
  glColorMask(true, true, true, true);
  CHECK_GL( "draw_square_stencil glColorMask" );
  glDepthMask(false);
  CHECK_GL( "draw_square_stencil glDepthMask" );
  glColor4f( 0.0f, 0.0f, 0.0f, 0.0f );
  glEnable( GL_STENCIL_TEST );
  CHECK_GL( "draw_square_stencil glEnable Stencil test" );
  glStencilFunc( GL_ALWAYS, 0x1, 0xffffffff );
  CHECK_GL( "draw_square_stencil glStencilFunc" );
  glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
  CHECK_GL( "draw_square_stencil glStencilOp" );

  glPushMatrix();

  double pr = 1.0;
  if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();
  glScaled( 1.0, pr, 1.0 );




  double W = (x2-x+1);
  double H = (y2-y+1);

  glTranslated( x, -y, 0 );


  //
  // Draw mask
  //
  glBegin( GL_POLYGON );
  {
      glVertex2d(0, 0);
      glVertex2d(W, 0);
      glVertex2d(W, -H);
      glVertex2d(0, -H);
  }
  glEnd();

  glPopMatrix();

  // just draw where inside of the mask
  glStencilFunc(GL_EQUAL, 0x1, 0xffffffff);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

/** 
 * Draws the mask
 * 
 * @param pct 
 */
void GLEngine::draw_mask( const float pct )
{
  mrv::media fg = _view->foreground();
  if ( !fg ) return;



  Image_ptr img = fg->image();

  mrv::Recti dpw2 = img->display_window2();
  mrv::Recti dpw = img->display_window();

  if ( dpw.w() == 0 )
  {
      dpw.w( img->width() );
  }

  if ( dpw.h() == 0 )
  {
      dpw.h( img->height() );
  }

  if ( img->stereo_type() & CMedia::kStereoSideBySide )
  {
      dpw.w( dpw.w() + dpw2.w() );
  }

  glColor3f( 0.0f, 0.0f, 0.0f );
  glDisable( GL_STENCIL_TEST );
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated( double(_view->w())/2, double(_view->h())/2, 0 );
  glScalef( _view->zoom(), _view->zoom(), 1.0f);
  glTranslated( _view->offset_x(), _view->offset_y(), 0.0 );
  double pr = 1.0;
  if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();
  glScaled( dpw.w(), dpw.h() * pr, 1.0 );
  glTranslated( 0.5, -0.5, 0.0 );

  double aspect = (double) dpw.w() / (double) dpw.h();   // 1.3
  double target_aspect = 1.0 / pct;
  double amount = (0.5 - target_aspect * aspect / 2);

  //
  // Bottom mask
  //
  glBegin( GL_POLYGON );
  {
    glVertex3d( -0.5,  -0.5 + amount, 0. );
    glVertex3d(  0.5,  -0.5 + amount, 0. );
    glVertex3d(  0.5,  -0.5, 0. );
    glVertex3d( -0.5,  -0.5, 0. );
  }
  glEnd();

  //
  // Top mask
  // //
  // glRectd( -0.5, 0.5 - amount, 0.5, 0.5 );
  glBegin( GL_POLYGON );
  {
    glVertex3d( -0.5,  0.5, 0. );
    glVertex3d(  0.5,  0.5, 0. );
    glVertex3d(  0.5,  0.5 - amount, 0. );
    glVertex3d( -0.5,  0.5 - amount, 0. );
  }
  glEnd();


}

/** 
 * Draw an overlay rectangle (like selection)
 * 
 * @param r rectangle to draw
 */
void GLEngine::draw_rectangle( const mrv::Rectd& r )
{

    mrv::media fg = _view->foreground();
    if (!fg) return;

    Image_ptr img = fg->image();

    mrv::Recti daw = img->data_window();
    mrv::Recti dpw = img->display_window();

    if ( dpw.w() == 0 )
    {
        dpw.w( img->width() );
        dpw.h( img->height() );
    }


    glPushMatrix();
    glPushAttrib( GL_STENCIL_TEST );
    glDisable( GL_STENCIL_TEST );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated( double(_view->w())/2, double(_view->h())/2, 0 );
    glScaled( _view->zoom(), _view->zoom(), 1.0);
    glTranslated( _view->offset_x(), _view->offset_y(), 0.0 );
    double pr = 1.0;
    if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();
    glScaled( 1.0, pr, 1.0 );

    glTranslated( r.x(), -r.y(), 0 );

    double rw = r.w();
    double rh = r.h();

    glLineWidth( 1.0 );

    glBegin(GL_LINE_LOOP);

    glVertex2i(0,  0);
    glVertex2i(rw, 0);
    glVertex2i(rw, -rh);
    glVertex2i(0,  -rh);

    glEnd();

    glPopAttrib();
    glPopMatrix();
}

void GLEngine::draw_safe_area_inner( const double tw, const double th,
                                     const char* name )
{
  glLineWidth( 1.0 );

  glBegin(GL_LINE_LOOP);

  glVertex2d(-tw,-th);
  glVertex2d(tw, -th);
  glVertex2d(tw,  th);
  glVertex2d(-tw, th);

  glEnd();

  if ( name )
    {
      glPushMatrix();
      glTranslated(tw+5, th, 0);
      glScalef( 0.1f, 0.1f, 1.0f );
      for (const char* p = name; *p; ++p)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
      glPopMatrix();
    }

}

/** 
 * Draw an unfilled rectangle (for safe area display)
 * 
 * @param percentX  percentage in X of area 
 * @param percentY  percentage in Y of area
 */
void GLEngine::draw_safe_area( const double percentX, const double percentY,
			       const char* name )
{

    mrv::media fg = _view->foreground();
    if (!fg) return;

    Image_ptr img = fg->image();

    mrv::Recti dpw = img->display_window();
    if ( dpw.w() == 0 )
    {
        dpw = img->data_window();
    }
    
    if ( dpw.w() == 0 )
    {
        dpw.w( img->width() );
        dpw.h( img->height() );
    }


  glDisable( GL_STENCIL_TEST );

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslated( double(_view->w())/2, double(_view->h())/2, 0 );
  glScaled( _view->zoom(), _view->zoom(), 1.0);
  glTranslated( _view->offset_x(), _view->offset_y(), 0.0 );
  double pr = 1.0;
  if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();
  glScaled( 1.0, pr, 1.0 );

  double tw = dpw.w() / 2.0;
  double th = dpw.h() / 2.0;

  glTranslated( tw, -th, 0 );

  tw *= percentX;
  th *= percentY;

  draw_safe_area_inner( tw, th, name );

  if ( _view->stereo_type() & CMedia::kStereoSideBySide )
  {
      glTranslated( dpw.w(), 0, 0 );
      draw_safe_area_inner( tw, th, name );
  }

}




void GLEngine::alloc_quads( size_t num )
{
  size_t num_quads = _quads.size();
  _quads.reserve( num );
  for ( size_t q = num_quads; q < num; ++q )
    {
      mrv::GLQuad* quad = new mrv::GLQuad( _view );
      _quads.push_back( quad );
    }
}


void GLEngine::translate( double x, double y )
{
   glTranslated( x, y, 0 );
}

void GLEngine::draw_images( ImageList& images )
{
    CHECK_GL("draw_images");

  // Check if lut types changed since last time
  static int  RT_lut_old_algorithm = Preferences::kLutPreferCTL;
  static int ODT_lut_old_algorithm = Preferences::kLutPreferCTL;
  static int LUT_quality           = 2;
  static std::string ODT_ICC_old_profile;

  if ( _view->use_lut() )
    {
      mrv::PreferencesUI* uiPrefs = _view->main()->uiPrefs;
      int RT_lut_algorithm = uiPrefs->RT_algorithm->value();
      int ODT_lut_algorithm = uiPrefs->ODT_algorithm->value();
      const char* ODT_ICC_profile = uiPrefs->uiODT_ICC_profile->text();
      int lut_quality = uiPrefs->uiLUT_quality->value();

      // Check if there was a change effecting lut.
      if ( ( RT_lut_algorithm != RT_lut_old_algorithm ) ||
	   ( ODT_lut_algorithm != ODT_lut_old_algorithm ) ||
	   ( ODT_ICC_old_profile != ODT_ICC_profile ) ||
	   ( LUT_quality != lut_quality ) )
	{
	  RT_lut_old_algorithm = RT_lut_algorithm;
	  ODT_lut_old_algorithm = ODT_lut_algorithm;
	  if ( ODT_ICC_profile )
	    ODT_ICC_old_profile = ODT_ICC_profile;
	  else
	    ODT_ICC_old_profile.clear();

	  refresh_luts();

	  if ( LUT_quality != lut_quality )
	    {
	      mrv::GLLut3d::clear();
	    }
	}

    }


  float normMin = 0.0f, normMax = 1.0f;
  if ( _view->normalize() )
  {
      minmax(); // calculate min-max
      minmax( normMin, normMax ); // retrieve them
  }


  size_t num_quads = 0;
  ImageList::iterator i = images.begin();
  ImageList::iterator e = images.end();

  for ( ; i != e; ++i )
    {
      const Image_ptr& img = *i;
      if ( img->has_subtitle() ) ++num_quads;
      if ( img->has_picture()  ) ++num_quads;
      if ( img->is_stereo() )    ++num_quads;
    }


  CHECK_GL( "glPrealloc quads GL_BLEND" );

  size_t num = _quads.size();
  if ( num_quads > num )
    {
      alloc_quads( num_quads );
    }



  glColor4f(1.0f,1.0f,1.0f,1.0f);



  QuadList::iterator q = _quads.begin();

  assert( q != _quads.end() );

  i = images.begin();
  e = images.end();

  Image_ptr fg = images.back();

  glDisable( GL_BLEND );
  CHECK_GL( "glDisable GL_BLEND" );

  for ( i = images.begin() ; i != e; ++i, ++q )
    {

      const Image_ptr& img = *i;
      mrv::image_type_ptr pic = img->hires();

      const boost::int64_t& frame = pic->frame();

      mrv::Recti& dpw = img->display_window(frame);
      mrv::Recti& daw = img->data_window(frame);

      if ( img->stereo_type() == CMedia::kStereoCrossed )
      {
          dpw = img->display_window2(frame);
          daw = img->data_window2(frame);
      }


      if ( fg != img )
      {
          const mrv::Recti& dp = fg->display_window(frame);
          texWidth = dp.w();
          texHeight = dp.h();
          if ( dpw == dp )
          {
              texWidth = pic->width();
              texHeight = pic->height();
          }

          if ( texWidth == 0 )  texWidth = pic->width();
          if ( texHeight == 0 ) texHeight = pic->height();
      }
      else
      {
          if ( daw.w() != 0 )
          {
              texWidth = daw.w();
              texHeight = daw.h();
          }
          else
          {
              texWidth = pic->width();
              texHeight = pic->height();
          }
      }


      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslated( double(_view->w())/2.0, double(_view->h())/2.0, 0.0 );
      glScaled( _view->zoom(), _view->zoom(), 1.0);
      glTranslated( _view->offset_x(), _view->offset_y(), 0.0 );



      if ( _view->display_window() && dpw != daw )
      {
          draw_square_stencil( dpw.l(), dpw.t(), dpw.r(), dpw.b() );
      }

      if ( _view->data_window() && daw != dpw )
      {
          mrv::Rectd r = mrv::Rectd( daw.x(), daw.y(), daw.w(), daw.h() );
          glColor4f( 0.5f, 0.5f, 0.5f, 0.0f );
          glLineStipple( 1, 0x00FF );
          glEnable( GL_LINE_STIPPLE );
          draw_rectangle( r );
          glDisable( GL_LINE_STIPPLE );
          if ( _view->display_window() )
              glEnable( GL_STENCIL_TEST );
      }

      glPushMatrix();

      glTranslatef( float(daw.x()), float(-daw.y()), 0 );

      if ( _view->main()->uiPixelRatio->value() )
          glScaled( double(texWidth), double(texHeight) / _view->pixel_ratio(),
                    1.0 );
      else
          glScaled( double(texWidth), double(texHeight), 1.0 );

      glTranslated( 0.5, -0.5, 0 );


 
      GLQuad* quad = *q;
      quad->minmax( normMin, normMax );

      if ( _view->use_lut() )
	{
	  if ( img->image_damage() & CMedia::kDamageLut )
	    {
	      quad->clear_lut();
	    }

	  quad->lut( img );

          if ( img->is_stereo() && (img->stereo_type() & 
                                    CMedia::kStereoSideBySide) )
          {
             (*(q+1))->lut( img );
          }
	}

      if ( i+1 == e ) wipe_area();

      if ( img->is_stereo() && (img->stereo_type() & 
                                CMedia::kStereoSideBySide) && 
           img->left() && img->right() )
      {
         if ( img->stereo_type() == CMedia::kStereoCrossed )
         {
            pic = img->right();
         }
         else
         {
            pic = img->left();
         }

         if ( pic )
         {
            quad->bind( pic );
            quad->gamma( img->gamma() );
            quad->draw( texWidth, texHeight );
         }
         
         ++q;
         quad = *q;

         if ( img->stereo_type() == CMedia::kStereoCrossed )
         {
            pic = img->left();
         }
         else
         {
            pic = img->right();
         }

         if ( pic )
         {
            quad->bind( pic );
         }


         glPopMatrix();

         glTranslated( dpw.w(), 0, 0 );

         mrv::Recti& dpw2 = img->display_window2(frame);
         mrv::Recti& daw2 = img->data_window2(frame);
         if ( img->stereo_type() == CMedia::kStereoCrossed )
         {
             dpw2 = img->display_window(frame);
             daw2 = img->data_window(frame);
         }


         glPushMatrix();




         if ( _view->display_window() && dpw2 != daw2 )
         {
             draw_square_stencil( dpw.l(), dpw.t(), dpw.r(), dpw.b() );
         }

         if ( _view->data_window() && daw2 != dpw2 )
         {
             mrv::Rectd r = mrv::Rectd( daw2.x()+dpw.w(), 
                                        daw2.y(), daw2.w(), daw2.h() );
             glColor4f( 0.5f, 0.5f, 0.5f, 0.0f );
             glLineStipple( 1, 0x00FF );
             glEnable( GL_LINE_STIPPLE );
             draw_rectangle( r );
             glDisable( GL_LINE_STIPPLE );
             if ( _view->display_window() ) glEnable( GL_STENCIL_TEST );
         }

         if ( daw2.w() != 0 )
         {
             texWidth = daw2.w();
             texHeight = daw2.h();
         }
         else
         {
             texWidth = pic->width();
             texHeight = pic->height();
         }

         glTranslatef( float(daw2.x()), float(-daw2.y()), 0 );

         if ( _view->main()->uiPixelRatio->value() )
             glScaled( double(texWidth), 
                       double(texHeight) / _view->pixel_ratio(),
                       1.0 );
         else
             glScaled( double(texWidth), double(texHeight), 1.0 );

         glTranslated( 0.5, -0.5, 0 );

      }
      else if ( img->hires() &&
                ( img->image_damage() & CMedia::kDamageContents ||
                  img->has_subtitle() ) )
	{
	   pic = img->hires();

	   if ( shader_type() == kNone && img->stopped() && 
	        pic->pixel_type() != image_type::kByte )
	   {
	      pic = display( pic, img );
	   }

           quad->bind( pic );
	}


      quad->gamma( img->gamma() );
      quad->draw( texWidth, texHeight );

      glEnable( GL_BLEND );

      if ( img->has_subtitle() )
	{
           image_type_ptr sub = img->subtitle();
           if ( sub )
           {
	      ++q;

	      quad->bind( sub );
	      quad->draw( texWidth, texHeight );
           }
	}

      glPopMatrix();

      img->image_damage( img->image_damage() & 
			 ~(CMedia::kDamageContents | CMedia::kDamageLut |
			   CMedia::kDamageSubtitle) );

    }

  glDisable( GL_SCISSOR_TEST );
  glDisable( GL_BLEND );
}



void GLEngine::draw_annotation( const GLShapeList& shapes )
{
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();

   double pr = 1.0;
   if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();

   double zoomX = _view->zoom();
   double zoomY = _view->zoom();

   double tw = texWidth  / 2.0f;
   double th = texHeight / 2.0f;

   double sw = ((double)_view->w() - texWidth  * zoomX) / 2;
   double sh = ((double)_view->h() - texHeight * zoomY) / 2;

   glTranslated(_view->offset_x() * zoomX + sw, 
		_view->offset_y() * zoomY + sh, 0);
   glTranslated(tw * zoomX, th * zoomY, 0);
   
   glScaled(zoomX, zoomY * pr, 1.0f);

   
   glClear(GL_STENCIL_BUFFER_BIT);

   glEnable( GL_STENCIL_TEST );

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glEnable( GL_LINE_SMOOTH );
   glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

   GLShapeList::const_reverse_iterator i = shapes.rbegin();
   GLShapeList::const_reverse_iterator e = shapes.rend();

   for ( ; i != e; ++i )
   {

      if ( _view->ghost_previous() )
      {
	 if ( (*i)->frame == _view->frame() - 1 )
	 {
	    float a = (*i)->a;
	    (*i)->a *= 0.25f;
	    (*i)->draw(zoomX);
	    (*i)->a = a;
            continue;
	 }
      }

      if ( _view->ghost_next() )
      {
	 if ( (*i)->frame == _view->frame() + 1 )
	 {
	    float a = (*i)->a;
	    (*i)->a *= 0.25f;
	    (*i)->draw(zoomX);
	    (*i)->a = a;
            continue;
	 }
      }

      if ( (*i)->frame == MRV_NOPTS_VALUE ||
	   (*i)->frame == _view->frame() )
      {
	 (*i)->draw(zoomX);
      }
   }

   glDisable(GL_BLEND);
   glDisable(GL_STENCIL_TEST);
}


void GLEngine::wipe_area()
{
  int  w = _view->w();
  int  h = _view->h();

  if ( _view->wipe_direction() == ImageView::kNoWipe )
     return;
  else if ( _view->wipe_direction() & ImageView::kWipeVertical )
  {
      w = (int) ( (float) w * _view->wipe_amount() );
  }
  else if ( _view->wipe_direction() & ImageView::kWipeHorizontal )
  {
      h = (int) ( (float) h * _view->wipe_amount() );
  }
  else
  {
     LOG_ERROR( _("Unknown wipe direction") );
  }
  
  glEnable( GL_SCISSOR_TEST );

  int  x = 0;
  int  y = 0;
  glScissor( x, y, w, h );
}


namespace {
  const char* ARBFP1Shader = 
"!!ARBfp1.0\n"
"# cgc version 2.0.0008, build date Nov  1 2007\n"
"# command line args: -I/home/gga/code/applications/mrViewer-cleanup/shaders -profile arbfp1\n"
"# source file: rgba.cg\n"
"#vendor NVIDIA Corporation\n"
"#version 2.0.0.8\n"
"#profile arbfp1\n"
"#program main\n"
"#semantic main.fgImage : TEXUNIT0\n"
"#semantic main.lut : TEXUNIT3\n"
"#semantic main.gain\n"
"#semantic main.gamma\n"
"#semantic main.channel\n"
"#semantic main.enableNormalization\n"
"#semantic main.normMin\n"
"#semantic main.normSpan\n"
"#semantic main.enableLut\n"
"#semantic main.lutF\n"
"#semantic main.lutMin\n"
"#semantic main.lutMax\n"
"#semantic main.lutM\n"
"#semantic main.lutT\n"
"#var float2 tc : $vin.TEXCOORD0 : TEX0 : 0 : 1\n"
"#var sampler2D fgImage : TEXUNIT0 : texunit 0 : 1 : 1\n"
"#var sampler3D lut : TEXUNIT3 : texunit 3 : 2 : 1\n"
"#var half gain :  : c[0] : 3 : 1\n"
"#var half gamma :  : c[1] : 4 : 1\n"
"#var int channel :  : c[2] : 5 : 1\n"
"#var bool enableNormalization :  : c[3] : 6 : 1\n"
"#var half normMin :  : c[4] : 7 : 1\n"
"#var half normSpan :  : c[5] : 8 : 1\n"
"#var bool enableLut :  : c[6] : 9 : 1\n"
"#var bool lutF :  :  : 10 : 0\n"
"#var half lutMin :  : c[7] : 11 : 1\n"
"#var half lutMax :  : c[8] : 12 : 1\n"
"#var half lutM :  : c[9] : 13 : 1\n"
"#var half lutT :  : c[10] : 14 : 1\n"
"#var float4 main.pixel : $vout.COLOR : COL : -1 : 1\n"
"#const c[11] = 0.33333334 1 0 2\n"
"#const c[12] = 3 4 5 6\n"
"#const c[13] = 2.7182817 0.69335938 0.5\n"
"#default gain = 1\n"
"#default gamma = 0.44995117\n"
"#default channel = 0\n"
"#default enableNormalization = 0\n"
"#default normMin = 0\n"
"#default normSpan = 1\n"
"#default enableLut = 0\n"
"#default lutF = 0\n"
"PARAM c[14] = { program.local[0..10],\n"
"		{ 0.33333334, 1, 0, 2 },\n"
"		{ 3, 4, 5, 6 },\n"
"		{ 2.7182817, 0.69335938, 0.5 } };\n"
"TEMP R0;\n"
"TEMP R1;\n"
"TEMP R2;\n"
"TEMP R3;\n"
"TEX R1, fragment.texcoord[0], texture[0], 2D;\n"
"RCP R0.w, c[5].x;\n"
"ADD R0.xyz, R1, -c[4].x;\n"
"MUL R0.xyz, R0, R0.w;\n"
"CMP R0.xyz, -c[3].x, R0, R1;\n"
"MUL R1.xyz, R0, c[0].x;\n"
"MIN R0.xyz, R1, c[8].x;\n"
"MAX R0.xyz, R0, c[7].x;\n"
"MOV R0.w, c[10].x;\n"
"MOV R3.zw, c[11].xyyw;\n"
"LG2 R0.x, R0.x;\n"
"LG2 R0.y, R0.y;\n"
"LG2 R0.z, R0.z;\n"
"MUL R0.xyz, R0, c[9].x;\n"
"MAD R0.xyz, R0, c[13].y, R0.w;\n"
"CMP R1.xyz, -c[6].x, R0, R1;\n"
"TEX R0.xyz, R1, texture[3], 3D;\n"
"POW R0.x, c[13].x, R0.x;\n"
"POW R0.z, c[13].x, R0.z;\n"
"POW R0.y, c[13].x, R0.y;\n"
"CMP R0.xyz, -c[6].x, R0, R1;\n"
"POW R2.w, R0.y, c[1].x;\n"
"POW R3.x, R0.z, c[1].x;\n"
"ADD R0.y, -R3.z, c[2].x;\n"
"POW R2.x, R0.x, c[1].x;\n"
"ABS R3.y, R0;\n"
"MOV R0, c[12];\n"
"ADD R0.x, -R0, c[2];\n"
"ABS R0.x, R0;\n"
"ADD R0.z, -R0, c[2].x;\n"
"ADD R0.w, -R0, c[2].x;\n"
"ABS R0.w, R0;\n"
"MOV R2.y, R2.w;\n"
"MOV R2.z, R3.x;\n"
"CMP R1.xyz, -R3.y, R2, R2.x;\n"
"CMP R2.z, -R3.y, c[11], c[11].y;\n"
"ADD R2.y, -R3.w, c[2].x;\n"
"ABS R3.y, R2;\n"
"ABS R2.y, R2.z;\n"
"CMP R2.z, -R3.y, c[11], c[11].y;\n"
"CMP R2.y, -R2, c[11].z, c[11];\n"
"MUL R3.y, R2, R2.z;\n"
"CMP R1.xyz, -R3.y, R2.w, R1;\n"
"ABS R2.z, R2;\n"
"CMP R2.z, -R2, c[11], c[11].y;\n"
"MUL R2.y, R2, R2.z;\n"
"CMP R0.x, -R0, c[11].z, c[11].y;\n"
"MUL R2.z, R2.y, R0.x;\n"
"CMP R1.xyz, -R2.z, R3.x, R1;\n"
"ADD R0.y, -R0, c[2].x;\n"
"ABS R2.z, R0.y;\n"
"ABS R0.x, R0;\n"
"CMP R0.y, -R0.x, c[11].z, c[11];\n"
"MUL R0.y, R2, R0;\n"
"CMP R0.x, -R2.z, c[11].z, c[11].y;\n"
"MUL R2.y, R0, R0.x;\n"
"ABS R0.x, R0;\n"
"CMP R0.x, -R0, c[11].z, c[11].y;\n"
"CMP R1.xyz, -R2.y, R1.w, R1;\n"
"ABS R0.z, R0;\n"
"MUL R2.z, R0.y, R0.x;\n"
"CMP R2.y, -R0.z, c[11].z, c[11];\n"
"MUL R0.x, R2.z, R2.y;\n"
"MUL R3.y, R2.x, c[13].z;\n"
"CMP R0.y, -R0.x, R3, R1.x;\n"
"MAD R0.z, R1.w, c[13], R0.y;\n"
"CMP R0.x, -R0, R0.z, R0.y;\n"
"MOV R0.yz, R1;\n"
"ADD R1.x, R2, R2.w;\n"
"ADD R1.x, R1, R3;\n"
"ABS R1.y, R2;\n"
"CMP R1.y, -R1, c[11].z, c[11];\n"
"MUL R1.x, R1, c[11];\n"
"CMP R0.w, -R0, c[11].z, c[11].y;\n"
"MUL R1.y, R2.z, R1;\n"
"MUL R0.w, R1.y, R0;\n"
"CMP result.color.xyz, -R0.w, R1.x, R0;\n"
"MOV result.color.w, R1;\n"
"END\n"
;

  const char* NVShader = 
"!!FP1.0\n"
"# cgc version 2.0.0008, build date Nov  1 2007\n"
"# command line args: -I/home/gga/code/applications/mrViewer-cleanup/shaders -profile fp30\n"
"# source file: rgba.cg\n"
"#vendor NVIDIA Corporation\n"
"#version 2.0.0.8\n"
"#profile fp30\n"
"#program main\n"
"#semantic main.fgImage : TEXUNIT0\n"
"#semantic main.lut : TEXUNIT3\n"
"#semantic main.gain\n"
"#semantic main.gamma\n"
"#semantic main.channel\n"
"#semantic main.enableNormalization\n"
"#semantic main.normMin\n"
"#semantic main.normSpan\n"
"#semantic main.enableLut\n"
"#semantic main.lutF\n"
"#semantic main.lutMin\n"
"#semantic main.lutMax\n"
"#semantic main.lutM\n"
"#semantic main.lutT\n"
"#var float2 tc : $vin.TEXCOORD0 : TEX0 : 0 : 1\n"
"#var sampler2D fgImage : TEXUNIT0 : texunit 0 : 1 : 1\n"
"#var sampler3D lut : TEXUNIT3 : texunit 3 : 2 : 1\n"
"#var half gain :  : gain : 3 : 1\n"
"#var half gamma :  : gamma : 4 : 1\n"
"#var int channel :  : channel : 5 : 1\n"
"#var bool enableNormalization :  : enableNormalization : 6 : 1\n"
"#var half normMin :  : normMin : 7 : 1\n"
"#var half normSpan :  : normSpan : 8 : 1\n"
"#var bool enableLut :  : enableLut : 9 : 1\n"
"#var bool lutF :  :  : 10 : 0\n"
"#var half lutMin :  : lutMin : 11 : 1\n"
"#var half lutMax :  : lutMax : 12 : 1\n"
"#var half lutM :  : lutM : 13 : 1\n"
"#var half lutT :  : lutT : 14 : 1\n"
"#var half4 main.pixel : $vout.COLOR : COL : -1 : 1\n"
"#default gain = 1\n"
"#default gamma = 0.44995117\n"
"#default channel = 0\n"
"#default enableNormalization = 0\n"
"#default normMin = 0\n"
"#default normSpan = 1\n"
"#default enableLut = 0\n"
"#default lutF = 0\n"
"DECLARE enableNormalization = {0};\n"
"DECLARE normMin = {0};\n"
"DECLARE normSpan = {1};\n"
"DECLARE gain = {1};\n"
"DECLARE enableLut = {0};\n"
"DECLARE lutMin;\n"
"DECLARE lutMax;\n"
"DECLARE lutT;\n"
"DECLARE lutM;\n"
"DECLARE gamma = {0.44995117};\n"
"DECLARE channel = {0};\n"
"TEX   H0, f[TEX0], TEX0, 2D;\n"
"ADDH  H1.xyz, H0, -normMin.x;\n"
"RCPH  H1.w, normSpan.x;\n"
"MOVXC RC.x, enableNormalization;\n"
"MULH  H0.xyz(NE.x), H1, H1.w;\n"
"MULH  H1.xyz, H0, gain.x;\n"
"MINH  H0.xyz, H1, lutMax.x;\n"
"MAXH  H0.xyz, H0, lutMin.x;\n"
"MOVXC RC.x, enableLut;\n"
"MOVH  H1.w, lutT.x;\n"
"LG2H  H0.x, H0.x;\n"
"LG2H  H0.z, H0.z;\n"
"LG2H  H0.y, H0.y;\n"
"MULH  H0.xyz, H0, lutM.x;\n"
"MADH  H1.xyz(NE.x), H0, {0.69335938}.x, H1.w;\n"
"TEX   R0.xyz, H1, TEX3, 3D;\n"
"POWR  H0.x, {2.7182817}.x, R0.x;\n"
"POWR  H0.y, {2.7182817}.x, R0.y;\n"
"POWR  H0.z, {2.7182817}.x, R0.z;\n"
"MOVH  H1.xyz(NE.x), H0;\n"
"POWH  H1.w, H1.z, gamma.x;\n"
"POWH  H2.x, H1.y, gamma.x;\n"
"POWH  H0.x, H1.x, gamma.x;\n"
"MOVH  H0.z, H1.w;\n"
"MOVH  H0.y, H2.x;\n"
"MOVH  H1.xyz, H0;\n"
"MOVR  R0.x, {1};\n"
"SEQR  H0.y, channel.x, R0.x;\n"
"MOVXC RC.x, H0.y;\n"
"MOVR  R0.x, {2};\n"
"SEQR  H0.z, channel.x, R0.x;\n"
"MOVR  R0.x, {3};\n"
"SEQR  H2.y, channel.x, R0.x;\n"
"MOVH  H1.xyz(NE.x), H0.x;\n"
"SEQX  H0.y, H0, {0}.x;\n"
"MULXC HC.x, H0.y, H0.z;\n"
"SEQX  H0.z, H0, {0}.x;\n"
"MOVH  H1.xyz(NE.x), H2.x;\n"
"MULX  H0.y, H0, H0.z;\n"
"MULXC HC.x, H0.y, H2.y;\n"
"MOVR  R0.x, {4};\n"
"SEQR  H0.z, channel.x, R0.x;\n"
"SEQX  H2.y, H2, {0}.x;\n"
"MULX  H0.y, H0, H2;\n"
"MOVH  H1.xyz(NE.x), H1.w;\n"
"MULXC HC.x, H0.y, H0.z;\n"
"SEQX  H2.y, H0.z, {0}.x;\n"
"MOVR  R0.x, {5};\n"
"SEQR  H0.z, channel.x, R0.x;\n"
"MOVH  H1.xyz(NE.x), H0.w;\n"
"MULX  H0.y, H0, H2;\n"
"MULXC HC.x, H0.y, H0.z;\n"
"MULH  H1.x(NE), H0, {0.5};\n"
"ADDH  H0.x, H0, H2;\n"
"SEQX  H0.z, H0, {0}.x;\n"
"MADH  H1.x(NE), H0.w, {0.5}, H1;\n"
"ADDH  H0.x, H0, H1.w;\n"
"MOVR  R0.x, {6};\n"
"SEQR  H1.w, channel.x, R0.x;\n"
"MULX  H0.y, H0, H0.z;\n"
"MULXC HC.x, H0.y, H1.w;\n"
"MULH  H1.xyz(NE.x), H0.x, {0.33333334}.x;\n"
"MOVH  H1.w, H0;\n"
"MOVH  o[COLH], H1;\n"
"END\n"
;

}

void GLEngine::handle_cg_errors()
{
//   std::cerr << cgGetErrorString (cgGetError()) << std::endl;
//   std::cerr << cgGetLastListing (cgContext) << std::endl;
  exit(1);
}


void
GLEngine::loadBuiltinFragShader()
{

  _rgba = new GLShader();

  if ( _hardwareShaders == kNV30 )
    {
      LOG_INFO( _("Loading built-in NV3.0 rgba shader") );
      _rgba->load( N_("builtin"), NVShader );
    }
  else 
    {
      LOG_INFO( _("Loading built-in arbfp1 rgba shader") );
      _hardwareShaders = kARBFP1;
      _rgba->load( N_("builtin"), ARBFP1Shader );
    }

}


void GLEngine::release()
{
  QuadList::iterator i = _quads.begin();
  QuadList::iterator e = _quads.end();
  for ( ; i != e; ++i )
    {
      delete *i;
    }
  _quads.clear();

  GLLut3d::clear();

  if ( sCharset )
     glDeleteLists( sCharset, 255 );

  if (_rgba)  delete _rgba;
  if (_YByRy) delete _YByRy;
  if (_YCbCr) delete _YCbCr;
}


void GLEngine::resize_background()
{
}

GLEngine::GLEngine(const mrv::ImageView* v) :
DrawEngine( v ),
texWidth( 0 ),
texHeight( 0 )
{
  initialize();
}
 
GLEngine::~GLEngine()
{
  release();
}





} // namespace mrv
