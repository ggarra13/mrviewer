

#ifndef mrvActionWindow_h
#define mrvActionWindow_h

#include <fltk/Window.h>

namespace mrv {

class ViewerUI;
class ImageView;

class ActionWindow : public fltk::Window
{
  public:
    ActionWindow( int w, int h, const char* const  lbl = 0 ) :
    fltk::Window( w, h, lbl )
    {
    }

    void main( mrv::ViewerUI* m ) { uiMain = m; }
    ImageView* view() const;
    
    virtual int handle( int event );

  protected:
    mrv::ViewerUI* uiMain;
};


}


#endif // mrvActionWindow_h
