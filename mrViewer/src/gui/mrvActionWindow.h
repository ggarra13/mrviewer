

#ifndef mrvActionWindow_h
#define mrvActionWindow_h

#include <FL/Fl_Window.H>

class ViewerUI;

namespace mrv {

class ImageView;

class ActionWindow : public Fl_Window
{
public:
    ActionWindow( int w, int h, const char* const  lbl = 0 ) :
    Fl_Window( w, h, lbl )
    {
    }

    void main( ViewerUI* m ) {
        uiMain = m;
    }
    ImageView* view() const;

    virtual int handle( int event );

protected:
    ViewerUI* uiMain;
};


}


#endif // mrvActionWindow_h
