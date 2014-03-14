/**
 * @file   smpteImage.h
 * @author gga
 * @date   Sun Oct 22 22:52:27 2006
 * 
 * @brief  A simple image generator of callibration images
 * 
 * 
 */


#ifndef smpteImage_h
#define smpteImage_h

#include "CMedia.h"


namespace mrv {

  class smpteImage : public CMedia
  {
  public:
    enum Type
      {
	kGammaChart,
	kLinearGradient,
	kLuminanceGradient,
	kCheckered,
      };

  public:
    smpteImage( const Type c = kGammaChart, const unsigned int dw = 720,
		const unsigned int dh = 480 );

    static bool test(const char* file) { return false; }

    virtual const char* const format() const { return "Built-in Image"; }

    bool fetch( const boost::int64_t frame );

  protected:
    void luminance_gradient();
    void linear_gradient();
    void checkered();

    void gamma_chart();
    void gamma_boxes( unsigned int x, unsigned int y, 
		      unsigned int w, unsigned int h,
		      float bg, float fg );
    void gamma_box( unsigned int x, unsigned int y, 
		    unsigned int w, unsigned int h );

    Pixel bg;
    Pixel fg;
    Type type_;
  };

} // namespace mrv


#endif // smpteImage_h
