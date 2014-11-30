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
 * @file   mrvGL.h
 * @author ggas
 * @date   Sun Jul  1 11:49:49 2007
 * 
 * @brief  Auxiliary class for opengl operations
 * 
 * 
 */

#include <vector>

#include <fltk/gl.h>
#include <boost/shared_array.hpp>

#include "core/mrvAlignedData.h"
#include "gui/mrvImageView.h"
#include "mrvDrawEngine.h"


namespace mrv {


  class GLShader;
  class GLQuad;
  class GLShape;


  class GLEngine : public mrv::DrawEngine {
  protected:
    typedef std::vector< GLQuad*  > QuadList;

  public:
    GLEngine( const mrv::ImageView* );
    ~GLEngine();

    /// Name of engine
    virtual const char* name() { return "OpenGL"; }
    virtual std::string options();

    virtual void refresh_luts();
    virtual void clear_canvas( float r, float g, float b, float a );
    virtual void set_blend_function( int source, int dest );
    virtual void reset_view_matrix();

    virtual void color( uchar r, uchar g, uchar b, uchar a  );
    virtual void color( float r, float g, float b, float a );

    // Init FB0 Drawing   
       virtual bool init_fbo( ImageList& images );
       virtual void end_fbo( ImageList& images );

    /// Draw a rectangle
    virtual void draw_rectangle( const mrv::Rectd& r );

    virtual void wipe_area();

      virtual void draw_square_stencil( const int x, const int y, 
					const int x2, const int y2 );
    virtual void draw_mask(const float pct);
    
    virtual void draw_safe_area( const double percentX, 
				 const double percentY,
				 const char* name = 0 );

    virtual void translate( double x, double y );

    /// Convert fg image to engine's drawable image
    virtual void draw_images( ImageList& images );


    virtual void draw_title(const float size,
			    const int y, const char* text );
    virtual void draw_text(const int x, const int y, const char* text );

    virtual void draw_cursor( const double x, const double y );

       virtual void draw_annotation( const GLShapeList& shapes );

    virtual void resize_background();

  public:
    static unsigned int maxTexWidth()  { return _maxTexWidth;  }
    static unsigned int maxTexHeight() { return _maxTexHeight; }
    static bool pboTextures()   { return _pboTextures;   }
    static bool pow2Textures()  { return _pow2Textures;  }
    static bool halfTextures()  { return _halfTextures;  }
    static bool floatTextures() { return _floatTextures; }
    static bool sdiOutput()     { return _sdiOutput;     }
    static bool fboRenderBuffer()  { return _fboRenderBuffer; }
    static GLint maxTexUnits()  { return _maxTexUnits;   }

    static GLShader* rgbaShader()    { return _rgba; }
    static GLShader* YCbCrShader()   { return _YCbCr; }
    static GLShader* YByRyShader()   { return _YByRy; }
    static GLShader* YCbCrAShader()  { return _YCbCrA; }
    static GLShader* YByRyAShader()  { return _YByRyA; }


    /// Auxiliary function used to check for openGL errors
    static void handle_gl_errors(const char* function);

  protected:
      void set_matrix( const ImageView::FlipDirection flip = 
                       ImageView::kFlipNone,
                       const bool pixel_ratio = false );

      void draw_shape( GLShape* const shape );

    /// Auxiliary function used to check for Cg errors;
    static void handle_cg_errors();

    void draw_safe_area_inner( const double tw, 
                               const double th,
                               const char* name = 0 );

    /// Allocate a number of quads
    void alloc_quads( size_t num );

    /// Function used to initialize all of opengl
    void initialize();

    /// Function used to release all opengl descriptors
    void release();

    /// Auxiliary function used to initialize texture info
    void init_textures();

    /// Auxiliary function used to initialize character set
    static void init_charset();

    /// Auxiliary function used to initialize GLEW library
    static void init_GLEW();

    //////////////////////
    //
    // Cg Stuff
    //
    //////////////////////

    /// Auxiliary function to load built-in rgba shader
    void loadBuiltinFragShader();


    static GLShader* _rgba;     //!< RGBA   Fragment shader
    static GLShader* _YCbCr;    //!< YCbCr  Fragment shader ( Y Cb  Cr )
    static GLShader* _YByRy;    //!< YBYRY  Fragment shader ( Y B-Y R-Y )
    static GLShader* _YCbCrA;   //!< YCbCrA Fragment shader ( Y Cb  Cr  A )
    static GLShader* _YByRyA;   //!< YBYRYA Fragment shader ( Y B-Y R-Y A )

    int	texWidth, texHeight;   //!< The texture dimensions (powers of two)

       GLuint textureId;       //!< The off-screen texture
    GLuint lutId;              //!< The lut texture index
    float lutMin, lutMax, lutM, lutT, lutF; //!< The lut calculated parameters

       GLuint id, rid;

    QuadList  _quads;


    //
    // GFX card limits
    //
    static unsigned int  _maxTexWidth, _maxTexHeight; //!< max texture sizes

    // OpenGL needs to be inited
    static bool _initGL;          //!< if not set, opengl must be inited for view
    static GLint _maxTexUnits;   //!< hardware texture units
    static bool _floatTextures;   //!< float textures supported
    static bool _halfTextures;    //!< half textures supported
    static bool _pow2Textures;    //!< only power of 2 textures supported
    static bool _pboTextures;     //!< Pixel Buffer Objects?
    static bool _sdiOutput;       //!< SDI output
    static bool _fboRenderBuffer; //!< Framebuffer object

    static GLuint     sCharset;   //!< display list for characters

  };


} // namespace mrv


