#include <math.h>
#include <iostream>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include "gui/mrvImageView.h"
#include "gui/mrvMultilineInput.h"
#include "mrViewer.h"

namespace mrv {

    const int kCrossSize = 10;

static char* underline_at;

/* If called with maxbuf==0, use an internally allocated buffer and enlarge it as needed.
 Otherwise, use buf as buffer but don't go beyond its length of maxbuf.
*/
    static const char* expand_text_(const char* from, char*& buf, int maxbuf, double maxw, int& n,
                                    double &width, int wrap, int draw_symbols) {
        char* e = buf+(maxbuf-4);
        underline_at = 0;
        double w = 0;
        static int l_local_buff = 500;
        static char *local_buf = (char*)malloc(l_local_buff); // initial buffer allocation
        if (maxbuf == 0) {
            buf = local_buf;
            e = buf + l_local_buff - 4;
        }
        char* o = buf;
        char* word_end = o;
        const char* word_start = from;

        const char* p = from;
        for (;; p++) {

            int c = *p & 255;

            if (!c || c == ' ' || c == '\n') {
                if (!c) break;
                else if (c == '\n') {p++; break;}
                word_start = p+1;
            }

            if (o > e) {
                if (maxbuf) break; // don't overflow buffer
                l_local_buff += int(o - e) + 200; // enlarge buffer
                buf = (char*)realloc(local_buf, l_local_buff);
                e = buf + l_local_buff - 4; // update pointers to buffer content
                o = buf + (o - local_buf);
                word_end = buf + (word_end - local_buf);
                local_buf = buf;
            }

            if (c == '\t') {
                for (c = fl_utf_nb_char((uchar*)buf, (int) (o-buf) )%8; c<8 && o<e; c++)
                    *o++ = ' ';
            } else if (c < ' ' || c == 127) { // ^X
                *o++ = '^';
                *o++ = c ^ 0x40;
            } else {
                *o++ = c;
            }
        }

        width = w + fl_width(word_end, (int) (o-word_end));
        *o = 0;
        n = (int) (o-buf);
        return p;
    }

    void measure(const char* str, int& w, int& h, int draw_symbols = 0) {
        h = fl_height();
        if (!str || !*str) {w = 0; return;}
        const char* p;
        const char* e;
        char* linebuf = NULL;
        int buflen;
        int lines;
        double width=0;
        int W = 0;

        for (p = str, lines=1; p;) {
            e = expand_text_(p, linebuf, 0, w, buflen, width,
                             w != 0, draw_symbols);
            if ((int)ceil(width) > W) W = (int)ceil(width);
            if ( *(e-1) == '\n' ) lines++;
            if (!*e ) break;
            p = e;
        }

        w = W;
        h = lines*h;
    }


    void MultilineInput::draw()
    {
        const Fl_Boxtype b = box();
        draw_box(b, color());
        const int X = x()+Fl::box_dx(b)+kCrossSize;
        const int Y = y()+Fl::box_dy(b)+kCrossSize;
        const int W = w()-Fl::box_dw(b);
        const int H = h()-Fl::box_dh(b);
        Fl_Input_::drawtext( X, Y, W, H );

        // if widget has focus, draw an X on corner
        if ( Fl::focus() == this )
        {
            const char* text = value();
            if ( text && strlen(text) > 0 )
                fl_color( 0, 255, 0 );
            else
                fl_color( 255, 0, 0 );
            fl_line_style( FL_SOLID, 3 );
            fl_line( x(), y(), x() + kCrossSize, y() + kCrossSize );
            fl_line( x() + kCrossSize, y(), x(), y() + kCrossSize );
        }

    }

    int MultilineInput::accept()
    {
        int ret = 0;
        ImageView* view = (ImageView*) window();

        GLShapeList& shapes = view->shapes();
        if ( shapes.empty() ) return 0;

        GLTextShape* s = dynamic_cast< GLTextShape* >( shapes.back().get() );
        if ( !s ) return 0;

        const char* text = value();
        if ( text && strlen(text) > 0 )
        {
            s->font( textfont() );
            s->size( textsize() / view->zoom() );
            s->text( text );

            mrv::media fg = view->foreground();
            if ( !fg ) return 0;

            CMedia* img = fg->image();
            if (!img) return 0;

            ret = 1;
            const Fl_Boxtype b = box();
            double xf = x() + Fl::box_dx(b) + kCrossSize + 1;
            double yf = y() + Fl::box_dy(b) + textsize() +
                        kCrossSize - 5;

            view->data_window_coordinates( img, xf, yf );

            yf = -yf;

            const mrv::Recti& daw = img->data_window();
            xf += daw.x();
            yf -= daw.y();

            s->pts[0].x = xf;
            s->pts[0].y = yf;
        }
        else
        {
            ret = 0;
            shapes.pop_back();
            if ( shapes.empty() )
            {
                view->main()->uiPaint->uiUndoDraw->deactivate();
                view->main()->uiUndoDraw->deactivate();
            }
        }

        window()->remove( this );
        delete this;

        return ret;

    }

    int MultilineInput::handle( int e )
    {
        switch( e )
        {
        case FL_PUSH:
        {
            const int button = Fl::event_button();
            if ( button == FL_LEFT_MOUSE )
            {
                if ( Fl::event_inside( x(), y(), kCrossSize, kCrossSize ) )
                {
                    if ( value() && strlen( value() ) )
                        return accept();
                    else
                    {
                        ImageView* view = (ImageView*) window();
                        view->undo_draw();
                    }
                    return 1;
                }
                // Adjust Fl::event_x() to compensate for cross
                if ( Fl::event_inside(this) ) Fl::e_x -= kCrossSize;
            }
            break;
        }
        case FL_MOVE:
        {
            if ( Fl::event_inside( x(), y(), kCrossSize, kCrossSize ) )
            {
                window()->cursor( FL_CURSOR_ARROW );
                return 1;
            }
            break;
        }
        }

        const int ret = Fl_Multiline_Input::handle( e );
        if ( e == FL_KEYBOARD )
        {
            int W = 0, H = 0;
            fl_font( textfont(), textsize() );
            measure( value(), W, H );
            W += kCrossSize * 2 + 10;
            H += kCrossSize * 2;
            size( W, H );
            redraw();
        }
        return ret;
    }

} // namespace mrv
