/**
 * @file   mrvColorInfo.h
 * @author gga
 * @date   Wed Nov 08 05:30:32 2006
 * 
 * @brief  Color Info Text Display
 * 
 * 
 */

#ifndef mrvColorInfo_h
#define mrvColorInfo_h

// #include <mrvBrowser.h>
#include <core/mrvRectangle.h>
#include <gui/mrvBrowser.h>

namespace fltk 
{
  class Widget;
  class Browser;
  class PopupMenu;
}


namespace mrv
{
  class CMedia;

  class ColorInfo : public fltk::Group
  {
  public:
    ColorInfo( int x, int y, int w, int h, const char* l = 0 );

    void main( mrv::ViewerUI* m ) { uiMain = m; }

    void update();
    void update( const CMedia* img,
		 const mrv::Rectd& selection );
  protected:
    fltk::Widget*    area;
    fltk::Browser*   browser;
    fltk::PopupMenu* uiColorB;
    mrv::ViewerUI*   uiMain;
  };

} // namespace mrv

#endif // mrvColorInfo_h

