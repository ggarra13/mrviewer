/**
 * @file   slateImage.h
 * @author gga
 * @date   Sun Oct 22 22:52:27 2006
 * 
 * @brief  A simple image slate generator
 * 
 * 
 */


#ifndef slateImage_h
#define slateImage_h

#include "CMedia.h"

typedef struct _DrawingWand DrawingWand;
typedef struct _PixelWand   PixelWand;
typedef struct _MagickWand  MagickWand;

namespace mrv {

  class slateImage : public CMedia
  {
  public:
    slateImage( const CMedia* src );

    static bool test(const char* file) { return false; }

    virtual const char* const format() const { return "Slate"; }

    virtual bool has_changed() { return false; }
    bool fetch( const boost::int64_t frame );

  protected:
    void draw_text( double x, double y, const char* text );
    void draw_gradient();
    void draw_bars();

    int _w, _h;
    int64_t _fstart;
    int64_t _fend;
    DrawingWand* dwand;
    PixelWand*   pwand;  
    MagickWand*   wand;
  };

} // namespace mrv


#endif // slateImage_h
