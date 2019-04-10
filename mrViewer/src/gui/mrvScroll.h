

#ifndef mrvScroll_h
#define mrvScroll_h

#include <FL/Fl_Scroll.H>

namespace mrv
{

class Scroll : public Fl_Scroll
{
    int nchild;
  public:
    Scroll(int X, int Y, int W, int H, const char* L=0) :
    Fl_Scroll(X,Y,W,H,L)
    {
        nchild = 0;
    }
    
    void resize(int X, int Y, int W, int H) {
        // Tell children to resize to our new width
        for ( int t=0; t<nchild; t++ ) {
            Fl_Widget *w = child(t);
            w->resize(w->x(), w->y(), W-20, w->h());    // W-20: leave room for scrollbar
        }
        // Tell scroll children changed in size
        init_sizes();
        Fl_Scroll::resize(X,Y,W,H);
    }

    void add( Fl_Widget* w )
    {
	Fl_Group* g = dynamic_cast< Fl_Group* >( w );
	if ( g )
	{
	    std::cerr << "Added " << g->child(0)->label() << std::endl;
	}
	Fl_Group::add( w );
	++nchild;
	redraw();
    }

};

} // namespace mrv

#endif // mrvScroll_h
