

#include <fltk/events.h>

#include "gui/mrvStereoWindow.h"
#include "gui/mrvImageView.h"
#include "mrViewer.h"

namespace mrv {

ImageView* StereoWindow::view() const {
    return uiMain->uiView;
}

int StereoWindow::handle( int event )
{
    int ok = 0;
    if ( event == fltk::KEY )
        ok = view()->handle( event );
    if ( !ok ) ok = fltk::Window::handle( event );
    return ok;
}

}
