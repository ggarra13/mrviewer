
#include "icons/icons.c"
#include "gui/mrvTree.h"

namespace mrv
{

PreferencesTree::PreferencesTree( int x, int y, int w, int h, const char* l ) :
Fl_Tree( x, y, w, h, l )
{
    closeicon( closed_icon );
}

}
