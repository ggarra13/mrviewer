

#ifndef mrvStereoWindow_h
#define mrvStereoWindow_h

#include <FL/Fl_Window.H>

class ViewerUI;

namespace mrv {

class ImageView;

class StereoWindow : public Fl_Window
{
public:
    StereoWindow( int w, int h, const char* const  lbl = 0 ) :
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


#endif // mrvStereoWindow_h
