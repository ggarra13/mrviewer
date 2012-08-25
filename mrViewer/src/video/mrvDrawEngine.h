/**
 * @file   mrvDrawEngine.h
 * @author gga
 * @date   Thu Jul  5 18:51:53 2007
 * 
 * @brief  Abstract class used by ImageView to draw elements
 * 
 * 
 */
#ifndef mrvDrawEngine_h
#define mrvDrawEngine_h

#include <vector>
#include <string>
#include <limits>

#include <boost/shared_array.hpp>

#include "core/CMedia.h"
#include "core/mrvRectangle.h"

#include "video/mrvGLShape.h"

namespace boost {
  class thread;
}

namespace mrv {
  class uvCoords;
  class ImageView;
}

namespace mrv {


  class DrawEngine
  {
  protected:
    struct uvCoords {
      float u;
      float v;
    };

  public:
    struct DisplayData
    {
      DisplayData() {};

      const ImageView*    view;
      const CMedia*   image;
      mrv::Recti          rect;
      mrv::image_type_ptr orig;
      mrv::image_type_ptr result;
      bool                alpha;
    };

    struct MinMaxData
    {
      MinMaxData() :
	pMin( std::numeric_limits< float >::max() ),
	pMax( std::numeric_limits< float >::min() )
      {};

      const CMedia*     image;
      mrv::Recti      rect;
      float pMin;
      float pMax;
    };

  public:
    enum ShaderType {
      kNone,
      kAuto,
      kGLSL,
      kNV30,
      kARBFP1,
    };

    static const char* shader_type_name();

    static ShaderType  shader_type()        { return _hardwareShaders; }
    static void shader_type( ShaderType b ) { _hardwareShaders = b; }


  public:
    DrawEngine(const ImageView* v );

    virtual ~DrawEngine();

    static bool supports_yuv()  { return _has_yuv; }
    static bool supports_yuva() { return _has_yuva; }

    /// Name of engine
    virtual const char* name() = 0;
    virtual std::string options() = 0;

    /// Refresh the luts
    virtual void refresh_luts() = 0;

    /// Clear canvas to a certain color
    virtual void clear_canvas( float r, float g, float b, float a = 0.0f ) = 0;

    /// Reset the view matrix
    virtual void reset_view_matrix() = 0;
    
    /// Change the color of an operation
    virtual void color( uchar r, uchar g, uchar b, uchar a = 255  ) = 0;
    virtual void color( float r, float g, float b, float a = 1.0f ) = 0;

    /// Draw a rectangle
    virtual void draw_rectangle( const mrv::Rectd& r ) = 0;

    /// Draw some arbitrary sized text centered on screen
    virtual void draw_title(const float size,
			    const int y, const char* text ) = 0;

    /// Draw some overlayed text
    virtual void draw_text(const int x, const int y, const char* text ) = 0;

    void draw_text( const int x, const int y, const std::string& text )
    {
      draw_text( x, y, text.c_str() );
    }

    /// Convert fg image to engine's drawable image
    virtual void draw_images( ImageList& images ) = 0;

    /// Wipe area (scissor test)
    virtual void wipe_area() = 0;

    
    /// Draw a safe area rectangle
    virtual void draw_safe_area( const double percentX, const double percentY,
				 const char* name = 0 ) = 0;

    /// Draw mask area rectangles
    virtual void draw_mask(const float pct) = 0;

       virtual void draw_cursor( const double x, const double y ) = 0;
       
    virtual void draw_annotation( const GLShapeList& shapes ) = 0;

    CMedia* const background();

    /// Convert a float image to uchar for display, taking into
    /// account gamma, gain, pixel ratio, lut, etc.
    /// This function may spawn multiple threads to do the conversions
    image_type_ptr display( const image_type_ptr& src, CMedia* img );

    void display( image_type_ptr& result, 
		  const image_type_ptr& src, CMedia* img );

     public:
       /// Find min/max values for an image, using multithreading if possible
       void minmax();

       // Retrieve min and max float values of image.  To be used after
       // minmax() is called once.
       inline void minmax( float& pMin, float& pMax ) {
	  pMin = _normMin;
	  pMax = _normMax;
       }

  protected:


    /// Find min/max values for an image, using multithreading if possible
    void minmax( float& pMin, float& pMax, const CMedia* img );



  protected:
    const ImageView* _view;

    //! Min-max pixel values (for normalization)
    float _normMin, _normMax;

    //! Copy of background image resized to fit foreground 
    CMedia* _background_resized;

    //! Buckets of threads for color correction
    typedef std::map< boost::thread*, void* > BucketList;
    BucketList buckets;

    static bool _has_yuv, _has_yuva;
    static ShaderType _hardwareShaders; //!< hardware shaders supported
  };

} // namespace mrv

#endif // mrvDrawEngine_h
