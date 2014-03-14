/**
 * @file   mrvVectorscope.h
 * @author gga
 * @date   Thu Nov 09 14:43:34 2006
 * 
 * @brief  Displays a waveform for an image.
 * 
 */

#ifndef mrvVectorscope_h
#define mrvVectorscope_h

#include <fltk/Widget.h>

#include "CMedia.h"

namespace mrv
{
  class ViewerUI;

  class Vectorscope : public fltk::Widget
  {
  public:
    Vectorscope( int x, int y, int w, int h, const char* l = 0 );

    virtual void draw();

    void main( mrv::ViewerUI* m ) { uiMain = m; }

  protected:
    void draw_grid( const fltk::Rectangle& r );
    void draw_pixels( const fltk::Rectangle& r );
    void draw_pixel( const fltk::Rectangle& r,
		     const CMedia::Pixel& rgb,
		     const CMedia::Pixel& hsv );

    int diameter_;

    mrv::ViewerUI* uiMain;
  };

}  // namespace mrv


#endif // mrvVectorscope_h
