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

        mrv::image_type_ptr pic;
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

      virtual double rot_x() const = 0;
      virtual double rot_y() const = 0;
      
      virtual void rot_x(double t) = 0;
      virtual void rot_y(double t) = 0;
      
      // Evaluate pixel with LUT 
      virtual void evaluate( const CMedia* img,
                             const Imath::V3f& rgb, Imath::V3f& out ) = 0;

    /// Refresh the luts
    virtual void refresh_luts() = 0;

    /// Clear canvas to a certain color
    virtual void clear_canvas( float r, float g, float b, float a = 0.0f ) = 0;

    virtual void set_blend_function( int source, int dest ) = 0;

    /// Reset the view matrix
    virtual void reset_view_matrix() = 0;

      // rotate model an angle in Z in degrees
      virtual void rotate( const double z ) = 0;

      
    /// Change the color of an operation
    virtual void color( uchar r, uchar g, uchar b, uchar a = 255  ) = 0;
    virtual void color( float r, float g, float b, float a = 1.0f ) = 0;

    // Init FB0 Drawing   
    virtual bool init_fbo( ImageList& images ) = 0;
    virtual void end_fbo( ImageList& images ) = 0;

    /// Draw a rectangle
      virtual void draw_rectangle( const mrv::Rectd& r,
                                   const mrv::ImageView::FlipDirection f =
                                   mrv::ImageView::kFlipNone,
                                   const double zdeg = 0.0 ) = 0;

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
       virtual void translate( double x, double y ) = 0;

      // Return the normalized min and max of image
      inline float norm_min() { return _normMin; }
      inline float norm_max() { return _normMax; }
      
    /// Convert fg image to engine's drawable image
    virtual void draw_images( ImageList& images ) = 0;

      virtual void angle( const float x ) = 0;
      virtual float angle() const = 0;

    /// Wipe area (scissor test)
    virtual void wipe_area() = 0;

      /// Draw a safe area rectangle
      virtual void draw_safe_area( const double percentX, const double percentY,
				 const char* name = 0 ) = 0;

      /// Draw display area rectangle
      virtual void draw_square_stencil(const int x, const int y, 
                                       const int x2, const int y2 ) = 0;

      // Draw film mask
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
