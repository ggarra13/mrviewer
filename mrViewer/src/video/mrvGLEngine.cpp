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
/* This file contains code borrowed from mpv's color corrections. */
/**
 * @file   mrvGLEngine.cpp
 * @author gga
 * @date   Fri Jul 13 23:47:14 2007
 *
 * @brief2  OpenGL engine
 *
 */

//
// Set these for testing some features as if using a lower gfx card
//
// #define TEST_NO_SHADERS  // test without hardware shaders
// #define TEST_NO_YUV      // test in rgba mode only

#define USE_HDR 0
#define USE_NV_SHADERS
#define USE_OPENGL2_SHADERS
#define USE_ARBFP1_SHADERS
//#define USE_STEREO_GL

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

extern "C" {

#include <GL/glew.h>

#if defined(WIN32) || defined(WIN64)
#  include <GL/wglew.h>
#elif defined(LINUX)
#  include <GL/glxew.h>
#endif

#include <libavutil/mastering_display_metadata.h>
#include <GL/glu.h>
#include <GL/glut.h>

}



#include <half.h>
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImfStringAttribute.h>
#include <Iex.h>
#include <CtlExc.h>
#include <halfLimits.h>


#include "core/mrvMath.h"
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
#include "video/mrvGLSphere.h"
#include "video/mrvGLCube.h"
#include "video/mrvGLLut3d.h"
#include "video/mrvCSPUtils.h"

#undef TRACE
#define TRACE(x)


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

// #define glPushMatrix() do {                     \
//     glPushMatrix(); \
// std::cerr << "push matrix " << __FUNCTION__ << " " << __LINE__ << std::endl; \
// } while(0)


// #define glPopMatrix() do {                      \
//     glPopMatrix(); \
// std::cerr << "pop matrix " << __FUNCTION__ << " " << __LINE__ << std::endl; \
// } while(0)




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

  GLuint GLEngine::sCharset = 0;   // display list for characters
  unsigned int GLEngine::_maxTexWidth;
  unsigned int GLEngine::_maxTexHeight;


  //
  // Check for opengl errors and print function name where it happened.
  //
void GLEngine::handle_gl_errors(const char* where, const unsigned line,
                                const bool print )
  {
      GLenum error = glGetError();
      if ( error == GL_NO_ERROR ) return;

      while (error != GL_NO_ERROR)
      {
          if ( print )
          {
              mrvALERT( where << " (" << line << ")"
                        << _(": Error ") << error << " "
                        << gluErrorString(error) );
              LOG_ERROR( where << " (" << line << ")"
                         << _(": Error ") << error << " "
                         << gluErrorString(error) );
          }
          error = glGetError();
      }
  }



void zrot2offsets( double& x, double& y,
                   const CMedia* img,
                   const mrv::ImageView::FlipDirection flip,
                   const double zdeg )
{
    return;
    x = 0.0; y = 0.0;
    double rad = zdeg * M_PI / 180.0;
    double sn = sin( rad );
    double cs = cos( rad );
    mrv::Recti dpw = img->display_window();
    if ( is_equal( sn, -1.0 ) )
    {
        if ( flip & ImageView::kFlipVertical )    y = (double)-dpw.w();
        if ( flip & ImageView::kFlipHorizontal )  x = (double)-dpw.h();
    }
    else if ( (is_equal( sn, 0.0, 0.001 ) && is_equal( cs, -1.0, 0.001 )) )
    {
        if ( flip & ImageView::kFlipVertical )    x = (double)dpw.w();
        if ( flip & ImageView::kFlipHorizontal )  y = (double)-dpw.h();
    }
    else if ( (is_equal( sn, 1.0, 0.001 ) && is_equal( cs, 0.0, 0.001 )) )
    {
        if ( flip & ImageView::kFlipVertical )    y = (double)dpw.w();
        if ( flip & ImageView::kFlipHorizontal )  x = (double)dpw.h();
    }
    else
    {
        if ( flip & ImageView::kFlipVertical )    x = (double)-dpw.w();
        if ( flip & ImageView::kFlipHorizontal )  y = (double) dpw.h();
    }
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


    DBG( __FUNCTION__ << " " << __LINE__ );
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
    DBG( __FUNCTION__ << " " << __LINE__ );
    return o.str();
  }



void GLEngine::init_charset()
{
  unsigned numChars = 255;
  int fontsize = 16;

#ifdef WIN32
    DBG( __FUNCTION__ << " " << __LINE__ );
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


    DBG( __FUNCTION__ << " " << __LINE__ );
    HFONT    fid = CreateFontIndirect(&lf);
    DBG( __FUNCTION__ << " " << __LINE__ );
    HFONT oldFid = (HFONT)SelectObject(hDC, fid);

    sCharset = glGenLists( numChars );

    wglMakeCurrent( hDC, hRC );
    wglUseFontBitmaps(hDC, 0, numChars-1, sCharset);

    SelectObject(hDC, oldFid);
    DBG( __FUNCTION__ << " " << __LINE__ );
#else
    DBG( __FUNCTION__ << " " << __LINE__ );
    // Find Window's default font
    Display* gdc = fltk::xdisplay;

    // Load XFont to user's specs
    char font_name[256];
    sprintf( font_name, N_("-*-fixed-*-r-normal--%d-0-0-0-*-*-iso8859-1"),
             fontsize );
    XFontStruct* hfont = XLoadQueryFont( gdc, font_name );
    if (!hfont) {
       LOG_ERROR( _("Could not open any font of size ") << fontsize);
       hfont = XLoadQueryFont( gdc, "fixed" );
       if ( !hfont ) return;
    }

    // Create GL lists out of XFont
    sCharset = glGenLists( numChars );
    glXUseXFont(hfont->fid, 0, numChars-1, sCharset);

    // Free font and struct
    XFreeFont( gdc, hfont );
    DBG( __FUNCTION__ << " " << __LINE__ );
#endif

  CHECK_GL;
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
  CHECK_GL;

#ifndef TEST_NO_PBO_TEXTURES // test not using pbo textures
  _pboTextures = ( GLEW_ARB_pixel_buffer_object != GL_FALSE );
#endif

  _has_yuv = false;

  _maxTexUnits = 1;
  if ( GLEW_ARB_multitexture )
    {
        DBG( __FUNCTION__ << " " << __LINE__ );
#ifndef TEST_NO_YUV
      glGetIntegerv(GL_MAX_TEXTURE_UNITS, &_maxTexUnits);
      CHECK_GL;

      if ( _maxTexUnits >= 3 )  _has_yuv = true;
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
    DBG( __FUNCTION__ << " " << __LINE__ );
    GLenum err = glewInit();
    DBG( __FUNCTION__ << " " << __LINE__ );
    if (GLEW_OK != err)
    {
        DBG( "glewInit failed" );
        /* Problem: glewInit failed, something is seriously wrong. */
        LOG_ERROR( _("GLEW Initialize Error: ") << glewGetErrorString(err) );
        return;
    }



// #if defined(WIN32) || defined(WIN64)
//   err = wglewInit();
//   if (GLEW_OK != err)
//     {
//       /* Problem: wglewInit failed, something is seriously wrong. */
//       std::cerr << "WGLEW Initialize Error: "
//              << glewGetErrorString(err) << std::endl;
//       exit(1);
//     }
// #else
//   err = glxewInit();
//   if (GLEW_OK != err)
//     {
//       /* Problem: glxewInit failed, something is seriously wrong. */
//       std::cerr << "GLXEW Initialize Error: "
//              << glewGetErrorString(err) << std::endl;
//       exit(1);
//     }
// #endif
}

void GLEngine::refresh_shaders()
{
    delete _YCbCr; _YCbCr = NULL;
    delete _YByRy; _YByRy = NULL;
    delete _YCbCrA; _YCbCrA = NULL;
    delete _YByRyA; _YByRyA = NULL;
    delete _rgba; _rgba = NULL;

    std::string directory;

    if ( _has_yuv )
    {

        _has_yuva = false;
        if ( _maxTexUnits > 4 )  // @todo: bug fix
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

    if ( ! directory.empty() )
    {
        char shaderFile[256];

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

        try
        {

            sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("rgba"), ext );

            DBG( __FUNCTION__ << " " << __LINE__ << " shader file "
                 << shaderFile );


            _rgba = new GLShader( shaderFile );
 
        }
        catch ( const std::exception& e )
        {
            LOG_ERROR( shaderFile << ": " << e.what() );
            directory.clear();
        }

        try
        {
            if ( _has_yuv )
            {
                if ( ! _has_hdr )
                {
                    sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("YCbCr"),
                             ext );
                    _YCbCr = new GLShader( shaderFile );
                }
                else
                {
                    _YCbCr = NULL;
                }


                sprintf( shaderFile, N_("%s/%s.%s"), dir, N_("YByRy"), ext );
                _YByRy = new GLShader( shaderFile );
            }
        }
        catch ( const std::exception& e )
        {
            LOG_ERROR( shaderFile << ": " << e.what() );
            delete _YByRy; _YByRy = NULL;
            delete _YCbCr; _YCbCr = NULL;
            _has_yuv  = false;
            _has_yuva = false;
        }

        try
        {
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
            LOG_ERROR( shaderFile << ": " << e.what() );
            delete _YByRyA; _YByRyA = NULL;
            delete _YCbCrA; _YCbCrA = NULL;
            _has_yuva = false;
        }
    }
    else
    {
        LOG_WARNING( _("Environment variable MRV_SHADER_PATH not found, "
                       "using built-in shader.") );

        if ( directory.empty() )
        {
            directory = N_(".");
            loadBuiltinFragShader();
        }
        else
        {
            LOG_INFO( _("Hardware shaders not available.") );
            _has_yuv  = false;
            _has_yuva = false;
        }
    }
}

/**
 * Initialize OpenGL context, textures, and get opengl features for
 * this gfx card.
 *
 */
void GLEngine::initialize()
{
  static bool glut_init = false;

  if ( !glut_init )
  {
      DBG( "call glutInit" );
      int argc = 1;
      static char* args[] = { (char*)"GlEngine", NULL };
      glutInit( &argc, args );
      glut_init = true;
  }

  init_GLEW();

  init_charset();

  init_textures();


// #if defined(WIN32) || defined(WIN64)
//   if ( WGLEW_WGL_swap_control )
//     {
//       std::cerr << "swap control vsync? "
//              << wglGetSwapIntervalEXT() << std::endl;
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

  const char* shader_type = getenv("MRV_SHADER_TYPE");
  if ( shader_type )
  {
      if ( stricmp( shader_type, "GL" ) == 0 ||
           stricmp( shader_type, "GLSL" ) == 0 ||
           stricmp( shader_type, "OPENGL" ) == 0 )
      {
          _hardwareShaders = kGLSL;
          _has_hdr = USE_HDR;
      }
      else if ( stricmp( shader_type, "NV" ) == 0 ||
                stricmp( shader_type, "NV30" ) == 0 ||
                stricmp( shader_type, "NVIDIA" ) == 0 ||
                stricmp( shader_type, "CG" ) == 0 ||
                stricmp( shader_type, "CGGL" ) == 0 )
          _hardwareShaders = kNV30;
      else if ( stricmp( shader_type, "ARBFP1" ) == 0 ||
                stricmp( shader_type, "ARBFP" ) == 0 )
          _hardwareShaders = kARBFP1;
      else
          _hardwareShaders = kAuto;
  }
  else
  {
      _hardwareShaders = kAuto;
  }


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
      {
        _hardwareShaders = kGLSL;
      }
#endif

#ifdef USE_NV_SHADERS
      if ( GLEW_NV_fragment_program )
        _hardwareShaders = kNV30;
#endif

      if ( _hardwareShaders == kGLSL )
          _has_hdr = USE_HDR;

      LOG_INFO( "Selecting shader type automatically: " << shader_type_name() );

#endif // ifndef TEST_NO_SHADERS

    }

  if ( _hardwareShaders != kNone )
  {
      LOG_INFO( _("Using hardware shader profile: ") << shader_type_name() );

      refresh_shaders();

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
  }

  _floatTextures     = ( GLEW_ARB_color_buffer_float != GL_FALSE );
  _halfTextures      = ( GLEW_ARB_half_float_pixel != GL_FALSE );
  _pow2Textures      = !GLEW_ARB_texture_non_power_of_two;
  _fboRenderBuffer   = ( GLEW_ARB_framebuffer_object != GL_FALSE );

  ImageView::VRType t = _view->vr();
  if ( t == ImageView::kVRSphericalMap )
      alloc_spheres( 4 );
  else if ( t == ImageView::kVRCubeMap )
      alloc_cubes( 4 );
  else
      alloc_quads( 4 );

  CHECK_GL;
}


/**
 * Resets the view matrix and sets the projection to match the window's viewport
 *
 */
void GLEngine::reset_view_matrix()
{
    glMatrixMode(GL_PROJECTION);


    if ( _view->vr() != vr )
    {
        vr = _view->vr();
        clear_quads();
    }

    ImageView* view = const_cast< ImageView* >( _view );
    if ( view->vr() == ImageView::kNoVR )
    {
        CHECK_GL;
        view->ortho();
        _rotX = _rotY = 0.0;
        CHECK_GL;
    }
    else
    {
        unsigned w = _view->w();
        unsigned h = _view->h();
        glLoadIdentity();
        glViewport(0, 0, w, h);
        gluPerspective( vr_angle, (float)w / (float)h, 0.1, 3.0);
        gluLookAt( 0, 0, 1, 0, 0, -1, 0, 1, 0 );
        CHECK_GL;
    }

    // Makes gl a tad faster
    glDisable(GL_DEPTH_TEST);
    CHECK_GL;
    glDisable(GL_LIGHTING);
    CHECK_GL;



}

void GLEngine::evaluate( const CMedia* img,
                         const Imath::V3f& rgb, Imath::V3f& out )
{
  QuadList::iterator q = _quads.begin();
  QuadList::iterator e = _quads.end();
  out = rgb;
  for ( ; q != e; ++q )
  {
      if ( (*q)->image() == img )
      {
          const GLLut3d* lut = (*q)->lut();
          if ( !lut ) {
              out = rgb;
              return;
          }

          lut->evaluate( rgb, out );
          return;
      }

  }


}

void GLEngine::rotate( const double z )
{
    glRotated( z, 0, 0, 1 );
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

#if 0
static void pass_convert_yuv(ostringstream& code)
{

    struct mp_csp_params cparams = MP_CSP_PARAMS_DEFAULTS;
    cparams.gray = p->is_gray;
    mp_csp_set_image_params(&cparams, &p->image_params);
    mp_csp_equalizer_state_get(p->video_eq, &cparams);
    p->user_gamma = 1.0 / (cparams.gamma * p->opts.gamma);

    pass_describe(p, "color conversion");

    if (p->color_swizzle[0])
        GLSLF("c = c.%s;\n", p->color_swizzle);

    // Pre-colormatrix input gamma correction
    if (cparams.color.space == MP_CSP_XYZ)
        GLSL(c.rgb = pow(c.rgb, vec3(2.6));) // linear light

    // We always explicitly normalize the range in pass_read_video
    cparams.input_bits = cparams.texture_bits = 0;

    // Conversion to RGB. For RGB itself, this still applies e.g. brightness
    // and contrast controls, or expansion of e.g. LSB-packed 10 bit data.
    struct mp_cmat m = {{{0}}};
    mp_get_csp_matrix(&cparams, &m);
    gl_sc_uniform_mat3(sc, "colormatrix", true, &m.m[0][0]);
    gl_sc_uniform_vec3(sc, "colormatrix_c", m.c);

    GLSL(c.rgb = mat3(colormatrix) * c.rgb + colormatrix_c;)

    if (p->image_params.color.space == MP_CSP_BT_2020_C) {
        // Conversion for C'rcY'cC'bc via the BT.2020 CL system:
        // C'bc = (B'-Y'c) / 1.9404  | C'bc <= 0
        //      = (B'-Y'c) / 1.5816  | C'bc >  0
        //
        // C'rc = (R'-Y'c) / 1.7184  | C'rc <= 0
        //      = (R'-Y'c) / 0.9936  | C'rc >  0
        //
        // as per the BT.2020 specification, table 4. This is a non-linear
        // transformation because (constant) luminance receives non-equal
        // contributions from the three different channels.
        GLSLF("// constant luminance conversion\n");
        GLSL(c.br = c.br * mix(vec2(1.5816, 0.9936),
                                       vec2(1.9404, 1.7184),
                                       lessThanEqual(c.br, vec2(0)))
                        + c.gg;)
        // Expand channels to camera-linear light. This shader currently just
        // assumes everything uses the BT.2020 12-bit gamma function, since the
        // difference between 10 and 12-bit is negligible for anything other
        // than 12-bit content.
        GLSL(c.rgb = mix(c.rgb * vec3(1.0/4.5),
                             pow((c.rgb + vec3(0.0993))*vec3(1.0/1.0993),
                                 vec3(1.0/0.45)),
                             lessThanEqual(vec3(0.08145), c.rgb));)
        // Calculate the green channel from the expanded RYcB
        // The BT.2020 specification says Yc = 0.2627*R + 0.6780*G + 0.0593*B
        GLSL(c.g = (c.g - 0.2627*c.r - 0.0593*c.b)*1.0/0.6780;)
        // Recompress to receive the R'G'B' result, same as other systems
        GLSL(c.rgb = mix(c.rgb * vec3(4.5),
                             vec3(1.0993) * pow(c.rgb, vec3(0.45)) - vec3(0.0993),
                             lessThanEqual(vec3(0.0181), c.rgb));)
    }

    GLSL(c.a = 1.0;)
}

#endif


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
    CHECK_GL;
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    CHECK_GL;
    glClearColor(r, g, b, a );
    CHECK_GL;
    glClear( GL_STENCIL_BUFFER_BIT );
    CHECK_GL;
    glClear( GL_COLOR_BUFFER_BIT );
    CHECK_GL;
    glShadeModel( GL_FLAT );
    CHECK_GL;
}

void GLEngine::set_blend_function( int source, int dest )
{
  // So compositing works properly
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc( (GLenum) source, (GLenum) dest );
  CHECK_GL;
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
   CHECK_GL;
   glBindTexture(GL_TEXTURE_2D, textureId);
   CHECK_GL;
   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
   CHECK_GL;
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   CHECK_GL;
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   CHECK_GL;

   GLenum internalFormat = GL_RGBA32F_ARB;
   GLenum dataFormat = GL_RGBA;
   GLenum pixelType = GL_FLOAT;

   Image_ptr img = images.back();

   mrv::image_type_ptr pic = img->hires();
   if (!pic) return false;

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
   CHECK_GL;

   glGenFramebuffers(1, &id);
   CHECK_GL;
   glBindFramebuffer(GL_FRAMEBUFFER, id);
   CHECK_GL;

   glGenRenderbuffers(1, &rid);
   CHECK_GL;
   glBindRenderbuffer( GL_RENDERBUFFER, rid );
   CHECK_GL;



   if ( w > GL_MAX_RENDERBUFFER_SIZE ) return false;
   if ( h > GL_MAX_RENDERBUFFER_SIZE ) return false;

   // glFramebufferParameteri( GL_DRAW_FRAMEBUFFER,
   //                       GL_FRAMEBUFFER_DEFAULT_WIDTH, w);
   // glFramebufferParameteri( GL_DRAW_FRAMEBUFFER,
   //                       GL_FRAMEBUFFER_DEFAULT_HEIGHT, h);
   // glFramebufferParameteri( GL_DRAW_FRAMEBUFFER,
   //                       GL_FRAMEBUFFER_DEFAULT_SAMPLES, 4 );

   // glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_STENCIL,
   //                        w, h );
   CHECK_GL;
   glBindRenderbuffer( GL_RENDERBUFFER, 0 );
   CHECK_GL;

   // attach a texture to FBO color attachement point
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D, textureId, 0);
   CHECK_GL;

   // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
   //                           GL_RENDERBUFFER, rid);
   // CHECK_GL;

   glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0);
   CHECK_GL;

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
   CHECK_GL;

   Image_ptr img = images.back();
   mrv::image_type_ptr pic = img->hires();
   if (!pic) return;

   unsigned w = pic->width();
   unsigned h = pic->height();
   glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
   CHECK_GL;
   glBindTexture(GL_TEXTURE_2D, 0);
   CHECK_GL;

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   CHECK_GL;
   glDeleteFramebuffers(1, &id);
   CHECK_GL;
   glDeleteRenderbuffers(1, &rid);
   CHECK_GL;

}

void GLEngine::draw_title( const float size,
                           const int y, const char* text )
{
  if ( !text ) return;

  DBG( __FUNCTION__ << " " << __LINE__ );
  void* font = GLUT_STROKE_MONO_ROMAN;

  glMatrixMode(GL_MODELVIEW);
  CHECK_GL;
  glPushMatrix();
  CHECK_GL;
  glLoadIdentity();
  CHECK_GL;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  CHECK_GL;
  glEnable(GL_BLEND);
  CHECK_GL;
  glEnable(GL_LINE_SMOOTH);
  CHECK_GL;

  glLineWidth(4.0);
  CHECK_GL;

  int sum = 0;
  for (const char* p = text; *p; ++p)
      sum += glutStrokeWidth( font, *p );
  CHECK_GL;

  float x = ( float( _view->w() ) - float(sum) * size ) / 2.0f;

  float rgb[4];
  glGetFloatv( GL_CURRENT_COLOR, rgb );
  CHECK_GL;

  glColor4f( 0.f, 0.f, 0.f, 1.0f );
  glLoadIdentity();
  glTranslatef( x, GLfloat( y ), 0 );
  glScalef( size, size, 1.0 );
  for (const char* p = text; *p; ++p)
    glutStrokeCharacter( font, *p );
  CHECK_GL;

  glColor4f( rgb[0], rgb[1], rgb[2], rgb[3] );
  CHECK_GL;
  glLoadIdentity();
  CHECK_GL;
  glTranslatef( x-2, float(y+2), 0 );
  CHECK_GL;
  glScalef( size, size, 1.0 );
  CHECK_GL;
  for (const char* p = text; *p; ++p)
    glutStrokeCharacter( font, *p );
  CHECK_GL;

  glMatrixMode( GL_MODELVIEW );
  CHECK_GL;
  glPopMatrix();

  CHECK_GL;
  glDisable(GL_BLEND);
  CHECK_GL;
  glDisable(GL_LINE_SMOOTH);
  CHECK_GL;
  glLineWidth(1.0);
  CHECK_GL;
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
    if (! sCharset ) return;

    DBG( __FUNCTION__ << " " << __LINE__ );
    glLoadIdentity();
    glRasterPos2i( x, y );

    glPushAttrib( GL_LIST_BIT | GL_DEPTH_TEST );
    glDisable( GL_DEPTH_TEST );

    glListBase(sCharset);
    glCallLists( GLsizei( strlen(s) ), GL_UNSIGNED_BYTE, s);

    glPopAttrib();
}

void GLEngine::draw_cursor( const double x, const double y )
{
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();

    DBG( __FUNCTION__ << " " << __LINE__ );
   double pr = 1.0;
   if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();

   double zoomX = _view->zoom();
   double zoomY = _view->zoom();

   double tw = double(texWidth)  / 2.0;
   double th = double(texHeight) / 2.0;

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
                                    const int W, const int H)
{
    DBG( __FUNCTION__ << " " << __LINE__ );
    CHECK_GL;
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    CHECK_GL;
    glDepthMask( GL_FALSE );
    CHECK_GL;
    glColor4f( 0.0f, 0.0f, 0.0f, 0.0f );
    glEnable( GL_STENCIL_TEST );
    CHECK_GL;
    glStencilFunc( GL_ALWAYS, 0x1, 0xffffffff );
    CHECK_GL;
    glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
    CHECK_GL;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    if ( _view->main()->uiPixelRatio->value() )
    {
        double pr = 1.0 / _view->pixel_ratio();
        glScaled( 1.0, pr, 1.0 );
    }


    glBegin( GL_QUADS );
    {
        glVertex2d(x, -y);
        glVertex2d(W, -y);
        glVertex2d(W, -H);
        glVertex2d(x, -H);
    }
    glEnd();

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    // just draw where inside of the mask
    glStencilFunc(GL_EQUAL, 0x1, 0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_BLEND);
}

inline
void GLEngine::set_matrix( const mrv::ImageView::FlipDirection flip )
{
    if ( _view->vr() ) return;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //
    // Translate to center of screen
    //
    glTranslated( double(_view->w())/2, double(_view->h())/2, 0 );


    //
    // Scale to zoom factor
    //
    glScaled( _view->zoom(), _view->zoom(), 1.0);


    //
    // Offset to user translation
    //
    glTranslated( _view->offset_x(), _view->offset_y(), 0.0 );

    //
    // Handle flip
    //
    if ( flip != ImageView::kFlipNone )
    {
        float x = 1.0f, y = 1.0f;
        if ( flip & ImageView::kFlipVertical )   x = -1.0f;
        if ( flip & ImageView::kFlipHorizontal ) y = -1.0f;

        glScalef( x, y, 1.0f );
    }

    //
    // Handle pixel ratio
    //
    if ( _view->main()->uiPixelRatio->value() ) {
        double pr = 1.0 / _view->pixel_ratio();
        glScaled( 1.0, pr, 1.0 );
    }

    CHECK_GL;

}

/**
 * Draws the mask
 *
 * @param pct percent of mask to draw
 */
void GLEngine::draw_mask( const float pct )
{
  mrv::media fg = _view->foreground();
  if ( !fg ) return;

    DBG( __FUNCTION__ << " " << __LINE__ );


  Image_ptr img = fg->image();

  mrv::Recti dpw2 = img->display_window2();
  mrv::Recti dpw = img->display_window();

  if ( ( img->stereo_output() & CMedia::kStereoSideBySide ) ==
       CMedia::kStereoSideBySide )
  {
      dpw.w( dpw.w() + dpw2.w() );
  }
  else if ( ( img->stereo_output() & CMedia::kStereoBottomTop ) ==
            CMedia::kStereoBottomTop )
  {
      dpw.h( dpw.h() + dpw2.h() );
  }

  glColor3f( 0.0f, 0.0f, 0.0f );
  glDisable( GL_STENCIL_TEST );

  ImageView::FlipDirection flip = _view->flip();

  set_matrix( flip );

  double zdeg = img->rot_z();

  double x=0.0, y = 0.0;
  //zrot2offsets( x, y, img, flip, zdeg );


  glRotated( zdeg, 0, 0, 1 );
  translate( img->x() + x + dpw.x(), img->y() + y - dpw.y(), 0 );

  glScaled( dpw.w(), dpw.h(), 1.0 );
  translate( 0.5, -0.5, 0.0 );


  double aspect = (double) dpw.w() / (double) dpw.h();   // 1.3
  double target_aspect = 1.0 / pct;
  double amount = (0.5 - target_aspect * aspect / 2);

  //
  // Bottom mask
  //
  glBegin( GL_POLYGON );
  {
    glVertex2d( -0.5,  -0.5 + amount );
    glVertex2d(  0.5,  -0.5 + amount );
    glVertex2d(  0.5,  -0.5 );
    glVertex2d( -0.5,  -0.5 );
  }
  glEnd();

  //
  // Top mask
  //
  glBegin( GL_POLYGON );
  {
    glVertex2d( -0.5,  0.5 );
    glVertex2d(  0.5,  0.5 );
    glVertex2d(  0.5,  0.5 - amount );
    glVertex2d( -0.5,  0.5 - amount );
  }
  glEnd();


}


/**
 * Draw an overlay rectangle (like selection)
 *
 * @param r rectangle to draw
 */
void GLEngine::draw_rectangle( const mrv::Rectd& r,
                               const mrv::ImageView::FlipDirection flip,
                               const double zdeg )
{

    mrv::media fg = _view->foreground();
    if (!fg) return;

    DBG( __FUNCTION__ << " " << __LINE__ );
    Image_ptr img = fg->image();

    mrv::Recti daw = img->data_window();


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPushAttrib( GL_STENCIL_TEST );
    glDisable( GL_STENCIL_TEST );

    set_matrix( flip );

    double x = 0.0, y = 0.0;
    zrot2offsets( x, y, img, flip, zdeg );

    glRotated( zdeg, 0, 0, 1 );
    translate( x + r.x(), y - r.y(), 0 );

    double rw = r.w();
    double rh = r.h();

    glLineWidth( 1.0 );

    // glEnable(GL_COLOR_LOGIC_OP);
    // glLogicOp(GL_XOR);

    glBegin(GL_LINE_LOOP);

    glVertex2d(0.0, 0.0);
    glVertex2d(rw,  0.0);
    glVertex2d(rw,  -rh);
    glVertex2d(0.0, -rh);

    glEnd();

    // glDisable(GL_COLOR_LOGIC_OP);

    glPopAttrib();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}

void GLEngine::draw_safe_area_inner( const double tw, const double th,
                                     const char* name )
{
  glLineWidth( 1.0 );

    DBG( __FUNCTION__ << " " << __LINE__ );
  glBegin(GL_LINE_LOOP);

  glVertex2d(-tw,-th);
  glVertex2d(tw, -th);
  glVertex2d(tw,  th);
  glVertex2d(-tw, th);

  glEnd();

  if ( name )
    {
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      translate(tw+5, th, 0);
      glScalef( 0.1f, 0.1f, 1.0f );
      for (const char* p = name; *p; ++p)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
      glMatrixMode(GL_MODELVIEW);
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

    mrv::media bg = _view->background();
    Image_ptr bimg = NULL;
    if (bg)
    {
        bimg = bg->image();
    }

    DBG( __FUNCTION__ << " " << __LINE__ );
    Image_ptr img = fg->image();

    mrv::Recti dpw2;
    if ( bimg) {
        dpw2 = bimg->display_window();
        dpw2.x( dpw2.x() + bimg->x() );
        dpw2.y( dpw2.y() - bimg->y() );
        dpw2.w( dpw2.w() * bimg->scale_x() );
        dpw2.h( dpw2.h() * bimg->scale_y() );
    }
    mrv::Recti dpw = img->display_window();

    ImageView::FlipDirection flip = _view->flip();

    glDisable( GL_STENCIL_TEST );

    set_matrix( flip );

    double x = dpw.x();
    double y = dpw.y();
    int tw = dpw.w() / 2.0;
    int th = dpw.h() / 2.0;

    dpw.merge( dpw2 );

    double zdeg = img->rot_z();

    zrot2offsets( x, y, img, flip, zdeg );

    glRotated( zdeg, 0, 0, 1 );
    translate(  x + tw, - y - th, 0 );


    tw *= percentX;
    th *= percentY;

    draw_safe_area_inner( tw, th, name );

    if ( ( img->stereo_output() & CMedia::kStereoSideBySide ) ==
         CMedia::kStereoSideBySide )
    {
        translate( dpw.w(), 0, 0 );
        draw_safe_area_inner( tw, th, name );
    }
    else if ( ( img->stereo_output() & CMedia::kStereoBottomTop ) ==
              CMedia::kStereoBottomTop )
    {
        translate( 0, -dpw.h(), 0 );
        draw_safe_area_inner( tw, th, name );
    }

}




inline double GLEngine::rot_y() const
{
    return _rotY;
}

inline double GLEngine::rot_x() const
{
    return _rotX;
}



inline void GLEngine::rot_x( double t )
{
    _rotX = t;
}

inline void GLEngine::rot_y( double t )
{
    _rotY = t;
}

void GLEngine::alloc_cubes( size_t num )
{
  size_t num_quads = _quads.size();
  _quads.reserve( num );
  for ( size_t q = num_quads; q < num; ++q )
    {
      mrv::GLCube* s = new mrv::GLCube( _view );
      _quads.push_back( s );
    }
}

void GLEngine::alloc_spheres( size_t num )
{
  size_t num_quads = _quads.size();
  _quads.reserve( num );
  for ( size_t q = num_quads; q < num; ++q )
    {
      mrv::GLSphere* s = new mrv::GLSphere( _view );
      _quads.push_back( s );
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


void GLEngine::draw_selection_marquee( const mrv::Rectd& r )
{
    Image_ptr img = _view->selected_image();
    if ( img == NULL ) return;

    DBG( __FUNCTION__ << " " << __LINE__ );
    ImageView::FlipDirection flip = _view->flip();

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    if ( _view->action_mode() == ImageView::kMovePicture )
    {
        glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );
    }
    else
    {
        glColor4f( 1.0f, 0.3f, 0.0f, 1.0f );
    }

    double zdeg = 0.0;
    if ( img ) zdeg = img->rot_z();

    draw_rectangle( r, _view->flip(), 0.0 );

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPushAttrib( GL_STENCIL_TEST );
    glDisable( GL_STENCIL_TEST );

    set_matrix( flip );


    mrv::Recti dpw = img->display_window();
    double x = 0.0, y = 0.0;
    if ( flip & ImageView::kFlipVertical )    x = (double)-dpw.w();
    if ( flip & ImageView::kFlipHorizontal )  y = (double) dpw.h();

    translate( x + r.x(), y - r.y(), 0 );

    if ( _view->action_mode() == ImageView::kScalePicture )
    {
        glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );
    }
    else
    {
        glColor4f( 1.0f, 0.3f, 0.0f, 1.0f );
    }
    glBegin( GL_TRIANGLES );

    const double kSize = 20; //r.w() / 64.0;
    {
        // glVertex2d(  0.0,  0.0 );
        // glVertex2d( kSize,  0.0 );
        // glVertex2d(  0.0, -kSize );

        // glVertex2d(  r.w(),        0.0 );
        // glVertex2d(  r.w()-kSize,  0.0 );
        // glVertex2d(  r.w(),       -kSize );

        glVertex2d(  r.w(),       -r.h() );
        glVertex2d(  r.w()-kSize, -r.h() );
        glVertex2d(  r.w(),       -r.h()+kSize );

        // glVertex2d(  0.0,  -r.h() );
        // glVertex2d( kSize, -r.h() );
        // glVertex2d(  0.0,  -r.h()+kSize );
    }

    glEnd();

    // Draw Crosshair
    glColor4f( 1.0f, 0.3f, 0.0f, 1.0f );
    double rw = r.w() / 2.0;
    double rh = -r.h() / 2.0;
    glBegin( GL_LINES );
    {
        glVertex2d( rw, rh );
        glVertex2d( rw, rh + kSize );

        glVertex2d( rw, rh );
        glVertex2d( rw + kSize, rh );

        glVertex2d( rw, rh );
        glVertex2d( rw, rh - kSize );

        glVertex2d( rw, rh );
        glVertex2d( rw - kSize, rh );
    }
    glEnd();

    char buf[128];

    if ( _view->action_mode() == ImageView::kScalePicture )
    {
        double x = img->scale_x();
        double y = img->scale_y();
        sprintf( buf, "Scale: %g, %g", x, y );
    }
    else
    {
        int xi = round(img->x());
        int yi = round(img->y());
        sprintf( buf, "Pos: %d, %d", xi, yi );
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    translate(rw+kSize, rh-kSize, 0);
    glScalef( 0.2f, 0.2f, 1.0f );
    for (const char* p = buf; *p; ++p)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void GLEngine::draw_data_window( const mrv::Rectd& r )
{
    DBG( __FUNCTION__ << " " << __LINE__ );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glColor4f( 0.5f, 0.5f, 0.5f, 0.0f );
    glLineStipple( 1, 0x00FF );
    glEnable( GL_LINE_STIPPLE );
    mrv::media fg = _view->foreground();
    CMedia* img = fg->image();
    double zdeg = 0.0;
    if ( img ) zdeg = img->rot_z();
    draw_rectangle( r, _view->flip(), 0.0 );
    glDisable( GL_LINE_STIPPLE );
    if ( _view->display_window() && !_view->vr() )
        glEnable( GL_STENCIL_TEST );
}

void GLEngine::translate( const double x, const double y, const double z )
{
   glTranslated( x, y, z );
}

void prepare_subtitle( GLQuad* quad, mrv::image_type_ptr sub,
                       double _rotX, double _rotY,
                       unsigned texWidth, unsigned texHeight )
{
    glEnable( GL_BLEND );
    glDisable( GL_SCISSOR_TEST );
    quad->mask( 0 );
    quad->mask_value( -10 );
    quad->bind( sub );
    quad->gamma( 1.0 );
    // Handle rotation of cube/sphere
    quad->rot_x( _rotX );
    quad->rot_y( _rotY );
    quad->draw( texWidth, texHeight );
}

void prepare_image( CMedia* img, mrv::Recti& daw, unsigned texWidth,
                    unsigned texHeight, const mrv::ImageView* view )
{
    glRotated( img->rot_z(), 0, 0, 1 );
    glTranslated( img->x(), img->y(), 0 );
    glTranslated( double(daw.x() - img->eye_separation()),
                  double(-daw.y()), 0 );
    CHECK_GL;

    glScaled( double(texWidth), double(texHeight), 1.0 );

    CHECK_GL;
    glTranslated( 0.5, -0.5, 0.0 );
    CHECK_GL;
}

void GLEngine::draw_images( ImageList& images )
{
    TRACE( "" );

    if ( _YCbCr == NULL )
    {
        loadOpenGLShader();
        if ( ! _YCbCr ) return;
    }

    CHECK_GL;

    DBG( __FUNCTION__ << " " << __LINE__ );
  // Check if lut types changed since last time
  static int  RT_lut_old_algorithm = Preferences::kLutPreferCTL;
  static int ODT_lut_old_algorithm = Preferences::kLutPreferCTL;
  static int LUT_quality           = 2;
  static std::string ODT_ICC_old_profile;
  static std::string ODT_CTL_old_transform;
  static unsigned kNumStops = 10;

  if ( _view->use_lut() )
    {
      mrv::PreferencesUI* uiPrefs = _view->main()->uiPrefs;
      int RT_lut_algorithm = uiPrefs->RT_algorithm->value();
      int ODT_lut_algorithm = uiPrefs->ODT_algorithm->value();
      const char* ODT_ICC_profile = uiPrefs->uiODT_ICC_profile->text();
      int lut_quality = uiPrefs->uiLUT_quality->value();
      unsigned num_stops = (unsigned)uiPrefs->uiPrefsNumStops->value();

      // Check if there was a change effecting lut.
      if ( ( RT_lut_algorithm != RT_lut_old_algorithm ) ||
           ( ODT_lut_algorithm != ODT_lut_old_algorithm ) ||
           ( ODT_ICC_old_profile != ODT_ICC_profile ) ||
           ( ODT_CTL_old_transform != mrv::Preferences::ODT_CTL_transform ) ||
           ( LUT_quality != lut_quality ) ||
           ( kNumStops != num_stops) )
        {
          RT_lut_old_algorithm = RT_lut_algorithm;
          ODT_lut_old_algorithm = ODT_lut_algorithm;
          if ( ODT_ICC_profile )
            ODT_ICC_old_profile = ODT_ICC_profile;
          else
            ODT_ICC_old_profile.clear();

          ODT_CTL_old_transform = mrv::Preferences::ODT_CTL_transform;

          refresh_luts();

          if ( LUT_quality != lut_quality ||
               kNumStops != num_stops )
            {
                LUT_quality = lut_quality;
                kNumStops = num_stops;
                mrv::GLLut3d::clear();
            }
        }

    }


    TRACE( "" );
    if ( _view->normalize() )
    {
        minmax(); // calculate min-max
        minmax( _normMin, _normMax ); // retrieve them
    }


    TRACE( "" );
    size_t num_quads = 0;
    ImageList::iterator i = images.begin();
    ImageList::iterator e = images.end();

    for ( ; i != e; ++i )
    {
        const Image_ptr& img = *i;
        bool stereo = img->stereo_output() != CMedia::kNoStereo;
        if ( img->has_subtitle() ) num_quads += 1 + stereo;
        if ( img->has_picture()  ) ++num_quads;
        if ( stereo )    ++num_quads;
    }

    TRACE( "" );

    CHECK_GL;

    if ( num_quads > _quads.size() )
    {
        ImageView::VRType t = _view->vr();
        if ( t == ImageView::kVRSphericalMap )
        {
            alloc_spheres( num_quads );
        }
        else if ( t == ImageView::kVRCubeMap )
        {
            alloc_cubes( num_quads );
        }
        else
        {
            alloc_quads( num_quads );
        }
        for ( i = images.begin(); i != e; ++i )
        {
            const Image_ptr& img = *i;
            img->image_damage( img->image_damage() | CMedia::kDamageContents );
        }
    }



    glColor4f(1.0f,1.0f,1.0f,1.0f);



    TRACE( "" );

    double x = _view->spin_x();
    double y = _view->spin_y();
    if ( x >= 1000.0 )  // dummy value used to reset view
    {
        ImageView* v = const_cast< ImageView* >( _view );
        v->spin_x( 0.0 );
        v->spin_y( 0.0 );
        _rotX = _rotY = 0.0;
    }
    else
    {
        _rotX += x;
        _rotY += y;
    }

    QuadList::iterator q = _quads.begin();
    assert( q != _quads.end() );

    e = images.end();

    const Image_ptr& fg = images.back();
    const Image_ptr& bg = images.front();

    glDisable( GL_BLEND );
    CHECK_GL;


    for ( i = images.begin(); i != e; ++i, ++q )
    {
        const Image_ptr& img = *i;
        mrv::image_type_ptr pic = img->hires();
        if (!pic)  continue;

	DBG( "draw image " << img->name() );

        CMedia::StereoOutput stereo = img->stereo_output();
        const boost::int64_t& frame = pic->frame();

        mrv::Recti dpw = img->display_window(frame);
        mrv::Recti daw = img->data_window(frame);


        if ( stereo & CMedia::kStereoRight )
        {
            dpw = img->display_window2(frame);
            daw = img->data_window2(frame);
        }

      // Handle background image size
        if ( fg != img && stereo == CMedia::kNoStereo )
        {
            mrv::PreferencesUI* uiPrefs = _view->main()->uiPrefs;
            if ( uiPrefs->uiPrefsResizeBackground->value() == 0 )
            {   // DO NOT SCALE BG IMAGE
                texWidth = dpw.w();
                texHeight = dpw.h();
                const mrv::Recti& dp = fg->display_window();
                daw.x( img->x() + daw.x() );
                daw.y( daw.y() - img->y() );
                dpw.x( daw.x() );
                dpw.y( daw.y() );
            }
            else
            {
                // NOT display_window(frame)
                const mrv::Recti& dp = fg->display_window();
                texWidth = dp.w();
                texHeight = dp.h();
            }
        }
        else
        {
            texWidth = daw.w();
            texHeight = daw.h();
        }

        if ( texWidth == 0 ) texWidth = fg->width();
        if ( texHeight == 0 ) texHeight = fg->height();

        texWidth *= img->scale_x();
        texHeight *= img->scale_y();

        ImageView::FlipDirection flip = _view->flip();

        set_matrix( flip );

        if ( flip && !_view->vr() )
        {
            const mrv::Recti& dp = fg->display_window();
            double x = 0.0, y = 0.0;
            if ( flip & ImageView::kFlipVertical )   x = (double)-dp.w();
            if ( flip & ImageView::kFlipHorizontal ) y = (double)dp.h();
            glTranslated( x, y, 0.0f );
        }


        if ( dpw != daw && ! _view->vr() )
        {

            if ( _view->display_window() )
            {
                int x = img->x();
                int y = img->y();
                draw_square_stencil( dpw.x() - x, dpw.y() + y,
                                     dpw.w() + x, dpw.h() - y );
            }

            if ( _view->data_window()  )
            {
                double x = img->x(), y = img->y();
                mrv::Rectd r( daw.x() + x, daw.y() - y,
                              daw.w(), daw.h() );
                draw_data_window( r );
            }
        }

        glDisable( GL_BLEND );
        CHECK_GL;

        set_matrix( flip );

        if ( flip && !_view->vr() )
        {
            const mrv::Recti& dp = fg->display_window();
            double x = 0.0, y = 0.0;
            if ( flip & ImageView::kFlipVertical )   x = (double)-dp.w();
            if ( flip & ImageView::kFlipHorizontal ) y = (double)dp.h();
            glTranslated( x, y, 0.0f );
        }

        glMatrixMode(GL_MODELVIEW);
        CHECK_GL;
        glPushMatrix();
        CHECK_GL;

        if ( !_view->vr() )
        {
            prepare_image( img, daw, texWidth, texHeight, _view );
        }

        GLQuad* quad = *q;
        quad->minmax( _normMin, _normMax );
        quad->image( img );
        // Handle rotation of cube/sphere
        quad->rot_x( _rotX );
        quad->rot_y( _rotY );

        if ( _view->use_lut() )
        {
            if ( img->image_damage() & CMedia::kDamageLut )
                quad->clear_lut();

            quad->lut( img );

            if ( stereo != CMedia::kNoStereo )
            {
                if ( img->image_damage() & CMedia::kDamageLut )
                    (*(q+1))->clear_lut();
                (*(q+1))->lut( img );
            }

            img->image_damage( img->image_damage() & ~CMedia::kDamageLut  );
        }

        if ( i+1 == e ) wipe_area();

        float g = img->gamma();

        int mask = 0;

        if ( stereo != CMedia::kNoStereo &&
             img->left() && img->right() )
        {
            if ( stereo & CMedia::kStereoRight )
            {
                pic = img->right();
                CMedia* right = img->right_eye();
                if ( right ) g = right->gamma();
            }
            else
            {
                pic = img->left();
            }


            if ( stereo & CMedia::kStereoAnaglyph )
                glColorMask( GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE );
            else
                glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
            CHECK_GL;

#ifdef USE_STEREO_GL
            if ( stereo & CMedia::kStereoOpenGL )
            {
                glDrawBuffer( GL_BACK_LEFT );
                CHECK_GL;
            }
#endif

            quad->mask( 0 );
            quad->mask_value( 10 );
            if ( stereo & CMedia::kStereoInterlaced )
            {
                if ( stereo == CMedia::kStereoInterlaced )
                    mask = 1; // odd even rows
                else if ( stereo == CMedia::kStereoInterlacedColumns )
                    mask = 2; // odd even columns
                else if ( stereo == CMedia::kStereoCheckerboard )
                    mask = 3; // checkerboard
                quad->mask( mask );
                quad->mask_value( 1 );
            }

            glDisable( GL_BLEND );
            CHECK_GL;
            if ( img->image_damage() & CMedia::kDamageContents )
            {
                if ( stereo & CMedia::kStereoRight )
                {
                    quad->right( true );
                }
                else
                {
                    quad->right( false );
                }
                CMedia::Mutex& mtx = img->video_mutex();
                SCOPED_LOCK( mtx );
                if ( pic->format() >= image_type::kYByRy420 )
                    quad->shader( GLEngine::YByRyShader() );
                else
                    quad->shader( GLEngine::YCbCrShader() );
                quad->bind( pic );
            }
            quad->gamma( g );
            quad->draw( texWidth, texHeight );


            if ( img->has_subtitle() )
            {
                CMedia::Mutex& mtx = img->subtitle_mutex();
                SCOPED_LOCK( mtx );
                image_type_ptr sub = img->subtitle();
                if ( sub )
                {
                    prepare_subtitle( *q++, sub, _rotX, _rotY,
                                      texWidth, texHeight );
                }
                img->image_damage( img->image_damage() &
                                   ~CMedia::kDamageSubtitle );
            }

            ++q;
            quad = *q;
            quad->minmax( _normMin, _normMax );
            quad->image( img );
            // Handle rotation of cube/sphere
            quad->rot_x( _rotX );
            quad->rot_y( _rotY );

            if ( stereo != CMedia::kStereoLeft &&
                 stereo != CMedia::kStereoRight )
            {
                CHECK_GL;
                glMatrixMode( GL_MODELVIEW );
                CHECK_GL;
                glPopMatrix();
                CHECK_GL;

                if ( ( stereo & CMedia::kStereoSideBySide ) ==
                     CMedia::kStereoSideBySide )
                {
                    glTranslated( dpw.w(), 0, 0 );
                }
                else if ( ( stereo & CMedia::kStereoBottomTop ) ==
                          CMedia::kStereoBottomTop )
                {
                    glTranslated( 0, -dpw.h(), 0 );
                }

                CHECK_GL;
                mrv::Recti dpw2 = img->display_window2(frame);
                mrv::Recti daw2 = img->data_window2(frame);

                if ( stereo & CMedia::kStereoRight )
                {
                    dpw2 = img->display_window(frame);
                    daw2 = img->data_window(frame);
                }


                CHECK_GL;
                glMatrixMode(GL_MODELVIEW);
                CHECK_GL;
                glPushMatrix();
                CHECK_GL;


                if ( dpw2 != daw2 )
                {
                    if ( _view->display_window() &&
                         ( !( stereo & CMedia::kStereoAnaglyph ) &&
                           !( stereo & CMedia::kStereoInterlaced ) &&
                           !( _view->vr() ) ) )
                    {
                        int x = img->x();
                        int y = img->y();
                        draw_square_stencil( dpw.x() + x,
                                             dpw.y() - y, dpw.w() - x,
                                             dpw.h() + y );
                    }

                    if ( _view->data_window() )
                    {
                        double x = img->x(), y = img->y();
                        if ( (stereo & CMedia::kStereoSideBySide) ==
                             CMedia::kStereoSideBySide )
                            x += dpw2.w();
                        else if ( (stereo & CMedia::kStereoBottomTop) ==
                                  CMedia::kStereoBottomTop )
                            y -= dpw2.h();

                        mrv::Rectd r( daw2.x() + x, daw2.y() - y,
                                      daw2.w(), daw2.h() );
                        draw_data_window( r );
                    }
                }

                g = img->gamma();

                if ( stereo & CMedia::kStereoRight )
                {
                    pic = img->left();
                }
                else
                {
                    pic = img->right();
                    CMedia* right = img->right_eye();
                    if ( right ) g = right->gamma();
                }

                if ( daw2.w() > 0 )
                {
                    texWidth = daw2.w();
                    texHeight = daw2.h();
                }
                else
                {
                    texWidth = pic->width();
                    texHeight = pic->height();
                }

                prepare_image( img, daw2, texWidth, texHeight, _view );
            }
        }
        else if ( img->hires() || img->has_subtitle() )
        {
            stereo = CMedia::kNoStereo;
            pic = img->hires();

            if ( shader_type() == kNone && img->stopped() &&
                 pic->pixel_type() != image_type::kByte )
            {
                pic = display( pic, img );
            }

      }

      if ( stereo & CMedia::kStereoAnaglyph )
          glColorMask( GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE );
      else
          glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

#ifdef USE_STEREO_GL
      if ( stereo & CMedia::kStereoOpenGL )
      {
          glDrawBuffer( GL_BACK_RIGHT );
          CHECK_GL;
      }
#endif

      quad->mask( 0 );
      quad->mask_value( 10 );
      if ( stereo & CMedia::kStereoInterlaced )
      {
          if ( stereo == CMedia::kStereoInterlaced )
              mask = 1; // odd even rows
          else if ( stereo == CMedia::kStereoInterlacedColumns )
              mask = 2; // odd even columns
          else if ( stereo == CMedia::kStereoCheckerboard )
              mask = 3; // checkerboard
          quad->mask( mask );
          quad->mask_value( 0 );
          glEnable( GL_BLEND );
      }

      if ( fg == img && bg != fg &&
           _view->show_background() ) glEnable( GL_BLEND );

      if ( img->image_damage() & CMedia::kDamageContents )
      {
          if ( stereo )
          {
              bool rightView = false;
              if ( stereo & CMedia::kStereoRight )
              {
                  rightView = false;
              }
              else
              {
                  rightView = true;
              }

              if ( stereo == CMedia::kStereoRight )
                  rightView = true;
              else if ( stereo == CMedia::kStereoLeft )
                  rightView = false;
              quad->right( rightView );
          }
          CMedia::Mutex& mtx = img->video_mutex();
          SCOPED_LOCK( mtx );
          if ( pic->format() >= image_type::kYByRy420 )
              quad->shader( GLEngine::YByRyShader() );
          else
              quad->shader( GLEngine::YCbCrShader() );
          quad->bind( pic );
          img->image_damage( img->image_damage() & ~CMedia::kDamageContents );
      }

      quad->gamma( g );
      quad->draw( texWidth, texHeight );

      if ( ! pic->valid() && pic->channels() >= 2 &&
           Preferences::missing_frame == Preferences::kScratchedRepeatFrame )
      {
          glDisable( GL_DEPTH );
          glDisable( GL_STENCIL_TEST );
          glDisable( GL_TEXTURE_2D );
          glDisable( GL_TEXTURE_3D );
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glLineWidth( 50.0 );
          glColor4f( 1.0f, 0.f, 0.f, 0.5f );
          glBegin(GL_LINES);
          glVertex2d( -0.5, -0.5 );
          glVertex2d( 0.5, 0.5 );
          glVertex2d( 0.5, -0.5 );
          glVertex2d( -0.5, 0.5 );
          glEnd();
          glDisable( GL_BLEND );
          glEnable( GL_DEPTH );
          glEnable( GL_STENCIL_TEST );
          glEnable( GL_TEXTURE_2D );
          glEnable( GL_TEXTURE_3D );
      }


      if ( ( _view->action_mode() == ImageView::kMovePicture ||
             _view->action_mode() == ImageView::kScalePicture ) &&
           _view->selected_image() == img )
      {
          mrv::Rectd r( img->x() + dpw.x(), dpw.y() - img->y(),
                        dpw.w() * img->scale_x(), dpw.h() * img->scale_y() );
          draw_selection_marquee( r );
      }


      if ( img->has_subtitle() )
        {
            image_type_ptr sub = img->subtitle();
            if ( sub )
            {
                prepare_subtitle( *q++, sub, _rotX, _rotY,
                                  texWidth, texHeight );
            }
            img->image_damage( img->image_damage() & ~CMedia::kDamageSubtitle );
        }

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }

  glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
  glDisable( GL_SCISSOR_TEST );
  glDisable( GL_BLEND );
  FLUSH_GL_ERRORS;
}


void GLEngine::draw_shape( GLShape* const shape )
{
   double zoomX = _view->zoom();
    DBG( __FUNCTION__ << " " << __LINE__ );
    if ( _view->ghost_previous() )
    {
        if ( shape->frame == _view->frame() - 1 )
        {
            float a = shape->a;
            shape->a *= 0.25f;
            shape->draw(zoomX);
            shape->a = a;
            return;
         }
      }

      if ( _view->ghost_next() )
      {
         if ( shape->frame == _view->frame() + 1 )
         {
            float a = shape->a;
            shape->a *= 0.25f;
            shape->draw(zoomX);
            shape->a = a;
            return;
         }
      }

      if ( shape->frame == MRV_NOPTS_VALUE ||
           shape->frame == _view->frame() )
      {
         shape->draw(zoomX);
      }
}


void GLEngine::draw_annotation( const GLShapeList& shapes )
{
    DBG( __FUNCTION__ << " " << __LINE__ );
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();

   double pr = 1.0;
   if ( _view->main()->uiPixelRatio->value() ) pr /= _view->pixel_ratio();

   double zoomX = _view->zoom();
   double zoomY = _view->zoom();

   double tw = double( texWidth  ) / 2.0;
   double th = double( texHeight ) / 2.0;

   double sw = ((double)_view->w() - texWidth  * zoomX) / 2;
   double sh = ((double)_view->h() - texHeight * zoomY) / 2;

   glTranslated( (tw + _view->offset_x()) * zoomX + sw,
                 (th + _view->offset_y()) * zoomY + sh, 0);

   glScaled(zoomX, zoomY * pr, 1.0f);


   glClear(GL_STENCIL_BUFFER_BIT);

   glEnable( GL_STENCIL_TEST );

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glEnable( GL_LINE_SMOOTH );
   glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

   {
       GLShapeList::const_reverse_iterator i = shapes.rbegin();
       GLShapeList::const_reverse_iterator e = shapes.rend();


       for ( ; i != e; ++i )
       {
           GLShape* shape = (*i).get();
           draw_shape( shape );
       }

   }

   glDisable(GL_BLEND);
   glDisable(GL_STENCIL_TEST);
}


void GLEngine::wipe_area()
{
  int  w = _view->w();
  int  h = _view->h();

    DBG( __FUNCTION__ << " " << __LINE__ );
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
"# cgc version 3.1.0013, build date Apr 24 2012\n"
"# command line args: -I/media/gga/Datos/code/applications/mrViewer/shaders -profile arbfp1\n"
"# source file: rgba.cg\n"
"#vendor NVIDIA Corporation\n"
"#version 3.1.0.13\n"
"#profile arbfp1\n"
"#program main\n"
"#semantic main.fgImage : TEXUNIT0"
"#semantic main.lut : TEXUNIT3\n"
"#semantic main.mask\n"
"#semantic main.mask_value\n"
"#semantic main.height\n"
"#semantic main.width\n"
"#semantic main.gain\n"
"#semantic main.gamma\n"
"#semantic main.channel\n"
"#semantic main.premult\n"
"#semantic main.unpremult\n"
"#semantic main.enableNormalization\n"
"#semantic main.normMin\n"
"#semantic main.normSpan\n"
"#semantic main.enableLut\n"
"#semantic main.lutF\n"
"#semantic main.lutMin\n"
"#semantic main.lutMax\n"
"#semantic main.scale\n"
"#semantic main.offset\n"
"#semantic main.lutM\n"
"#semantic main.lutT\n"
"#var float2 tc : $vin.TEXCOORD0 : TEX0 : 0 : 1\n"
"#var sampler2D fgImage : TEXUNIT0 : texunit 0 : 1 : 1\n"
"#var sampler3D lut : TEXUNIT3 : texunit 3 : 2 : 1\n"
"#var int mask :  : c[0] : 3 : 1\n"
"#var int mask_value :  : c[1] : 4 : 1\n"
"#var int height :  : c[2] : 5 : 1\n"
"#var int width :  : c[3] : 6 : 1\n"
"#var half gain :  : c[4] : 7 : 1\n"
"#var half gamma :  : c[5] : 8 : 1\n"
"#var int channel :  : c[6] : 9 : 1\n"
"#var bool premult :  : c[7] : 10 : 1\n"
"#var bool unpremult :  : c[8] : 11 : 1\n"
"#var bool enableNormalization :  : c[9] : 12 : 1\n"
"#var half normMin :  : c[10] : 13 : 1\n"
"#var half normSpan :  : c[11] : 14 : 1\n"
"#var bool enableLut :  : c[12] : 15 : 1\n"
"#var bool lutF :  : c[13] : 16 : 1\n"
"#var half lutMin :  : c[14] : 17 : 1\n"
"#var half lutMax :  : c[15] : 18 : 1\n"
"#var half scale :  : c[16] : 19 : 1\n"
"#var half offset :  : c[17] : 20 : 1\n"
"#var half lutM :  : c[18] : 21 : 1\n"
"#var half lutT :  : c[19] : 22 : 1\n"
"#var float4 main.pixel : $vout.COLOR : COL : -1 : 1\n"
"#const c[20] = 0.5 0.33333334 1 0\n"
"#const c[21] = 2 3 1000 4\n"
"#const c[22] = 5 6 0.00010001659 2.718282\n"
"#const c[23] = 2.71875 0.69335938\n"
"#default mask = 0\n"
"#default mask_value = 0\n"
"#default height = 256\n"
"#default width = 256\n"
"#default gain = 1\n"
"#default gamma = 1.0\n"
"#default channel = 0\n"
"#default premult = 0\n"
"#default unpremult = 0\n"
"#default enableNormalization = 0\n"
"#default normMin = 0\n"
"#default normSpan = 1\n"
"#default enableLut = 0\n"
"#default lutF = 0\n"
"#default scale = 1\n"
"#default offset = 0\n"
"PARAM c[24] = { program.local[0..19],\n"
"		{ 0.5, 0.33333334, 1, 0 },\n"
"		{ 2, 3, 1000, 4 },\n"
"		{ 5, 6, 0.00010001659, 2.718282 },\n"
"		{ 2.71875, 0.69335938 } };\n"
"TEMP R0;\n"
"TEMP R1;\n"
"TEMP R2;\n"
"TEMP R3;\n"
"TEMP R4;\n"
"TEMP R5;\n"
"TEMP R6;\n"
"TEMP R7;\n"
"TEX R0, fragment.texcoord[0], texture[0], 2D;\n"
"RCP R3.w, c[13].x;\n"
"RCP R1.w, c[11].x;\n"
"ADD R1.xyz, R0, -c[10].x;\n"
"MUL R1.xyz, R1, R1.w;\n"
"CMP R0.xyz, -c[9].x, R1, R0;\n"
"MUL R3.xyz, R0, c[4].x;\n"
"MIN R0.xyz, R3, c[15].x;\n"
"MAX R0.xyz, R0, c[14].x;\n"
"MOV R1.w, c[19].x;\n"
"LG2 R0.x, R0.x;\n"
"LG2 R0.y, R0.y;\n"
"LG2 R0.z, R0.z;\n"
"MUL R0.xyz, R0, c[18].x;\n"
"MAD R0.xyz, R0, c[23].y, R1.w;\n"
"MUL R1.xyz, R0, c[13].x;\n"
"FLR R0.xyz, R1;\n"
"ADD R4.xyz, -R0, R1;\n"
"ADD R2.xyz, R0, c[20].z;\n"
"MUL R7.xyz, R3.w, R2;\n"
"MUL R2.xyz, R0, R3.w;\n"
"TEX R1.xyz, R7, texture[3], 3D;\n"
"ADD R2.w, -R4.x, c[20].z;\n"
"ADD R3.w, -R4.y, c[20].z;\n"
"MUL R1.xyz, R4.x, R1;\n"
"MOV R0.x, R2;\n"
"MOV R0.yz, R7;\n"
"TEX R0.xyz, R0, texture[3], 3D;\n"
"MAD R1.xyz, R2.w, R0, R1;\n"
"MUL R5.xyz, R4.y, R1;\n"
"TEX R1.xyz, R2, texture[3], 3D;\n"
"MOV R0.y, R2;\n"
"MOV R0.xz, R7;\n"
"TEX R0.xyz, R0, texture[3], 3D;\n"
"MUL R0.xyz, R4.x, R0;\n"
"MAD R6.xyz, R2.w, R1, R0;\n"
"MAD R5.xyz, R6, R3.w, R5;\n"
"MOV R6.yz, R2;\n"
"MOV R6.x, R7;\n"
"MOV R0.z, R2;\n"
"MOV R0.xy, R7;\n"
"TEX R0.xyz, R0, texture[3], 3D;\n"
"MOV R2.y, R7;\n"
"MUL R0.xyz, R4.x, R0;\n"
"TEX R2.xyz, R2, texture[3], 3D;\n"
"MAD R0.xyz, R2, R2.w, R0;\n"
"TEX R6.xyz, R6, texture[3], 3D;\n"
"MUL R2.xyz, R4.x, R6;\n"
"MAD R1.xyz, R2.w, R1, R2;\n"
"MUL R0.xyz, R4.y, R0;\n"
"MAD R0.xyz, R1, R3.w, R0;\n"
"MUL R1.xyz, R4.z, R5;\n"
"ADD R2.y, -R4.z, c[20].z;\n"
"MAD R0.xyz, R0, R2.y, R1;\n"
"MOV R2.x, c[13];\n"
"MUL R1.x, R2, c[12];\n"
"CMP R2.xyz, -R1.x, R0, R3;\n"
"POW R0.x, c[23].x, R2.x;\n"
"POW R0.z, c[23].x, R2.z;\n"
"POW R0.y, c[23].x, R2.y;\n"
"CMP R0.xyz, -R1.x, R0, R2;\n"
"MIN R1.xyz, R0, c[15].x;\n"
"MAX R1.xyz, R1, c[14].x;\n"
"LG2 R1.x, R1.x;\n"
"LG2 R1.z, R1.z;\n"
"LG2 R1.y, R1.y;\n"
"MUL R2.xyz, R1, c[18].x;\n"
"ABS R1.x, c[13];\n"
"MAD R2.xyz, R2, c[23].y, R1.w;\n"
"CMP R1.x, -R1, c[20].w, c[20].z;\n"
"MUL R1.w, R1.x, c[12].x;\n"
"CMP R1.xyz, -R1.w, R2, R0;\n"
"MOV R2.w, c[17].x;\n"
"MAD R0.xyz, R1, c[16].x, R2.w;\n"
"TEX R0.xyz, R0, texture[3], 3D;\n"
"POW R0.x, c[22].w, R0.x;\n"
"POW R0.z, c[22].w, R0.z;\n"
"POW R0.y, c[22].w, R0.y;\n"
"CMP R0.xyz, -R1.w, R0, R1;\n"
"RCP R1.y, R0.w;\n"
"SLT R1.x, c[22].z, R0.w;\n"
"MUL R2.xyz, R0, R1.y;\n"
"MUL R1.x, R1, c[8];\n"
"CMP R0.xyz, -R1.x, R2, R0;\n"
"MOV R2.xyz, c[21].xyww;\n"
"ADD R1.y, -R2.x, c[6].x;\n"
"MOV R1.z, c[20];\n"
"ADD R1.x, -R1.z, c[6];\n"
"ABS R1.y, R1;\n"
"ADD R2.x, -R2, c[0];\n"
"ADD R1.z, -R1, c[0].x;\n"
"ABS R1.z, R1;\n"
"ABS R1.x, R1;\n"
"CMP R1.y, -R1, c[20].w, c[20].z;\n"
"ADD R2.z, -R2, c[6].x;\n"
"ABS R2.x, R2;\n"
"POW R0.y, R0.y, c[5].x;\n"
"POW R0.z, R0.z, c[5].x;\n"
"POW R0.x, R0.x, c[5].x;\n"
"CMP R0.xyz, -R1.x, R0, R0.x;\n"
"CMP R1.x, -R1, c[20].w, c[20].z;\n"
"ABS R1.x, R1;\n"
"CMP R1.x, -R1, c[20].w, c[20].z;\n"
"MUL R1.w, R1.x, R1.y;\n"
"CMP R0.xyz, -R1.w, R0.y, R0;\n"
"ADD R1.w, -R2.y, c[6].x;\n"
"ADD R2.y, -R2, c[0].x;\n"
"ABS R3.y, R2;\n"
"CMP R2.y, -R2.x, c[20].w, c[20].z;\n"
"CMP R3.w, -R3.y, c[20], c[20].z;\n"
"MUL R3.y, fragment.texcoord[0], c[2].x;\n"
"ABS R1.y, R1;\n"
"CMP R1.y, -R1, c[20].w, c[20].z;\n"
"ABS R1.w, R1;\n"
"MUL R1.x, R1, R1.y;\n"
"CMP R1.w, -R1, c[20], c[20].z;\n"
"MUL R1.y, R1.x, R1.w;\n"
"CMP R0.xyz, -R1.y, R0.z, R0;\n"
"ABS R1.y, R1.w;\n"
"ABS R1.w, R2.z;\n"
"CMP R1.y, -R1, c[20].w, c[20].z;\n"
"MUL R2.z, R1.x, R1.y;\n"
"CMP R1.w, -R1, c[20], c[20].z;\n"
"MUL R1.x, R2.z, R1.w;\n"
"CMP R0.xyz, -R1.x, R0.w, R0;\n"
"MOV R1.xy, c[22];\n"
"ADD R2.w, -R1.x, c[6].x;\n"
"ABS R1.x, R1.w;\n"
"ABS R1.w, R2;\n"
"CMP R1.x, -R1, c[20].w, c[20].z;\n"
"CMP R2.x, -R1.z, c[20].w, c[20].z;\n"
"FLR R4.y, R3;\n"
"MUL R3.x, R0, c[20];\n"
"CMP R1.w, -R1, c[20], c[20].z;\n"
"MUL R1.x, R2.z, R1;\n"
"MUL R2.z, R1.x, R1.w;\n"
"CMP R0.x, -R2.z, R3, R0;\n"
"MAD R2.w, R0, c[20].x, R0.x;\n"
"CMP R0.x, -R2.z, R2.w, R0;\n"
"ADD R2.z, R0.x, R0.y;\n"
"ADD R2.z, R2, R0;\n"
"ABS R3.x, R2.y;\n"
"ABS R2.w, R2.x;\n"
"CMP R2.w, -R2, c[20], c[20].z;\n"
"CMP R3.x, -R3, c[20].w, c[20].z;\n"
"MUL R3.z, R2.w, R3.x;\n"
"MUL R3.x, fragment.texcoord[0], c[3];\n"
"FLR R4.x, R3;\n"
"ADD R4.z, R4.x, R4.y;\n"
"MUL R3.z, R3, R3.w;\n"
"SLT R3.w, R4.x, -R4.y;\n"
"MUL R4.z, R4, c[20].x;\n"
"ABS R4.x, R4.z;\n"
"ABS R3.w, R3;\n"
"FRC R4.x, R4;\n"
"CMP R3.w, -R3, c[20], c[20].z;\n"
"MUL R2.y, R2.w, R2;\n"
"MUL R4.x, R4, c[21];\n"
"MUL R3.w, R3.z, R3;\n"
"CMP R3.w, -R3, R4.x, -R4.x;\n"
"SLT R4.x, R3, c[20].w;\n"
"ABS R2.w, R4.x;\n"
"SLT R4.x, R3.y, c[20].w;\n"
"CMP R2.w, -R2, c[20], c[20].z;\n"
"ABS R4.x, R4;\n"
"MUL R2.w, R2.y, R2;\n"
"CMP R4.x, -R4, c[20].w, c[20].z;\n"
"MUL R4.y, R3, c[20].x;\n"
"MUL R3.y, R2.x, R4.x;\n"
"MUL R2.x, R3, c[20];\n"
"ABS R4.x, R4.y;\n"
"FRC R3.x, R4;\n"
"ABS R2.x, R2;\n"
"MUL R3.x, R3, c[21];\n"
"CMP R3.x, -R3.y, R3, -R3;\n"
"ABS R3.y, R3.x;\n"
"FRC R2.x, R2;\n"
"MUL R2.x, R2, c[21];\n"
"CMP R2.x, -R2.w, R2, -R2;\n"
"SLT R2.w, R3.x, c[20];\n"
"FLR R3.y, R3;\n"
"CMP R3.x, -R2.w, -R3.y, R3.y;\n"
"ABS R2.w, R2.x;\n"
"CMP R3.x, -R1.z, c[21].z, R3;\n"
"SLT R1.z, R2.x, c[20].w;\n"
"FLR R2.w, R2;\n"
"CMP R1.z, -R1, -R2.w, R2.w;\n"
"CMP R2.x, -R2.y, R1.z, R3;\n"
"SLT R1.z, R3.w, c[20];\n"
"CMP R1.z, -R3, R1, R2.x;\n"
"ADD R2.x, -R1.y, c[6];\n"
"ABS R1.y, R1.w;\n"
"CMP R1.y, -R1, c[20].w, c[20].z;\n"
"ABS R1.w, R2.x;\n"
"MUL R1.x, R1, R1.y;\n"
"CMP R1.w, -R1, c[20], c[20].z;\n"
"MUL R1.x, R1, R1.w;\n"
"MUL R2.z, R2, c[20].y;\n"
"CMP R0.xyz, -R1.x, R2.z, R0;\n"
"ADD R1.x, R1.z, -c[1];\n"
"ABS R1.x, R1;\n"
"CMP R0, -R1.x, R0, c[20].w;\n"
"MUL R1.xyz, R0, R0.w;\n"
"CMP result.color.xyz, -c[7].x, R1, R0;\n"
"MOV result.color.w, R0;\n"
"END\n"
"# 205 instructions, 8 R-regs\n";

const char* NVShader =
"!!FP1.0\n"
"# cgc version 3.1.0013, build date Apr 24 2012\n"
"# command line args: -I/media/gga/Datos/code/applications/mrViewer/shaders -profile fp30\n"
"# source file: rgba.cg\n"
"#vendor NVIDIA Corporation\n"
"#version 3.1.0.13\n"
"#profile fp30\n"
"#program main\n"
"#semantic main.fgImage : TEXUNIT0\n"
"#semantic main.lut : TEXUNIT3\n"
"#semantic main.mask\n"
"#semantic main.mask_value\n"
"#semantic main.height\n"
"#semantic main.width\n"
"#semantic main.gain\n"
"#semantic main.gamma\n"
"#semantic main.channel\n"
"#semantic main.premult\n"
"#semantic main.unpremult\n"
"#semantic main.enableNormalization\n"
"#semantic main.normMin\n"
"#semantic main.normSpan\n"
"#semantic main.enableLut\n"
"#semantic main.lutF\n"
"#semantic main.lutMin\n"
"#semantic main.lutMax\n"
"#semantic main.scale\n"
"#semantic main.offset\n"
"#semantic main.lutM\n"
"#semantic main.lutT\n"
"#var float2 tc : $vin.TEXCOORD0 : TEX0 : 0 : 1\n"
"#var sampler2D fgImage : TEXUNIT0 : texunit 0 : 1 : 1\n"
"#var sampler3D lut : TEXUNIT3 : texunit 3 : 2 : 1\n"
"#var int mask :  : mask : 3 : 1\n"
"#var int mask_value :  : mask_value : 4 : 1\n"
"#var int height :  : height : 5 : 1\n"
"#var int width :  : width : 6 : 1\n"
"#var half gain :  : gain : 7 : 1\n"
"#var half gamma :  : gamma : 8 : 1\n"
"#var int channel :  : channel : 9 : 1\n"
"#var bool premult :  : premult : 10 : 1\n"
"#var bool unpremult :  : unpremult : 11 : 1\n"
"#var bool enableNormalization :  : enableNormalization : 12 : 1\n"
"#var half normMin :  : normMin : 13 : 1\n"
"#var half normSpan :  : normSpan : 14 : 1\n"
"#var bool enableLut :  : enableLut : 15 : 1\n"
"#var bool lutF :  : lutF : 16 : 1\n"
"#var half lutMin :  : lutMin : 17 : 1\n"
"#var half lutMax :  : lutMax : 18 : 1\n"
"#var half scale :  : scale : 19 : 1\n"
"#var half offset :  : offset : 20 : 1\n"
"#var half lutM :  : lutM : 21 : 1\n"
"#var half lutT :  : lutT : 22 : 1\n"
"#var half4 main.pixel : $vout.COLOR : COL : -1 : 1\n"
"#default mask = 0\n"
"#default mask_value = 0\n"
"#default height = 256\n"
"#default width = 256\n"
"#default gain = 1\n"
"#default gamma = 1.0\n"
"#default channel = 0\n"
"#default premult = 0\n"
"#default unpremult = 0\n"
"#default enableNormalization = 0\n"
"#default normMin = 0\n"
"#default normSpan = 1\n"
"#default enableLut = 0\n"
"#default lutF = 0\n"
"#default scale = 1\n"
"#default offset = 0\n"
"DECLARE enableNormalization = {0};\n"
"DECLARE normMin = {0};\n"
"DECLARE normSpan = {1};\n"
"DECLARE gain = {1};\n"
"DECLARE enableLut = {0};\n"
"DECLARE lutF = {0};\n"
"DECLARE lutMax;\n"
"DECLARE lutMin;\n"
"DECLARE lutT;\n"
"DECLARE lutM;\n"
"DECLARE scale = {1};\n"
"DECLARE offset = {0};\n"
"DECLARE unpremult = {0};\n"
"DECLARE gamma = {1.0};\n"
"DECLARE channel = {0};\n"
"DECLARE mask = {0};\n"
"DECLARE height = {256};\n"
"DECLARE width = {256};\n"
"DECLARE mask_value = {0};\n"
"DECLARE premult = {0};\n"
"TEX   H1, f[TEX0], TEX0, 2D;\n"
"ADDH  H0.xyz, H1, -normMin.x;\n"
"MULR  R1.x, f[TEX0], width;\n"
"MULR  R1.y, f[TEX0], height.x;\n"
"RCPH  H0.w, normSpan.x;\n"
"MOVXC RC.x, enableNormalization;\n"
"MULH  H1.xyz(NE.x), H0, H0.w;\n"
"MULH  H1.xyz, H1, gain.x;\n"
"MINH  H0.xyz, H1, lutMax.x;\n"
"MAXH  H0.xyz, H0, lutMin.x;\n"
"MOVH  H2.w, lutT.x;\n"
"RCPH  H3.w, lutF.x;\n"
"MOVR  R0.w, {3}.x;\n"
"LG2H  H0.x, H0.x;\n"
"LG2H  H0.z, H0.z;\n"
"LG2H  H0.y, H0.y;\n"
"MULH  H0.xyz, H0, lutM.x;\n"
"MADH  H0.xyz, H0, {0.69335938}.x, H2.w;\n"
"MULH  H0.xyz, H0, lutF.x;\n"
"FLRH  H4.xyz, H0;\n"
"ADDH  H2.xyz, -H4, H0;\n"
"ADDH  H0.xyz, H4, {1}.x;\n"
"MULH  H0.xyw, H3.w, H0.yzzx;\n"
"MULH  H7.xyz, H4, H3.w;\n"
"TEX   H5.xyz, H0.wxyw, TEX3, 3D;\n"
"ADDH  H3.xyz, -H2, {1}.x;\n"
"MULH  H5.xyz, H2.x, H5;\n"
"MOVH  H4.yz, H0.xxyw;\n"
"MOVH  H4.x, H7;\n"
"TEX   H4.xyz, H4, TEX3, 3D;\n"
"MADH  H4.xyz, H3.x, H4, H5;\n"
"MULH  H6.xyz, H2.y, H4;\n"
"MOVH  H4.xy, H0.wxzw;\n"
"MOVH  H4.z, H7;\n"
"TEX   H4.xyz, H4, TEX3, 3D;\n"
"MULH  H5.xyz, H2.x, H4;\n"
"MOVH  H4.y, H0.x;\n"
"MOVH  H0.xz, H0.wyyw;\n"
"MOVH  H4.xz, H7;\n"
"TEX   H4.xyz, H4, TEX3, 3D;\n"
"MADH  H4.xyz, H4, H3.x, H5;\n"
"MOVH  H0.y, H7;\n"
"TEX   H0.xyz, H0, TEX3, 3D;\n"
"MULH  H5.xyz, H2.x, H0;\n"
"TEX   H0.xyz, H7, TEX3, 3D;\n"
"MADH  H5.xyz, H3.x, H0, H5;\n"
"MADH  H5.xyz, H3.y, H5, H6;\n"
"MOVH  H6.x, H0.w;\n"
"MOVX  H0.w, lutF.x;\n"
"MULXC HC.x, H0.w, enableLut;\n"
"MOVH  H6.yz, H7;\n"
"TEX   H6.xyz, H6, TEX3, 3D;\n"
"MULH  H6.xyz, H2.x, H6;\n"
"MULH  H4.xyz, H2.y, H4;\n"
"MADH  H0.xyz, H3.x, H0, H6;\n"
"MOVX  H0.w, {0}.x;\n"
"MADH  H0.xyz, H0, H3.y, H4;\n"
"MULH  H2.xyz, H2.z, H5;\n"
"MADH  H1.xyz(NE.x), H0, H3.z, H2;\n"
"MOVH  H2.xyz, H1;\n"
"POWH  H0.x, {2.71875}.x, H1.x;\n"
"POWH  H0.y, {2.71875}.x, H1.y;\n"
"POWH  H0.z, {2.71875}.x, H1.z;\n"
"MOVH  H2.xyz(NE.x), H0;\n"
"MINH  H0.xyz, H2, lutMax.x;\n"
"MAXH  H0.xyz, H0, lutMin.x;\n"
"SEQX  H0.w, lutF.x, H0;\n"
"MOVH  H1.xyz, H2;\n"
"MULXC HC.x, H0.w, enableLut;\n"
"LG2H  H0.x, H0.x;\n"
"LG2H  H0.z, H0.z;\n"
"LG2H  H0.y, H0.y;\n"
"MULH  H0.xyz, H0, lutM.x;\n"
"MADH  H1.xyz(NE.x), H0, {0.69335938}.x, H2.w;\n"
"MOVH  H0.x, offset;\n"
"MADH  H0.xyz, H1, scale.x, H0.x;\n"
"TEX   R0.xyz, H0, TEX3, 3D;\n"
"POWR  H0.x, {2.718282}.x, R0.x;\n"
"POWR  H0.y, {2.718282}.x, R0.y;\n"
"POWR  H0.z, {2.718282}.x, R0.z;\n"
"MOVH  H1.xyz(NE.x), H0;\n"
"MOVR  R0.x, {1};\n"
"MOVR  R0.y, {2}.x;\n"
"SGTH  H0.x, H1.w, {0.00010001659};\n"
"SEQR  H2.x, channel, R0.y;\n"
"MULXC HC.x, H0, unpremult;\n"
"RCPH  H0.y, H1.w;\n"
"MULH  H1.xyz(NE.x), H1, H0.y;\n"
"SEQR  H0.w, channel.x, R0.x;\n"
"MOVXC RC.x, H0.w;\n"
"POWH  H0.x, H1.x, gamma.x;\n"
"POWH  H0.z, H1.z, gamma.x;\n"
"POWH  H0.y, H1.y, gamma.x;\n"
"MOVH  H1.xyz, H0;\n"
"MOVH  H1.xyz(NE.x), H0.x;\n"
"SEQX  H0.w, H0, {0}.x;\n"
"MOVH  H0.xyz, H1;\n"
"MULXC HC.x, H0.w, H2;\n"
"MOVH  H0.xyz(NE.x), H1.y;\n"
"MOVH  H1.xyz, H0;\n"
"SEQX  H0.x, H2, {0};\n"
"MULX  H0.x, H0.w, H0;\n"
"SEQR  H0.y, channel.x, R0.w;\n"
"MULXC HC.x, H0, H0.y;\n"
"SEQX  H0.y, H0, {0}.x;\n"
"MOVH  H1.xyz(NE.x), H0.z;\n"
"MOVR  R0.z, {4}.x;\n"
"SEQR  H0.z, channel.x, R0;\n"
"MULX  H0.y, H0.x, H0;\n"
"MULXC HC.x, H0.y, H0.z;\n"
"MOVH  H1.xyz(NE.x), H1.w;\n"
"MOVR  R0.z, {5}.x;\n"
"SEQR  H2.x, channel, R0.z;\n"
"SEQX  H0.z, H0, {0}.x;\n"
"MULX  H0.w, H0.y, H0.z;\n"
"MOVH  H0.x, H1;\n"
"MULXC HC.x, H0.w, H2;\n"
"MULH  H0.x(NE), H1, {0.5};\n"
"MADH  H0.x(NE), H1.w, {0.5}, H0;\n"
"ADDH  H0.y, H0.x, H1;\n"
"ADDH  H1.x, H0.y, H1.z;\n"
"MOVH  H0.yz, H1;\n"
"SEQX  H1.y, H2.x, {0}.x;\n"
"MULX  H0.w, H0, H1.y;\n"
"MOVR  R0.z, {6}.x;\n"
"SEQR  H1.z, channel.x, R0;\n"
"MULXC HC.x, H0.w, H1.z;\n"
"SLTR  H2.x, R1, {0};\n"
"MULH  H0.xyz(NE.x), H1.x, {0.33333334}.x;\n"
"SEQR  H0.w, mask.x, R0.x;\n"
"MULR  R0.z, R1.x, {0.5}.x;\n"
"SEQR  H1.y, mask.x, R0;\n"
"SEQX  H1.x, H0.w, {0};\n"
"MULX  H1.z, H1.x, H1.y;\n"
"FRCR  R0.x, |R0.z|;\n"
"MULR  R0.y, R0.x, {2}.x;\n"
"SEQX  H2.x, H2, {0};\n"
"MULXC HC.x, H1.z, H2;\n"
"MOVR  R0.x, -R0.y;\n"
"MOVR  R0.x(NE), R0.y;\n"
"FLRR  R0.y, |R0.x|;\n"
"MOVRC RC.x, R0;\n"
"MOVR  R0.z, R0.y;\n"
"MULR  R0.x, R1.y, {0.5};\n"
"FRCR  R0.x, |R0|;\n"
"SLTR  H2.x, R1.y, {0};\n"
"SEQX  H1.y, H1, {0}.x;\n"
"MOVR  R0.z(LT.x), -R0.y;\n"
"MULR  R0.x, R0, {2};\n"
"SEQX  H2.x, H2, {0};\n"
"MOVR  R0.y, -R0.x;\n"
"MULXC HC.x, H0.w, H2;\n"
"MOVR  R0.y(NE.x), R0.x;\n"
"FLRR  R1.z, |R0.y|;\n"
"MOVRC RC.x, R0.y;\n"
"MOVR  R0.x, R1.z;\n"
"MOVR  R0.x(LT), -R1.z;\n"
"MOVXC RC.x, H0.w;\n"
"MOVR  R0.x(EQ), {1000};\n"
"MOVXC RC.x, H1.z;\n"
"MOVR  R0.x(NE), R0.z;\n"
"FLRR  R0.y, R1;\n"
"FLRR  R0.z, R1.x;\n"
"ADDR  R1.x, R0.z, R0.y;\n"
"SLTR  H0.w, R0.z, -R0.y;\n"
"MULR  R1.x, R1, {0.5};\n"
"FRCR  R1.x, |R1|;\n"
"MULR  R1.x, R1, {2};\n"
"MOVR  R0.y, -R1.x;\n"
"SEQX  H0.w, H0, {0}.x;\n"
"SEQR  H1.z, mask.x, R0.w;\n"
"MULX  H1.x, H1, H1.y;\n"
"MULX  H1.x, H1, H1.z;\n"
"MULXC HC.x, H1, H0.w;\n"
"MOVR  R0.y(NE.x), R1.x;\n"
"MOVXC RC.x, H1;\n"
"SLTR  R0.x(NE), R0.y, {1};\n"
"MOVH  H0.w, H1;\n"
"SEQRC HC.x, R0, mask_value;\n"
"MOVH  H0(NE.x), {0}.x;\n"
"MOVH  o[COLH].xyz, H0;\n"
"MOVXC RC.x, premult;\n"
"MULH  o[COLH].xyz(NE.x), H0, H0.w;\n"
"MOVH  o[COLH].w, H0;\n"
"END\n"
"# 184 instructions, 2 R-regs, 8 H-regs\n";

} // namespace


void GLEngine::handle_cg_errors()
{
//   std::cerr << cgGetErrorString (cgGetError()) << std::endl;
//   std::cerr << cgGetLastListing (cgContext) << std::endl;
//   exit(1);
}

char buf1[512];
#define GLSL(x) do { code << #x << std::endl; } while( 0 );
#define GLSLF(...) do { sprintf(buf1, __VA_ARGS__ ); code << buf1; } while(0);
#define GLSLH(x)   do { hdr << #x << "\n"; } while(0);
#define GLSLHF(...) do { sprintf(buf1, __VA_ARGS__ ); hdr << buf1; } while(0);

// A := A * B
static void mp_mul_matrix3x3(float a[3][3], float b[3][3])
{
    float a00 = a[0][0], a01 = a[0][1], a02 = a[0][2],
          a10 = a[1][0], a11 = a[1][1], a12 = a[1][2],
          a20 = a[2][0], a21 = a[2][1], a22 = a[2][2];

    for (int i = 0; i < 3; i++) {
        a[0][i] = a00 * b[0][i] + a01 * b[1][i] + a02 * b[2][i];
        a[1][i] = a10 * b[0][i] + a11 * b[1][i] + a12 * b[2][i];
        a[2][i] = a20 * b[0][i] + a21 * b[1][i] + a22 * b[2][i];
    }
}

void gl_sc_uniform_vec3( ostringstream& code, const char* name, float v[3] )
{
    code << "uniform vec3 " << name << " = vec3( " << v[0] << ", " << v[1]
         << ", " << v[2] << ");" << std::endl;
}

void gl_sc_uniform_mat3( ostringstream& code, const char* name,
                         bool transpose, float* m )
{
    if ( transpose )
    {
        code << "uniform mat3 " << name << " = mat3( " << m[0] << ", "
             << m[3] << ", " << m[6] << "," << std::endl
             << "\t\t" << m[1] << ", " << m[4] << ", " << m[7] << ", "
             << std::endl << "\t\t" << m[2] << ", " << m[5]
             << ", " << m[8] << " ); " << std::endl;
    }
    else
    {
        code << "uniform mat3 " << name << " = mat3( " << m[0] << ", "
             << m[1] << ", " << m[2] << "," << std::endl
             << "\t\t" << m[3] << ", " << m[4] << ", " << m[5] << ", "
             << std::endl << "\t\t" << m[6] << ", " << m[7]
             << ", " << m[8] << " ); " << std::endl;
    }
}

enum tone_mapping {
    TONE_MAPPING_CLIP,
    TONE_MAPPING_MOBIUS,
    TONE_MAPPING_REINHARD,
    TONE_MAPPING_HABLE,
    TONE_MAPPING_GAMMA,
    TONE_MAPPING_LINEAR,
};


#define MP_REF_WHITE 100.0


// Average light level for SDR signals. This is equal to a signal level of 0.5
// under a typical presentation gamma of about 2.0.
static const float sdr_avg = 0.25;  // <- was 0.25

// The threshold for which to consider an average luminance difference to be
// a sign of a scene change.
static const int scene_threshold = 0.2 * MP_REF_WHITE;


// How many frames to average over for HDR peak detection
#define PEAK_DETECT_FRAMES 100

// Common constants for SMPTE ST.2084 (HDR)
static const float PQ_M1 = 2610./4096 * 1./4,
                   PQ_M2 = 2523./4096 * 128,
                   PQ_C1 = 3424./4096,
                   PQ_C2 = 2413./4096 * 32,
                   PQ_C3 = 2392./4096 * 32;

// Common constants for ARIB STD-B67 (HLG)
static const float HLG_A = 0.17883277,
                   HLG_B = 0.28466892,
                   HLG_C = 0.55991073;

// Common constants for Panasonic V-Log
static const float VLOG_B = 0.00873,
                   VLOG_C = 0.241514,
                   VLOG_D = 0.598206;

// Common constants for Sony S-Log
static const float SLOG_A = 0.432699,
                   SLOG_B = 0.037584,
                   SLOG_C = 0.616596 + 0.03,
                   SLOG_P = 3.538813,
                   SLOG_Q = 0.030001,
                   SLOG_K2 = 155.0 / 219.0;


static void hdr_update_peak(ostringstream& code, ostringstream& hdr )
{
    // For performance, we want to do as few atomic operations on global
    // memory as possible, so use an atomic in shmem for the work group.
    GLSLH(shared uint wg_sum;);
    GLSL(wg_sum = 0;)

    // Have each thread update the work group sum with the local value
    GLSL(barrier();)
    GLSLF("atomicAdd(wg_sum, uint(sig * %f));\n", MP_REF_WHITE);

    // Have one thread per work group update the global atomics. We use the
    // work group average even for the global sum, to make the values slightly
    // more stable and smooth out tiny super-highlights.
    GLSL(memoryBarrierShared();)
    GLSL(barrier();)
    GLSL(if (gl_LocalInvocationIndex == 0) {)
    GLSL(    uint wg_avg = wg_sum / (gl_WorkGroupSize.x * gl_WorkGroupSize.y);)
    GLSL(    atomicMax(frame_max[frame_idx], wg_avg);)
    GLSL(    atomicAdd(frame_avg[frame_idx], wg_avg);)
    GLSL(})

    const float refi = 1.0 / MP_REF_WHITE;

    // Update the sig_peak/sig_avg from the old SSBO state
    GLSL(uint num_wg = gl_NumWorkGroups.x * gl_NumWorkGroups.y;)
    GLSL(if (frame_num > 0) {)
    GLSLF("    float peak = %f * float(total_max) / float(frame_num);\n", refi);
    GLSLF("    float avg = %f * float(total_avg) / float(frame_num);\n", refi);
    GLSLF("    sig_peak = max(1.0, peak);\n");
    GLSLF("    sig_avg  = max(%f, avg);\n", sdr_avg);
    GLSL(});

    // Finally, to update the global state, we increment a counter per dispatch
    GLSL(memoryBarrierBuffer();)
    GLSL(barrier();)
    GLSL(if (gl_LocalInvocationIndex == 0 && atomicAdd(counter, 1) == num_wg - 1) {)

    // Since we sum up all the workgroups, we also still need to divide the
    // average by the number of work groups
    GLSL(    counter = 0;)
    GLSL(    frame_avg[frame_idx] /= num_wg;)
    GLSL(    uint cur_max = frame_max[frame_idx];)
    GLSL(    uint cur_avg = frame_avg[frame_idx];)

    // Scene change detection
    GLSL(    int diff = int(frame_num * cur_avg) - int(total_avg);)
    GLSLF("  if (abs(diff) > frame_num * %d) {\n", scene_threshold);
    GLSL(        frame_num = 0;)
    GLSL(        total_max = total_avg = 0;)
    GLSLF("      for (uint i = 0; i < %d; i++)\n", PEAK_DETECT_FRAMES+1);
    GLSL(            frame_max[i] = frame_avg[i] = 0;)
    GLSL(        frame_max[frame_idx] = cur_max;)
    GLSL(        frame_avg[frame_idx] = cur_avg;)
    GLSL(    })

    // Add the current frame, then subtract and reset the next frame
    GLSLF("  uint next = (frame_idx + 1) %% %d;\n", PEAK_DETECT_FRAMES+1);
    GLSL(    total_max += cur_max - frame_max[next];)
    GLSL(    total_avg += cur_avg - frame_avg[next];)
    GLSL(    frame_max[next] = frame_avg[next] = 0;)

    // Update the index and count
    GLSL(    frame_idx = next;)
    GLSLF("  frame_num = min(frame_num + 1, %d);\n", PEAK_DETECT_FRAMES);
    GLSL(    memoryBarrierBuffer();)
    GLSL(})
}


// Inverse of the function pass_ootf, for completeness' sake.
void pass_inverse_ootf(ostringstream& code, enum mp_csp_light light, float peak)
{
    if (light == MP_CSP_LIGHT_DISPLAY)
        return;

    GLSLF("// apply inverse ootf\n");
    GLSLF("c.rgb *= vec3(%f);\n", peak);

    switch (light)
    {
    case MP_CSP_LIGHT_SCENE_HLG:
        GLSLF("c.rgb *= vec3(1.0/%f);\n", (1000 / MP_REF_WHITE) / pow(12, 1.2));
        GLSL(c.rgb /= vec3(max(1e-6, pow(dot(src_luma, c.rgb), 0.2/1.2)));)
        break;
    case MP_CSP_LIGHT_SCENE_709_1886:
        GLSL(c.rgb = pow(c.rgb, vec3(1.0/2.4));)
        GLSL(c.rgb = mix(c.rgb * vec3(1.0/4.5),
                             pow((c.rgb + vec3(0.0993)) * vec3(1.0/1.0993),
                                 vec3(1/0.45)),
                             lessThan(vec3(0.08145), c.rgb));)
        break;
    case MP_CSP_LIGHT_SCENE_1_2:
        GLSL(c.rgb = pow(c.rgb, vec3(1.0/1.2));)
        break;
    default:
        abort();
    }

    GLSLF("c.rgb *= vec3(1.0/%f);\n", peak);
}

static void pass_tone_map( ostringstream& code, ostringstream& hdr,
                           bool detect_peak,
                           float src_peak, float dst_peak,
                           enum tone_mapping algo, float param, float desat )
{
    GLSLF("// HDR tone mapping\n");


    // To prevent discoloration due to out-of-bounds clipping, we need to make
    // sure to reduce the value range as far as necessary to keep the entire
    // signal in range, so tone map based on the brightest component.
    GLSL(float sig = max(max(c.r, c.g), c.b);)
    GLSLF("float sig_peak = %f;\n", src_peak);
    GLSLF("float sig_avg = %f;\n", sdr_avg * 2.0f);

    if (detect_peak)
        hdr_update_peak(code, hdr);

    if (dst_peak > 1.0) {
        GLSLF("sig *= %f;\n", 1.0 / dst_peak);
        GLSLF("sig_peak *= %f;\n", 1.0 / dst_peak);
    }

    GLSL(float sig_orig = sig;)
    GLSLF("float slope = min(1.0, %f / sig_avg);\n", sdr_avg);
    GLSL(sig *= slope;)
    GLSL(sig_peak *= slope;)

    // Desaturate the color using a coefficient dependent on the signal.
    // Do this after peak detection in order to prevent over-desaturating
    // overly bright souces
    if (desat > 0) {
        float base = 0.18 * dst_peak;
        GLSL(float luma = dot(dst_luma, c.rgb);)
        GLSLF("float coeff = max(sig - %f, 1e-6) / max(sig, 1e-6);\n", base);
        GLSLF("coeff = pow(coeff, %f);\n", 10.0 / desat);
        GLSL(c.rgb = mix(c.rgb, vec3(luma), coeff);)
        GLSL(sig = mix(sig, luma * slope, coeff);) // also make sure to update `sig`
    }

    switch (algo) {
    case TONE_MAPPING_CLIP:
        GLSLF("sig = %f * sig;\n", isnan(param) ? 1.0 : param);
        break;

    case TONE_MAPPING_MOBIUS:
        GLSLF("if (sig_peak > (1.0 + 1e-6)) {\n");
        GLSLF("const float j = %f;\n", isnan(param) ? 0.3 : param);
        // solve for M(j) = j; M(sig_peak) = 1.0; M'(j) = 1.0
        // where M(x) = scale * (x+a)/(x+b)
        GLSLF("float a = -j*j * (sig_peak - 1.0) / (j*j - 2.0*j + sig_peak);\n");
        GLSLF("float b = (j*j - 2.0*j*sig_peak + sig_peak) / "
              "max(1e-6, sig_peak - 1.0);\n");
        GLSLF("float scale = (b*b + 2.0*b*j + j*j) / (b-a);\n");
        GLSL(sig = mix(sig, scale * (sig + a) / (sig + b), sig > j);)
        GLSLF("}\n");
        break;

    case TONE_MAPPING_REINHARD: {
        float contrast = isnan(param) ? 0.5 : param,
              offset = (1.0 - contrast) / contrast;
        GLSLF("sig = sig / (sig + %f);\n", offset);
        GLSLF("float scale = (sig_peak + %f) / sig_peak;\n", offset);
        GLSL(sig *= scale;)
        break;
    }

    case TONE_MAPPING_HABLE: {
        float A = 0.15, B = 0.50, C = 0.10, D = 0.20, E = 0.02, F = 0.30;
        GLSLHF("float hable(float x) {\n");
        GLSLHF("return ((x * (%f*x + %f)+%f)/(x * (%f*x + %f) + %f)) - %f;\n",
               A, C*B, D*E, A, B, D*F, E/F);
        GLSLHF("}\n");
        GLSL(sig = hable(sig) / hable(sig_peak);)
        break;
    }

    case TONE_MAPPING_GAMMA: {
        float gamma = isnan(param) ? 1.8 : param;
        GLSLF("const float cutoff = 0.05, gamma = %f;\n", 1.0/gamma);
        GLSL(float scale = pow(cutoff / sig_peak, gamma) / cutoff;)
        GLSL(sig = sig > cutoff ? pow(sig / sig_peak, gamma) : scale * sig;)
        break;
    }

    case TONE_MAPPING_LINEAR: {
        float coeff = isnan(param) ? 1.0 : param;
        GLSLF("sig = %f / sig_peak * sig;\n", coeff);
        break;
    }

    default:
        abort();
    }

    // Apply the computed scale factor to the color, linearly to prevent
    // discoloration
    GLSL(sig = min(sig, 1.0);)
    GLSL(c.rgb *= vec3(sig / sig_orig);)
}

// Linearize (expand), given a TRC as input. In essence, this is the ITU-R
// EOTF, calculated on an idealized (reference) monitor with a white point of
// MP_REF_WHITE and infinite contrast.
void pass_linearize(ostringstream& code, enum mp_csp_trc trc)
{
    if (trc == MP_CSP_TRC_LINEAR)
        return;

    GLSLF("// linearize\n");

    // Note that this clamp may technically violate the definition of
    // ITU-R BT.2100, which allows for sub-blacks and super-whites to be
    // displayed on the display where such would be possible. That said, the
    // problem is that not all gamma curves are well-defined on the values
    // outside this range, so we ignore it and just clip anyway for sanity.
    GLSL(c.rgb = clamp(c.rgb, 0.0, 1.0);)

    switch (trc) {
    case MP_CSP_TRC_SRGB:
        GLSL(c.rgb = mix(c.rgb * vec3(1.0/12.92),
                             pow((c.rgb + vec3(0.055))/vec3(1.055), vec3(2.4)),
                             lessThan(vec3(0.04045), c.rgb));)
        break;
    case MP_CSP_TRC_BT_1886:
        GLSL(c.rgb = pow(c.rgb, vec3(2.4));)
        break;
    case MP_CSP_TRC_GAMMA18:
        GLSL(c.rgb = pow(c.rgb, vec3(1.8));)
        break;
    case MP_CSP_TRC_GAMMA22:
        GLSL(c.rgb = pow(c.rgb, vec3(2.2));)
        break;
    case MP_CSP_TRC_GAMMA28:
        GLSL(c.rgb = pow(c.rgb, vec3(2.8));)
        break;
    case MP_CSP_TRC_PRO_PHOTO:
        GLSL(c.rgb = mix(c.rgb * vec3(1.0/16.0),
                             pow(c.rgb, vec3(1.8)),
                             lessThan(vec3(0.03125), c.rgb));)
        break;
    case MP_CSP_TRC_PQ:
        GLSLF("c.rgb = pow(c.rgb, vec3(1.0/%f));\n", PQ_M2);
        GLSLF("c.rgb = max(c.rgb - vec3(%f), vec3(0.0)) \n"
              "             / (vec3(%f) - vec3(%f) * c.rgb);\n",
              PQ_C1, PQ_C2, PQ_C3);
        GLSLF("c.rgb = pow(c.rgb, vec3(1.0/%f));\n", PQ_M1);
        // PQ's output range is 0-10000, but we need it to be relative to to
        // MP_REF_WHITE instead, so rescale
        GLSLF("c.rgb *= vec3(%f);\n", 10000 / MP_REF_WHITE);
        break;
    case MP_CSP_TRC_HLG:
        GLSLF("c.rgb = mix(vec3(4.0) * c.rgb * c.rgb,\n"
              "                exp((c.rgb - vec3(%f)) * vec3(1.0/%f)) + vec3(%f),\n"
              "                lessThan(vec3(0.5), c.rgb));\n",
              HLG_C, HLG_A, HLG_B);
        break;
    case MP_CSP_TRC_V_LOG:
        GLSLF("c.rgb = mix((c.rgb - vec3(0.125)) * vec3(1.0/5.6), \n"
              "    pow(vec3(10.0), (c.rgb - vec3(%f)) * vec3(1.0/%f)) \n"
              "              - vec3(%f),                                  \n"
              "    lessThanEqual(vec3(0.181), c.rgb));                \n",
              VLOG_D, VLOG_C, VLOG_B);
        break;
    case MP_CSP_TRC_S_LOG1:
        GLSLF("c.rgb = pow(vec3(10.0), (c.rgb - vec3(%f)) * vec3(1.0/%f))\n"
              "            - vec3(%f);\n",
              SLOG_C, SLOG_A, SLOG_B);
        break;
    case MP_CSP_TRC_S_LOG2:
        GLSLF("c.rgb = mix((c.rgb - vec3(%f)) * vec3(1.0/%f),      \n"
              "    (pow(vec3(10.0), (c.rgb - vec3(%f)) * vec3(1.0/%f)) \n"
              "              - vec3(%f)) * vec3(1.0/%f),                   \n"
              "    lessThanEqual(vec3(%f), c.rgb));                    \n",
              SLOG_Q, SLOG_P, SLOG_C, SLOG_A, SLOG_B, SLOG_K2, SLOG_Q);
        break;
    default:
        abort();
    }

    // Rescale to prevent clipping on non-float textures
    GLSLF("c.rgb *= vec3(1.0/%f);\n", mp_trc_nom_peak(trc));
}

// Delinearize (compress), given a TRC as output. This corresponds to the
// inverse EOTF (not the OETF) in ITU-R terminology, again assuming a
// reference monitor.
void pass_delinearize( ostringstream& code, enum mp_csp_trc trc)
{
    if (trc == MP_CSP_TRC_LINEAR)
        return;

    GLSLF("// delinearize\n");
    GLSL(c.rgb = clamp(c.rgb, 0.0, 1.0);)
    GLSLF("c.rgb *= vec3(%f);\n", mp_trc_nom_peak(trc));

    switch (trc) {
    case MP_CSP_TRC_SRGB:
        GLSL(c.rgb = mix(c.rgb * vec3(12.92),
                             vec3(1.055) * pow(c.rgb, vec3(1.0/2.4))
                                 - vec3(0.055),
                             lessThanEqual(vec3(0.0031308), c.rgb));)
        break;
    case MP_CSP_TRC_BT_1886:
        GLSL(c.rgb = pow(c.rgb, vec3(1.0/2.4));)
        break;
    case MP_CSP_TRC_GAMMA18:
        GLSL(c.rgb = pow(c.rgb, vec3(1.0/1.8));)
        break;
    case MP_CSP_TRC_GAMMA22:
        GLSL(c.rgb = pow(c.rgb, vec3(1.0/2.2));)
        break;
    case MP_CSP_TRC_GAMMA28:
        GLSL(c.rgb = pow(c.rgb, vec3(1.0/2.8));)
        break;
    case MP_CSP_TRC_PRO_PHOTO:
        GLSL(c.rgb = mix(c.rgb * vec3(16.0),
                             pow(c.rgb, vec3(1.0/1.8)),
                             lessThanEqual(vec3(0.001953), c.rgb));)
        break;
    case MP_CSP_TRC_PQ:
        GLSLF("c.rgb *= vec3(1.0/%f);\n", 10000 / MP_REF_WHITE);
        GLSLF("c.rgb = pow(c.rgb, vec3(%f));\n", PQ_M1);
        GLSLF("c.rgb = (vec3(%f) + vec3(%f) * c.rgb) \n"
              "             / (vec3(1.0) + vec3(%f) * c.rgb);\n",
              PQ_C1, PQ_C2, PQ_C3);
        GLSLF("c.rgb = pow(c.rgb, vec3(%f));\n", PQ_M2);
        break;
    case MP_CSP_TRC_HLG:
        GLSLF("c.rgb = mix(vec3(0.5) * sqrt(c.rgb),\n"
              "                vec3(%f) * log(c.rgb - vec3(%f)) + vec3(%f),\n"
              "                lessThan(vec3(1.0), c.rgb));\n",
              HLG_A, HLG_B, HLG_C);
        break;
    case MP_CSP_TRC_V_LOG:
        GLSLF("c.rgb = mix(vec3(5.6) * c.rgb + vec3(0.125),   \n"
              "                vec3(%f) * log(c.rgb + vec3(%f))   \n"
              "                    + vec3(%f),                        \n"
              "                lessThanEqual(vec3(0.01), c.rgb)); \n",
              VLOG_C / M_LN10, VLOG_B, VLOG_D);
        break;
    case MP_CSP_TRC_S_LOG1:
        GLSLF("c.rgb = vec3(%f) * log(c.rgb + vec3(%f)) + vec3(%f);\n",
              SLOG_A / M_LN10, SLOG_B, SLOG_C);
        break;
    case MP_CSP_TRC_S_LOG2:
        GLSLF("c.rgb = mix(vec3(%f) * c.rgb + vec3(%f),                \n"
              "                vec3(%f) * log(vec3(%f) * c.rgb + vec3(%f)) \n"
              "                    + vec3(%f),                                 \n"
              "                lessThanEqual(vec3(0.0), c.rgb));           \n",
              SLOG_P, SLOG_Q, SLOG_A / M_LN10, SLOG_K2, SLOG_B, SLOG_C);
        break;
    default:
        abort();
    }
}

// Apply the OOTF mapping from a given light type to display-referred light.
// The extra peak parameter is used to scale the values before and after
// the OOTF, and can be inferred using mp_trc_nom_peak
void pass_ootf( ostringstream& code, enum mp_csp_light light, float peak)
{
    if (light == MP_CSP_LIGHT_DISPLAY)
        return;

    GLSLF("// apply ootf\n");
    GLSLF("c.rgb *= vec3(%f);\n", peak);

    switch (light)
    {
    case MP_CSP_LIGHT_SCENE_HLG:
        // HLG OOTF from BT.2100, assuming a reference display with a
        // peak of 1000 cd/m² -> gamma = 1.2
        GLSLF("c.rgb *= vec3(%f * pow(dot(src_luma, c.rgb), 0.2));\n",
              (1000 / MP_REF_WHITE) / pow(12, 1.2));
        break;
    case MP_CSP_LIGHT_SCENE_709_1886:
        // This OOTF is defined by encoding the result as 709 and then decoding
        // it as 1886; although this is called 709_1886 we actually use the
        // more precise (by one decimal) values from BT.2020 instead
        GLSL(c.rgb = mix(c.rgb * vec3(4.5),
                             vec3(1.0993) * pow(c.rgb, vec3(0.45)) - vec3(0.0993),
                             lessThan(vec3(0.0181), c.rgb));)
        GLSL(c.rgb = pow(c.rgb, vec3(2.4));)
        break;
    case MP_CSP_LIGHT_SCENE_1_2:
        GLSL(c.rgb = pow(c.rgb, vec3(1.2));)
        break;
    default:
        abort();
    }

    GLSLF("c.rgb *= vec3(1.0/%f);\n", peak);
}

// Map colors from one source space to another. These source spaces must be
// known (i.e. not MP_CSP_*_AUTO), as this function won't perform any
// auto-guessing. If is_linear is true, we assume the input has already been
// linearized (e.g. for linear-scaling). If `detect_peak` is true, we will
// detect the peak instead of relying on metadata. Note that this requires
// the caller to have already bound the appropriate SSBO and set up the
// compute shader metadata
void pass_color_map(ostringstream& code,
                    ostringstream& hdr,
                    struct mp_colorspace src, struct mp_colorspace dst,
                    enum tone_mapping algo, float tone_mapping_param,
                    float tone_mapping_desat, bool detect_peak,
                    bool gamut_warning, bool is_linear)
{
    GLSLF("///////////////////\n");
    GLSLF("// color mapping //\n");
    GLSLF("///////////////////\n");


    // Some operations need access to the video's luma coefficients, so make
    // them available
    float rgb2xyz[3][3];
    mp_get_rgb2xyz_matrix(mp_get_csp_primaries(src.primaries), rgb2xyz);
    gl_sc_uniform_vec3(hdr, "src_luma", rgb2xyz[1]);
    mp_get_rgb2xyz_matrix(mp_get_csp_primaries(dst.primaries), rgb2xyz);
    gl_sc_uniform_vec3(hdr, "dst_luma", rgb2xyz[1]);

    bool need_ootf = src.light != dst.light;
    if (src.light == MP_CSP_LIGHT_SCENE_HLG && src.sig_peak != dst.sig_peak)
        need_ootf = true;

    // All operations from here on require linear light as a starting point,
    // so we linearize even if src.gamma == dst.gamma when one of the other
    // operations needs it
    bool need_linear = src.gamma != dst.gamma ||
                       src.primaries != dst.primaries ||
                       src.sig_peak > dst.sig_peak ||
                       need_ootf;

    if (need_linear && !is_linear) {
        pass_linearize(code, src.gamma);
        is_linear= true;
    }

    // Pre-scale the incoming values into an absolute scale
    GLSLF("c.rgb *= vec3(%f);\n", mp_trc_nom_peak(src.gamma));

    if (need_ootf)
        pass_ootf(code, src.light, src.sig_peak);

    // Adapt to the right colorspace if necessary
    if (src.primaries != dst.primaries) {
        struct mp_csp_primaries csp_src = mp_get_csp_primaries(src.primaries),
                                csp_dst = mp_get_csp_primaries(dst.primaries);
        float m[3][3] = {{0}};
        mp_get_cms_matrix(csp_src, csp_dst, MP_INTENT_RELATIVE_COLORIMETRIC, m);
        gl_sc_uniform_mat3(hdr, "cms_matrix", true, &m[0][0]);
        GLSL(c.rgb = cms_matrix * c.rgb;)
    }

    // Tone map to prevent clipping when the source signal peak exceeds the
    // encodable range or we've reduced the gamut
    if (src.sig_peak > dst.sig_peak) {
        pass_tone_map(code, hdr, detect_peak, src.sig_peak, dst.sig_peak, algo,
                      tone_mapping_param, tone_mapping_desat);
    }

    if (need_ootf)
        pass_inverse_ootf(code, dst.light, dst.sig_peak);

    // Post-scale the outgoing values from absolute scale to normalized.
    // For SDR, we normalize to the chosen signal peak. For HDR, we normalize
    // to the encoding range of the transfer function.
    float dst_range = dst.sig_peak;
    if (mp_trc_is_hdr(dst.gamma))
        dst_range = mp_trc_nom_peak(dst.gamma);

    GLSLF("c.rgb *= vec3(%f);\n", 1.0 / dst_range);

    // Warn for remaining out-of-gamut colors is enabled
    if (gamut_warning) {
        GLSL(if (any(greaterThan(c.rgb, vec3(1.01)))))
            GLSL(c.rgb = vec3(1.0) - c.rgb;) // invert
    }

    // if (is_linear)
    //     pass_delinearize(code, dst.gamma);
}

void add_normal_code( ostringstream& code )
{
    code << "}\n"
    "else {\n"
    "yuv.r = 1.1643 * ( pre.r - 0.0625 );\n"
    "yuv.g = pre.g - 0.5;\n"
    "yuv.b = pre.b - 0.5;\n"
    "\n"
    "c.r = yuv.r + 1.5958 * yuv.b;\n"
    "c.g = yuv.r - 0.39173 * yuv.g - 0.81290 * yuv.b;\n"
    "c.b = yuv.r + 2.017 * yuv.g;\n"
    "\n";
}

void GLEngine::loadOpenGLShader()
{
    if ( !_image )
    {
        LOG_ERROR( "No image to proceed" );
        return;
    }

    setlocale( LC_NUMERIC, "C" );
    std::locale::global( std::locale("C") );
    code.imbue(std::locale());
    hdr.imbue(std::locale());
    foot.imbue(std::locale());

    hdr.clear();
    hdr.str("");
    hdr << " \n"
    " /** \n"
    " * @file   YCbCr.glsl \n"
    " * @author gga \n"
    " * @date   Thu Jul  5 22:50:08 2007 \n"
    " * \n"
    " * @brief    simple YCbCr texture with 3D lut shader \n"
    " * \n"
    " */ \n"
    " \n"
    "#version 130\n\n"
    "// Images \n"
    "uniform sampler2D YImage; \n"
    "uniform sampler2D UImage; \n"
    "uniform sampler2D VImage; \n"
    "uniform sampler3D lut; \n"
    " \n"
    "// Standard controls \n"
    "uniform float gain; \n"
    "uniform float gamma; \n"
    "uniform int   channel; \n"
    "\n"
    "// Interlaced/Checkerboard controls (don't work) \n"
    "uniform int mask; \n"
    "uniform int mask_value; \n"
    "uniform int height; \n"
    "uniform int width; \n"
    " \n"
    "// Normalization variables \n"
    "uniform bool  premult; \n"
    "uniform bool  unpremult; \n"
    "uniform bool  enableNormalization; \n"
    "uniform float normMin; \n"
    "uniform float normSpan; \n"
    "\n"
    "// YCbCr variables \n"
    "uniform bool  coeffs;  // Use fed coefficients instead of builtin ones \n"
    "uniform vec3  Koff; \n"
    "uniform vec3  Kr; \n"
    "uniform vec3  Kg; \n"
    "uniform vec3  Kb; \n"
    " \n"
    "// Lut variables  \n"
    "uniform bool  enableLut; \n"
    "uniform bool  lutF; \n"
    "uniform float lutMin; \n"
    "uniform float lutMax; \n"
    "uniform float lutM; \n"
    "uniform float lutT; \n"
    "uniform float scale; \n"
    "uniform float offset; \n"
    "\n"
    "\n";

    code.clear();
    code.str("");
    code <<
    "void main() \n"
    "{ \n"
    "  // \n"
    "  // Sample luminance and chroma, convert to RGB. \n"
    "  // \n"
    "  vec3 yuv; \n"
    "  vec4 c; \n"
    "  vec3 pre; \n"
    "  vec2 tc = gl_TexCoord[0].st; \n"
    "  pre.r = texture2D(YImage, tc.st).r;  // Y \n"
    "  pre.g = texture2D(UImage, tc.st).r;  // U \n"
    "  pre.b = texture2D(VImage, tc.st).r;  // V \n"
    " \n"
    "  if ( coeffs ) \n"
    "  { \n"
    "        pre += Koff; \n"
    "	\n"
    "\tc.r = dot(Kr, pre); \n"
    "\tc.g = dot(Kg, pre); \n"
    "\tc.b = dot(Kb, pre); \n" << std::endl;

    foot.clear();
    foot.str("");
    foot << " }\n"
    "       //\n"
    "       // Apply channel selection\n"
    "       //\n"
    "  int x = 1000;\n"
    "  \n"
    "  if ( mask == 1 )  // even odd rows\n"
    "  {\n"
    "      float f = tc.y * height;\n"
    "      x = int( mod( f, 2 ) );\n"
    "  }\n"
    "  else if ( mask == 2 ) // even odd columns\n"
    "  {\n"
    "      float f2 = tc.x * width;\n"
    "      x = int( mod( f2, 2 ) );\n"
    "  }\n"
    "  else if ( mask == 3 ) // checkerboard\n"
    "  {\n"
    "      float f = tc.y * height;\n"
    "      float f2 = tc.x * width;\n"
    "      x = int( mod( floor( f2 ) + floor( f ), 2 ) < 1 );\n"
    "  }\n"
    "\n"
    "  if ( x == mask_value )\n"
    "  {\n"
    "      c.r = c.g = c.b = c.a = 0.0;\n"
    "  }\n"
    "\n"
    "  //\n"
    "  // Apply normalization\n"
    "  //\n"
    "  if (enableNormalization)\n"
    "    {\n"
    "      c.rgb = (c.rgb - normMin) / normSpan;\n"
    "    }\n"
    "\n"
    "  //\n"
    "  // Apply gain \n"
    "  //\n"
    "  c.rgb *= gain;\n"
    "\n"
    "  //\n"
    "  // Apply 3D color lookup table (in log space).\n"
    "  //\n"
    "  if (enableLut)\n"
    "    {\n"
    "      c.rgb = lutT + lutM * log( clamp(c.rgb, lutMin, lutMax) );\n"
    "      c.rgb = exp( texture3D(lut, scale * c.rgb + offset ).rgb ); \n"
    "    }\n"
    "\n"
    "  if ( unpremult && c.a > 0.00001 )\n"
    "  {\n"
    "    c.rgb /= c.a;\n"
    "  }\n"
    "  \n"
    "  //\n"
    "  // Apply video gamma correction.\n"
    "  // \n"
    "  c.r = pow( c.r, gamma );\n"
    "  c.g = pow( c.g, gamma );\n"
    "  c.b = pow( c.b, gamma );\n"
    " \n"
    "  if ( channel == 1 )\n"
    "    {\n"
    "      c.rgb = c.rrr;\n"
    "    }\n"
    "  else if ( channel == 2 )\n"
    "    {\n"
    "      c.rgb = c.ggg;\n"
    "    }\n"
    "  else if ( channel == 3 )\n"
    "    {\n"
    "      c.rgb = c.bbb;\n"
    "    }\n"
    "  else if ( channel == 4 )\n"
    "    {\n"
    "      c.rgb = c.aaa;\n"
    "    }\n"
    "  else if ( channel == 5 )\n"
    "    {\n"
    "      c.r *= 0.5;\n"
    "      c.r += c.a * 0.5;\n"
    "    }\n"
    "  else if ( channel == 6 )\n"
    "    {\n"
    "      c.rgb = vec3( (c.r + c.g + c.b) / 3.0 );\n"
    "    }\n"
    "\n"
    "  if ( premult )\n"
    "  {\n"
    "      c.rgb *= c.a;\n"
    "  }\n"
    "\n"
    "  gl_FragColor = c;\n"
    "} ";

    _hardwareShaders = kGLSL;

    AVStream* st = _image->get_video_stream();
    if (!st)
    {
        add_normal_code( code );

        std::string all = hdr.str() + code.str() + foot.str();

        _YCbCr = new GLShader();
        _YCbCr->load( N_("builtin"), all.c_str() );
        return;
    }
    AVCodecParameters* c = st->codecpar;

    int size;
    AVMasteringDisplayMetadata* m = (AVMasteringDisplayMetadata*)
    av_stream_get_side_data( st,
                             AV_PKT_DATA_MASTERING_DISPLAY_METADATA,
                             &size );

    double max_cll = 100000;
    if (m)
    {
        if ( size == sizeof( AVMasteringDisplayMetadata ) )
        {
            if ( m->has_primaries )
            {
                LOG_INFO( "Primaries:" );
                LOG_INFO( "red " <<
                          av_q2d( m->display_primaries[0][0] )
                          << ", " <<
                          av_q2d( m->display_primaries[0][1] ) );
                LOG_INFO( "green "
                          << av_q2d( m->display_primaries[1][0] )
                          << ", "
                          << av_q2d( m->display_primaries[1][1] ) );
                LOG_INFO( "blue "
                          << av_q2d( m->display_primaries[2][0] )
                          << ", "
                          << av_q2d( m->display_primaries[2][1] ) );
                LOG_INFO( "white " << av_q2d( m->white_point[0] )
                          << ", " << av_q2d( m->white_point[1] )
                          );
            }

            if ( m->has_luminance )
            {
                max_cll = av_q2d( m->max_luminance );
                LOG_INFO( "Luminance: " );
                LOG_INFO( "min: " << av_q2d( m->min_luminance ) );
                LOG_INFO( "max: " << av_q2d( m->max_luminance ) );
                LOG_INFO( "max/MP_REF_WHITE: " << av_q2d( m->max_luminance ) /
                          MP_REF_WHITE );
            }
        }
    }

    mp_colorspace src
    {
        avcol_spc_to_mp_csp( c->color_space ), // space
        avcol_range_to_mp_csp_levels( c->color_range ),  // levels
        avcol_pri_to_mp_csp_prim( c->color_primaries ), // primaries
        avcol_trc_to_mp_csp_trc( c->color_trc ),   // gamma
        MP_CSP_LIGHT_DISPLAY,  // light
        max_cll / MP_REF_WHITE               // sig_peak
        };

    mp_colorspace dst
    {
        MP_CSP_AUTO,
        MP_CSP_LEVELS_AUTO,
        MP_CSP_PRIM_BT_709,
        MP_CSP_TRC_GAMMA22,
        MP_CSP_LIGHT_DISPLAY,
        1.0f
        };

    tone_mapping algo = TONE_MAPPING_HABLE;
    float tone_mapping_param = std::numeric_limits<float>::quiet_NaN();
    float tone_mapping_desat = 0.25f;
    bool  detect_peak = false;
    bool gamut_warning = false;
    bool is_linear = false;

    if ( mp_trc_is_hdr( src.gamma ) )
    {
        pass_color_map(code, hdr, src, dst,
                       algo, tone_mapping_param,
                       tone_mapping_desat, detect_peak,
                       gamut_warning, is_linear);
    }
    else
    {
        add_normal_code( code );
    }

    std::string all = hdr.str() + code.str() + foot.str();

    // std::cerr << all << std::endl;

    _YCbCr = new GLShader();
    _YCbCr->load( N_("builtin"), all.c_str() );
}

void
GLEngine::loadBuiltinFragShader()
{

    DBG( __FUNCTION__ << " " << __LINE__ );
    _rgba = new GLShader();

    try {
        if ( _hardwareShaders == kNV30 )
        {
            LOG_INFO( _("Loading built-in NV3.0 rgba shader") );
            _rgba->load( N_("builtin"), NVShader );
            DBG( "NVShader builtin" );
        }
        else
        {
            LOG_INFO( _("Loading built-in arbfp1 rgba shader") );
            DBG( "kARBFP1 builtin shader" );
            _hardwareShaders = kARBFP1;
            _rgba->load( N_("builtin"), ARBFP1Shader );
        }
    }
    catch( const Iex::BaseExc& e )
    {
      LOG_ERROR( e.what() );
    }
    catch( const std::exception& e )
    {
      LOG_ERROR( e.what() );
    }
    catch( ... )
    {
        LOG_ERROR( "Unknown error in loadBuiltinFragShader" );
    }

    setlocale( LC_NUMERIC, "" );
    std::locale::global( std::locale("") );
}


void GLEngine::clear_quads()
{
    TRACE("");
    DBG( __FUNCTION__ << " " << __LINE__ );
    QuadList::iterator i = _quads.begin();
    QuadList::iterator e = _quads.end();
    for ( ; i != e; ++i )
    {
        delete *i;
    }
    _quads.clear();

}


void GLEngine::release()
{
    TRACE("");
    DBG( __FUNCTION__ << " " << __LINE__ );
    clear_quads();

    TRACE("");
    DBG( __FUNCTION__ << " " << __LINE__ );
    GLLut3d::clear();

    TRACE("");
    DBG( __FUNCTION__ << " " << __LINE__ );
    if ( sCharset ) {
        glDeleteLists( sCharset, 255 );
        CHECK_GL;
    }

    DBG( __FUNCTION__ << " " << __LINE__ );
    TRACE("");
    if (_rgba)  delete _rgba;
    DBG( __FUNCTION__ << " " << __LINE__ );
    TRACE("");
    if (_YByRy) delete _YByRy;
    DBG( __FUNCTION__ << " " << __LINE__ );
    TRACE("");
    if (_YCbCr) delete _YCbCr;

    if (_YByRyA) delete _YByRyA;
    DBG( __FUNCTION__ << " " << __LINE__ );
    TRACE("");
    if (_YCbCrA) delete _YCbCrA;
}


void GLEngine::resize_background()
{
}

GLEngine::GLEngine(const mrv::ImageView* v) :
DrawEngine( v ),
texWidth( 0 ),
texHeight( 0 ),
vr( ImageView::kNoVR ),
vr_angle( 45.0 ),
_rotX( 0.0 ),
_rotY( 0.0 )
{
    initialize();
}

GLEngine::~GLEngine()
{
  release();
}





} // namespace mrv
