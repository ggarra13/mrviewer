/**
 * @file   mrvGL.h
 * @author gga
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
#include "mrvDrawEngine.h"


namespace mrv {


  class GLShader;
  class GLQuad;


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
    virtual void reset_view_matrix();

    virtual void color( uchar r, uchar g, uchar b, uchar a  );
    virtual void color( float r, float g, float b, float a );

    /// Draw a rectangle
    virtual void draw_rectangle( const mrv::Rectd& r );

    virtual void wipe_area();

    virtual void draw_mask(const float pct);
    
    virtual void draw_safe_area( const float percentX, 
				 const float percentY,
				 const char* name = 0 );

    /// Convert fg image to engine's drawable image
    virtual void draw_images( ImageList& images );

    virtual void draw_title(const float size,
			    const int y, const char* text );
    virtual void draw_text(const int x, const int y, const char* text );

    virtual void resize_background();

  public:
    static unsigned int maxTexWidth()  { return _maxTexWidth;  }
    static unsigned int maxTexHeight() { return _maxTexHeight; }
    static bool pboTextures()   { return _pboTextures;   }
    static bool pow2Textures()  { return _pow2Textures;  }
    static bool halfTextures()  { return _halfTextures;  }
    static bool floatTextures() { return _floatTextures; }
    static bool sdiOutput()     { return _sdiOutput;     }
    static GLint maxTexUnits()  { return _maxTexUnits;   }

    static GLShader* rgbaShader()    { return _rgba; }
    static GLShader* YCbCrShader()   { return _YCbCr; }
    static GLShader* YByRyShader()   { return _YByRy; }
    static GLShader* YCbCrAShader()  { return _YCbCrA; }
    static GLShader* YByRyAShader()  { return _YByRyA; }


    /// Auxiliary function used to check for openGL errors
    static void handle_gl_errors(const char* function);

  protected:

    /// Auxiliary function used to check for Cg errors;
    static void handle_cg_errors();

    /// Allocate a number of quads
    void alloc_quads( unsigned int num );

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

    GLuint lutId;              //!< The lut texture index
    float lutMin, lutMax, lutM, lutT, lutF; //!< The lut calculated parameters


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

    static GLuint     sCharset;   //!< display list for characters

  };


} // namespace mrv


