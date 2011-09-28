/**
 * @file   colorBarsImage.h
 * @author gga
 * @date   Sun Oct 22 22:52:27 2006
 * 
 * @brief  A simple 3-second color bar generator (with tone)
 * 
 * 
 */


#ifndef mrvColorBarsImage_h
#define mrvColorBarsImage_h

#include "CMedia.h"


namespace mrv {

  class ColorBarsImage : public CMedia
  {
  public:
    enum Type
      {
	kSMPTE_NTSC,
	kSMPTE_NTSC_HDTV,
	kPAL,
	kPAL_HDTV,
      };

  public:
    ColorBarsImage( const Type c = kSMPTE_NTSC_HDTV );

    static bool test(const char* file) { return false; }

    virtual const char* const format() const { return "Built-in Image"; }

    virtual bool fetch( const boost::int64_t frame );

  protected:
    void NTSC_color_bars();
    void PAL_color_bars();
    void NTSC_HDTV_color_bars();
    void PAL_HDTV_color_bars();
    
    void smpte_color_bars( const unsigned int X, const unsigned int W, 
			   const unsigned int H, const float pct );
    void smpte_bottom_bars( const unsigned int X, const unsigned int Y, 
			    const unsigned int W, const unsigned int H );
  };

} // namespace mrv


#endif // mrvColorBarsImage_h
