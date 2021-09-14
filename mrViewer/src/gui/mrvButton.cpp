#include <iostream>
#include "mrvButton.h"

#include <FL/fl_draw.H>


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


    CheckButton::CheckButton( int X, int Y, int W, int H, const char* L ) :
        Fl_Check_Button( X, Y, W, H, L )
    {
    }

    void
    CheckButton::draw()
    {
        if (box()) draw_box(this==Fl::pushed() ? fl_down(box()) : box(), color());
        Fl_Color col = value() ? (active_r() ? selection_color() :
                                  fl_inactive(selection_color())) : color();

        int W  = labelsize();
        int bx = Fl::box_dx(box());   // box frame width
        int dx = bx + 2;              // relative position of check mark etc.
        int dy = (h() - W) / 2;       // neg. offset o.k. for vertical centering
        int lx = 0;                   // relative label position (STR #3237)

        if (down_box()) {
            // draw other down_box() styles:
            switch (down_box()) {
            case FL_DOWN_BOX :
            case FL_UP_BOX :
            case _FL_PLASTIC_DOWN_BOX :
            case _FL_PLASTIC_UP_BOX :
                // Check box...
                draw_box(down_box(), x()+dx, y()+dy, W, W, FL_BACKGROUND2_COLOR);
                if (value()) {
                    fl_color(col);
                    int tx = x() + dx + 3;
                    int tw = W - 6;
                    int d1 = tw/3;
                    int d2 = tw-d1;
                    int ty = y() + dy + (W+d2)/2-d1-2;
                    for (int n = 0; n < 3; n++, ty++) {
                        fl_line(tx, ty, tx+d1, ty+d1);
                        fl_line(tx+d1, ty+d1, tx+tw-1, ty+d1-d2+1);
                    }
                }
                break;
            case _FL_ROUND_DOWN_BOX :
            case _FL_ROUND_UP_BOX :
                // Radio button...
                draw_box(down_box(), x()+dx, y()+dy, W, W, FL_BACKGROUND2_COLOR);
                if (value()) {
                    int tW = (W - Fl::box_dw(down_box())) / 2 + 1;
                    if ((W - tW) & 1) tW++; // Make sure difference is even to center
                    int tdx = dx + (W - tW) / 2;
                    int tdy = dy + (W - tW) / 2;

                    if (Fl::is_scheme("gtk+")) {
                        fl_color(FL_SELECTION_COLOR);
                        tW --;
                        fl_pie(x() + tdx - 1, y() + tdy - 1, tW + 3, tW + 3, 0.0, 360.0);
                        fl_color(fl_color_average(FL_WHITE, FL_SELECTION_COLOR, 0.2f));
                    } else fl_color(col);

                    switch (tW) {
                        // Larger circles draw fine...
                    default :
                        fl_pie(x() + tdx, y() + tdy, tW, tW, 0.0, 360.0);
                        break;

                        // Small circles don't draw well on many systems...
                    case 6 :
                        fl_rectf(x() + tdx + 2, y() + tdy, tW - 4, tW);
                        fl_rectf(x() + tdx + 1, y() + tdy + 1, tW - 2, tW - 2);
                        fl_rectf(x() + tdx, y() + tdy + 2, tW, tW - 4);
                        break;

                    case 5 :
                    case 4 :
                    case 3 :
                        fl_rectf(x() + tdx + 1, y() + tdy, tW - 2, tW);
                        fl_rectf(x() + tdx, y() + tdy + 1, tW, tW - 2);
                        break;

                    case 2 :
                    case 1 :
                        fl_rectf(x() + tdx, y() + tdy, tW, tW);
                        break;
                    }

                    if (Fl::is_scheme("gtk+")) {
                        fl_color(fl_color_average(FL_WHITE, col, 0.5));
                        fl_arc(x() + tdx, y() + tdy, tW + 1, tW + 1, 60.0, 180.0);
                    }
                }
                break;
            default :
                draw_box(down_box(), x()+dx, y()+dy, W, W, col);
                break;
            }
            lx = dx + W + 2;
        } else {
            // if down_box() is zero, draw light button style:
            int hh = h()-2*dy - 2;
            int ww = W/2+1;
            int xx = dx;
            if (w()<ww+2*xx) xx = (w()-ww)/2;
            if (Fl::is_scheme("plastic")) {
                col = active_r() ? selection_color() : fl_inactive(selection_color());
                fl_color(value() ? col : fl_color_average(col, FL_BLACK, 0.5f));
                fl_pie(x()+xx, y()+dy+1, ww, hh, 0, 360);
            } else {
                draw_box(FL_THIN_DOWN_BOX, x()+xx, y()+dy+1, ww, hh, col);
            }
            lx = dx + ww + 2;
        }
        draw_label(x()+lx, y(), w()-lx-bx, h());
        if (Fl::focus() == this) draw_focus();
    }

    RadioButton::RadioButton( int X, int Y, int W, int H, const char* L ) :
        Fl_Radio_Button( X, Y, W, H, L )
    {
    }

    void
    RadioButton::draw()
    {
        Fl_Radio_Button::draw();
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
