
#include "gui/mrvDoubleWindow.h"

mrvDoubleWindow::mrvDoubleWindow( int x, int y, int w, int h, 
                                  const char* l ) :
Fl_Double_Window( x, y, w, h, l )
{
}

mrvDoubleWindow::mrvDoubleWindow( int w, int h, const char* l ) :
Fl_Double_Window( w, h, l )
{
}

bool mrvDoubleWindow::exec() 
{ 
    set_modal(); show(); return _exec;
};
