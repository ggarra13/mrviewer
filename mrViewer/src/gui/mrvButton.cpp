#include <iostream>
#include "mrvButton.h"



namespace mrv
{


    Button::Button( int X, int Y, int W, int H, const char* L ) :
        Fl_Button( X, Y, W, H, L )
    {
        box( FL_FLAT_BOX ); // here it does not work
    }

    void Button::draw()
    {
        if (type() == FL_HIDDEN_BUTTON) return;
        Fl_Color col = value() ? selection_color() : color();
        box( FL_FLAT_BOX );  // here it works
        draw_box(value() ? (down_box()?down_box():fl_down(box())) : box(), col);
        draw_backdrop();
        if ( value() ) labelcolor( FL_CYAN );
        else labelcolor( 28 );
        draw_label();
        if (Fl::focus() == this) draw_focus();
    }


    Toggle_Button::Toggle_Button( int X, int Y, int W, int H, const char* L ) :
        Fl_Toggle_Button( X, Y, W, H, L )
    {
        box( FL_FLAT_BOX ); // here it does not work
        down_box( FL_PLASTIC_ROUND_DOWN_BOX );
    }

    void Toggle_Button::draw()
    {
        if (type() == FL_HIDDEN_BUTTON) return;
        Fl_Color col = value() ? selection_color() : color();
        box( FL_FLAT_BOX );  // here it works
        draw_box(value() ? down_box() : box(), col);
        draw_backdrop();
        if ( value() ) labelcolor( FL_CYAN );
        else labelcolor( 28 );
        draw_label();
        if (Fl::focus() == this) draw_focus();
    }

}
