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
#define USE_ARBFP1_SHADERS
#define USE_OPENGL2_SHADERS

#include <vector>
#include <iostream>
#include <sstream>

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

#include <GL/glut.h>



#if defined(WIN32) || defined(WIN64)
#  include <fltk/win32.h>   // for fltk::getDC()
#endif

#include <half.h>
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImfStringAttribute.h>
#include <Iex.h>
#include <CtlExc.h>

#include "core/ctlToLut.h"
#include "core/mrvThread.h"
#include "core/mrvRectangle.h"

#include "gui/mrvIO.h"
#include "gui/mrvImageView.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvPreferences.h"
#include "mrViewer.h"

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

  GLint  GLEngine::_maxTexUnits   = 1;
  bool   GLEngine::_floatTextures = false;
  bool   GLEngine::_halfTextures  = false;
  bool   GLEngine::_pow2Textures  = true;
  bool   GLEngine::_pboTextures   = false;
  bool   GLEngine::_sdiOutput     = false;

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
	std::cerr << where << ": Error " << error << std::endl;
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
    if (!hfont) LOG_ERROR( _("Could not open any font of size ") << fontsize);

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
  _pboTextures = (bool) GLEW_ARB_pixel_buffer_object;
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
      std::cerr << _("GLEW Initialize Error: ") 
		<< glewGetErrorString(err) << std::endl;
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
	  LOG_INFO( _("mrViewer supports YUVA through shaders.") );
	}
      else
	{
	  LOG_INFO( _("mrViewer supports YUV through shaders.") );
	}
    }
  else
    {
      LOG_INFO( _("mrViewer does not support YUV images.") );
    }

  _floatTextures     = (bool) GLEW_ARB_color_buffer_float;
  _halfTextures      = (bool) GLEW_ARB_half_float_pixel;
  _pow2Textures      = !GLEW_ARB_texture_non_power_of_two;


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
  
  // So compositing works properly
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLEngine::color( uchar r, uchar g, uchar b, uchar a = 255 )
{
  glColor4ub( r, g, b, a );
}

void GLEngine::color( float r, float g, float b, float a = 255 )
{
  glColor4f( r, g, b, a );
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
    glListBase(sCharset);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, s);
  glPopAttrib();
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

  glColor3f( 0.0f, 0.0f, 0.0f );

#if 0
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef( float(_view->w()/2), float(_view->h()/2), 0 );
  glScalef( _view->zoom(), _view->zoom(), 1.0f);
  glTranslatef( _view->offset_x(), _view->offset_y(), 0.0f );

  if ( _view->main()->uiPixelRatio->value() )
    glScalef( float(dw), dh / _view->pixel_ratio(), 1.0f );
  else
    glScalef( float(dw), float(dh), 1.0f );
#endif

  double aspect = (double) texWidth / (double) texHeight;   // 1.3
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
  //
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
  float zoomX = _view->zoom();
  float zoomY = _view->zoom();
  if ( _view->main()->uiPixelRatio->value() ) zoomY /= _view->pixel_ratio();

  double sw = ((double)_view->w() -  texWidth * zoomX) / 2;
  double sh = ((double)_view->h() - texHeight * zoomY) / 2;

  unsigned rw = unsigned( r.w() * texWidth  );
  unsigned rh = unsigned( r.h() * texHeight );

  unsigned tx = unsigned( r.x() * texWidth  );
  unsigned ty = unsigned( texHeight - r.y() * texHeight );

  assert( tx <= texWidth );
  assert( ty <= texHeight);
  assert( rw <= texWidth );
  assert( rh <= texHeight);


  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(_view->offset_x() * zoomX + sw, _view->offset_y() * zoomY + sh, 0);
  glTranslatef( tx * zoomX, ty * zoomY, 0);

  glScalef(zoomX, zoomY, 1.0f);

  glBegin(GL_LINE_LOOP);

  glVertex2i(0,  0);
  glVertex2i(rw, 0);
  glVertex2i(rw, -rh);
  glVertex2i(0,  -rh);

  glEnd();

}

/** 
 * Draw an unfilled rectangle (for safe area display)
 * 
 * @param percentX  percentage in X of area 
 * @param percentY  percentage in Y of area
 */
void GLEngine::draw_safe_area( const float percentX, const float percentY,
			       const char* name )
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  float zoomX = _view->zoom();
  float zoomY = _view->zoom();

  float pr = 1.0f;
  if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();

  float sw = ((float)_view->w() - texWidth  * zoomX) / 2;
  float sh = ((float)_view->h() - texHeight * zoomY) / 2;

  float tw = texWidth  / 2.0f;
  float th = texHeight / 2.0f;

  glTranslatef(_view->offset_x() * zoomX + sw, 
	       _view->offset_y() * zoomY + sh, 0);
  glTranslatef(tw * zoomX, th * zoomY, 0);

  glScalef(zoomX * percentX, zoomY * percentY * pr, 1.0f);


  glBegin(GL_LINE_LOOP);

  glVertex2f(-tw,-th);
  glVertex2f(tw, -th);
  glVertex2f(tw,  th);
  glVertex2f(-tw, th);

  glEnd();

  if ( name )
    {
      glPushMatrix();
      glTranslatef(tw+5, th, 0);
      glScalef( 0.1f, 0.1f, 1.0f );
      for (const char* p = name; *p; ++p)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
      glPopMatrix();
    }

}




void GLEngine::alloc_quads( unsigned int num )
{
  unsigned num_quads = _quads.size();
  _quads.reserve( num );
  for ( unsigned q = num_quads; q < num; ++q )
    {
      mrv::GLQuad* quad = new mrv::GLQuad( _view );
      _quads.push_back( quad );
    }
}


void GLEngine::draw_images( ImageList& images )
{
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
     minmax();
     minmax( normMin, normMax );
  }


  unsigned num_quads = 0;
  ImageList::iterator i = images.begin();
  ImageList::iterator e = images.end();

  for ( ; i != e; ++i )
    {
      const Image_ptr& img = *i;
      if ( img->has_subtitle() ) ++num_quads;
      if ( img->has_picture()  ) ++num_quads;
    }

  unsigned num = _quads.size();
  if ( num_quads > num )
    {
      alloc_quads( num_quads );
    }


#if 0
  // @todo: this should be refactored here.
  glColor4f(1.0f,1.0f,1.0f,1.0f);

  glTranslatef( float(_view->w()/2), float(_view->h()/2), 0 );
  glScalef( _view->zoom(), _view->zoom(), 1.0f);
  glTranslatef( _view->offset_x(), _view->offset_y(), 0.0f );

  if ( _view->main()->uiPixelRatio->value() )
    glScalef( float(dw), dh / _view->pixel_ratio(), 1.0f );
  else
    glScalef( float(dw), float(dh), 1.0f );
#endif


  texWidth  = images.back()->width();
  texHeight = images.back()->height();
  QuadList::iterator q = _quads.begin();


  i = images.begin();
  e = images.end();

  glDisable( GL_BLEND );

  for ( ; i != e; ++i, ++q )
    {
      assert( q != _quads.end() );

      const Image_ptr& img = *i;
      GLQuad* quad = *q;
      quad->minmax( normMin, normMax );

      if ( _view->use_lut() )
	{
	  if ( img->image_damage() & CMedia::kDamageLut )
	    {
	      quad->clear_lut();
	    }

	  quad->lut( img );
	}

      if ( img->hires() && img->image_damage() & CMedia::kDamageContents )
	{
	   image_type_ptr pic = img->hires();
	   if ( shader_type() == kNone && img->stopped() && 
		pic->pixel_type() != image_type::kByte )
	   {
	      pic = display( pic, img );
	   }
	   
	   quad->bind( pic );
	   
	   if ( img->is_sequence() )
	   {
	      if ( !img->is_cache_filled( pic->frame() ) )
	      {
		 if ( Preferences::cache_type != Preferences::kCacheAsLoaded &&
		      pic->pixel_type() != image_type::kByte )
		 {
		    image_type::PixelType pixel_type;
		    switch( Preferences::cache_type )
		    {
		       case Preferences::kCacheHalf:
			  pixel_type = image_type::kHalf;
			  break;
		       case Preferences::kCacheFloat:
			  pixel_type = image_type::kFloat;
			  break;
		       case Preferences::kCache8bits:
		       default:
			  pixel_type = image_type::kByte;
			  break;
		    }
		    
		    image_type_ptr dst( new image_type(
						       pic->frame(),
						       pic->width(),
						       pic->height(),
						       pic->channels(),
						       pic->format(),
						       pixel_type
						       ) 
					);
		    display( dst, pic, img );
		    pic = dst;
		 }
		 img->cache( pic );
	      }
	   }
	}
      
      if ( i+1 == e ) wipe_area();
      quad->draw( texWidth, texHeight );
      
      glEnable( GL_BLEND );

      if ( img->has_subtitle() )
	{
	  image_type_ptr sub = img->subtitle();
	  if ( sub )
	    {
	      ++q;
	      // if ( img->image_damage() & CMedia::kDamageSubtitle )
	      // {
		 quad->bind( sub );
	      // }

	      quad->draw( texWidth, texHeight );
	    }
	}


      img->image_damage( img->image_damage() & 
			 ~(CMedia::kDamageContents | CMedia::kDamageLut |
			   CMedia::kDamageSubtitle) );

    }

  glDisable( GL_SCISSOR_TEST );
  glDisable( GL_BLEND );
}



void GLEngine::wipe_area()
{
  int  w = _view->w();
  int  h = _view->h();

  if ( _view->wipe_direction() == ImageView::kNoWipe )
     return;
  else if ( _view->wipe_direction() & ImageView::kWipeVertical )
  {
     w *= _view->wipe_amount();
  }
  else if ( _view->wipe_direction() & ImageView::kWipeHorizontal )
  {
     h *= _view->wipe_amount();
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

  if (_rgba)  delete _rgba;
  if (_YByRy) delete _YByRy;
  if (_YCbCr) delete _YCbCr;
}


void GLEngine::resize_background()
{
}

GLEngine::GLEngine(const mrv::ImageView* v) :
  DrawEngine( v )
{
  initialize();
}
 
GLEngine::~GLEngine()
{
  release();
}





} // namespace mrv
