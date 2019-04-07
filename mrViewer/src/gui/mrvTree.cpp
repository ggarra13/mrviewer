
#include <FL/Fl_XPM_Image.H>
#include "FLU/flu_pixmaps.h"
#include "gui/mrvTree.h"

namespace mrv
{

PreferencesTree::PreferencesTree( int x, int y, int w, int h, const char* l ) :
Fl_Tree( x, y, w, h, l )
{
    {
	Fl_Pixmap closed_icon( folder_closed_xpm );
	Fl_RGB_Image* img = new Fl_RGB_Image( &closed_icon );
	openicon( img );
    }

    
    {
	Fl_Pixmap opened_icon( folder_open_xpm );
	Fl_RGB_Image* img = new Fl_RGB_Image( &opened_icon );
	closeicon( img );
    }

    
}

}
