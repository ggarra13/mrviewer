

#include <fltk/events.h>

#include "gui/mrvActionWindow.h"
#include "gui/mrvImageView.h"
#include "mrViewer.h"

namespace mrv {

ImageView* ActionWindow::view() const {
    return uiMain->uiView;
}

int ActionWindow::handle( int event )
{
    int ok = 0;
    if ( event == fltk::KEY )
        ok = view()->handle( event );
    if ( !ok ) ok = fltk::Window::handle( event );
    return ok;
}

}
