/*
   mrViewer - the professional movie and flipbook playback
   Copyright (C) 2007-2022  Gonzalo Garramuño

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvTimeline.cpp
 * @author gga
 * @date   Fri Oct 13 13:36:04 2006
 *
 * @brief  An Fl_Widget to draw a timeline.
 *
 *
 */

#include "core/mrvI8N.h"
#include <cassert>
#include <cmath>  // for fabs()

#include <core/mrvRectangle.h>
#include <FL/fl_draw.H>

#include "core/mrvColor.h"
#include "core/mrvThread.h"

#include "gui/mrvImageBrowser.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvImageView.h"
#include "gui/mrvHotkey.h"
#include "mrViewer.h"
#include "mrvPreferencesUI.h"
#include "mrvReelUI.h"
#include "gui/mrvMedia.h"
#include "gui/mrvMediaList.h"

namespace
{
// Maximum number of frames to show cacheline for.  Setting it too high can
// impact GUI playback when the image/movies are that long.
unsigned kMAX_FRAMES = 5000;
double kMinFrame = std::numeric_limits<double>::min();
double kMaxFrame = std::numeric_limits<double>::max();
}


namespace mrv
{

mrv::Timecode::Display Timeline::_display = Timecode::kFrames;

Timeline::Timeline( int x, int y, int w, int h, char* l ) :
mrv::Slider( x, y, w, h, l ),
_edl( false ),
_draw_annotation( true ),
_draw_cache( true ),
_tc( 0 ),
_fps( 24 ),
_display_min( 1 ),
_display_max( 50 ),
_undo_display_min( 1 ),
_undo_display_max( 50 ),
image( NULL ),
win( NULL ),
uiMain( NULL )
{
    type( TICK_ABOVE );
    slider_type( kNORMAL );
    Fl_Slider::minimum( 1 );
    Fl_Slider::maximum( 50 );
}

Timeline::~Timeline()
{
    _draw_cache = _draw_annotation = false;
    _edl = false;
    uiMain = NULL;
    delete win; win = NULL;
}

mrv::ImageBrowser* Timeline::browser() const
{
    assert( uiMain != NULL );
    assert( uiMain->uiReelWindow != NULL );
    if ( uiMain == NULL ) return NULL;
    if ( uiMain->uiReelWindow == NULL ) return NULL;
    return uiMain->uiReelWindow->uiBrowser;
}

void Timeline::display_minimum( double x )
{
    if ( x > _display_max ) x = _display_max;
    if ( x >= minimum() ) {
        //if ( _edl )
            _undo_display_min = _display_min;
        _display_min = x;
        if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
        {
            Fl_Slider::minimum( x );
        }
    }


    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMinDisplay %lf"), x );
        uiMain->uiView->send_network( buf );
    }
}

void Timeline::undo_display_minimum()
{
    if ( _undo_display_min == AV_NOPTS_VALUE ) return;
    _display_min = _undo_display_min;
    if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    {
        Fl_Slider::minimum( _display_min );
    }
    _undo_display_min = AV_NOPTS_VALUE;
    redraw();
}

void Timeline::undo_display_maximum()
{
    if ( _undo_display_max == AV_NOPTS_VALUE ) return;
    _display_max = _undo_display_max;
    if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    {
        Fl_Slider::maximum( _display_max );
    }
    _undo_display_max = AV_NOPTS_VALUE;
    redraw();
}

void Timeline::display_maximum( double x )
{
    if ( x < _display_min ) x = _display_min;
    if ( x <= maximum() ) {
        //if ( _edl )
            _undo_display_max = _display_max;
        _display_max = x;
        if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
        {
            Fl_Slider::maximum( x );
        }
    }


    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMaxDisplay %lf"), x );
        uiMain->uiView->send_network( buf );
    }
}

void Timeline::minimum( double x )
{
    if ( x > _display_max ) x = _display_max;

    Fl_Slider::minimum( x );
    _display_min = x;

    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMin %lf"), x );
        uiMain->uiView->send_network( buf );
    }
}

void Timeline::maximum( double x )
{
    if ( x < _display_min ) x = _display_min;

    Fl_Slider::maximum( x );
    _display_max = x;

    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMax %lf"), x );
        uiMain->uiView->send_network( buf );
    }
}

void Timeline::edl( bool x )
{
    _edl = x;


    if ( _edl && uiMain && browser() )
    {
        mrv::Timecode* uiFrame = uiMain->uiFrame;

        // Calculate frame range for timeline
        minimum( 1 );
        if ( uiMain->uiStartFrame )
            uiMain->uiStartFrame->frame( 1 );
        if ( uiMain->uiFrame && uiMain->uiFrame->value() < 1 )
            uiFrame->frame(1);

        uint64_t total = 0;

        const mrv::Reel& reel = browser()->current_reel();
        if ( !reel ) return;

        mrv::MediaList::const_iterator i = reel->images.begin();
        mrv::MediaList::const_iterator e = reel->images.end();

        for ( ; i != e; ++i )
        {
            CMedia* img = (*i)->image();
            if ( (*i)->position() == MRV_NOPTS_VALUE )
                (*i)->position( total );
            total += img->duration();
        }

        maximum( double(total) );
        if ( uiMain->uiEndFrame ) uiMain->uiEndFrame->frame( total );
        if ( uiFrame && uiFrame->value() > int64_t(total) )
            uiFrame->frame(total);
    }

    redraw();
}
void Timeline::draw_ticks(const mrv::Recti& r, int min_spacing)
{
    int x1, y1, x2, y2, dx, dy, w;
    x1 = x2 = r.x() + int(slider_size()-1)/2;
    dx = 1;
    y1 = r.y();
    y2 = r.b()-1;
    dy = 0;
    w = r.w();



    fl_push_clip( r.x(), r.y(), r.w(), r.h() );

    if (w <= 0) return;

    double A,B;
    if ( ! uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() &&
         ( display_minimum() > minimum() || display_maximum() < maximum() ) )
    {
        A = display_minimum();
        B = display_maximum();
        if (A > B) {
            A = B;
            B = display_minimum();
        }
    }
    else
    {
        A = minimum();
        B = maximum();
        if (A > B) {
            A = B;
            B = minimum();
        }
    }
    //if (!finite(A) || !finite(B)) return;

    if (min_spacing < 1) min_spacing = 10; // fix for fill sliders
    // Figure out approximate size of min_spacing at zero:

    double mul = 1; // how far apart tick marks are
    double div = 1;
    int smallmod = 5; // how many tick marks apart "larger" ones are
    int nummod = 15; // how many tick marks apart numbers are

    if ( _display != Timecode::kFrames )
    {
        nummod = int(_fps);
    }

    int powincr = 10000;

    double derivative = (B-A)*min_spacing/w;
    if (derivative < step()) derivative = step();
    while (mul*5 <= derivative) mul *= 10;
    while (mul > derivative*2*div) div *= 10;
    if (derivative*div > mul*2) {
        mul *= 5;
        smallmod = 2;
    }
    else if (derivative*div > mul) {
        mul *= 2;
        nummod /= 2;
    }
    if ( nummod <= 1 ) nummod = 1;

    Fl_Color textcolor = fl_contrast( labelcolor(), color() );
    if ( _edl ) textcolor = FL_BLACK;
    Fl_Color linecolor = FL_BLACK;
    if ( Preferences::schemes.name == "Black" )
    {
        linecolor = fl_inactive( fl_contrast( fl_rgb_color( 70, 70, 70 ),
                                              selection_color() ) );
    }

    fl_color(linecolor);
    char buffer[32];
    fl_font(labelfont(), labelsize());
    for (int n = 0; ; n++) {
        // every ten they get further apart for log slider:
        if (n > powincr) {
            mul *= 10;
            n = (n-1)/10+1;
        }
        double v = mul*n/div;
        if (v > fabs(A) && v > fabs(B)) break;
        int sm = n%smallmod ? 3 : 0;
        if (v >= A && v <= B) {
            int t = slider_position(v, w);
            int x1dxt = x1 + dx*t;
            int y1dyt = y1 + dy*t;
            fl_line(x1dxt+dy*sm, y1dyt+dx*sm, x2+dx*t, y2+dy*t);
            if (n-1 != 0 && (n-1)%nummod == 0) {
                mrv::Timecode::format( buffer, _display, int64_t(v),
                                       _tc, _fps );
                fl_color(textcolor);
                int wt = 0, ht = 0;
                fl_measure( buffer, wt, ht );
                fl_draw(buffer, x1dxt-wt/2, y1dyt+fl_height()-fl_descent());
                fl_color(linecolor);
            }
        }
        if (v && -v >= A && -v <= B) {
            int t = slider_position(-v, w);
            int x1dxt = x1 + dx*t;
            int y1dyt = y1 + dy*t;
            fl_line(x1dxt+dy*sm, y1dyt+dx*sm, x2+dx*t, y2+dy*t);
            if (n%nummod == 0) {
                TRACE2( "v = " << v );
                mrv::Timecode::format( buffer, _display, int64_t(-v), _tc,
                                       _fps );
                fl_color(textcolor);
                // int wt = 0, ht = 0;
                // measure( p, wt, ht );
                fl_draw(buffer, x1dxt, y1dyt+fl_height()-fl_descent());
                fl_color(linecolor);
            }
        }
    }

    fl_pop_clip();
}

/*!
  Subclasses can use this to redraw the moving part.
  Draw everything that is inside the box, such as the tick marks,
  the moving slider, and the "slot". The slot only drawn if \a slot
  is true. You should already have drawn the background of the slider.
*/
bool Timeline::draw(const mrv::Recti& sr, int flags, bool slot)
{
    // for back compatability, use type flag to set slider size:
    if (type()&16/*FILL*/) slider_size(0);


    // draw the tick marks and inset the slider drawing area to clear them:
    if (tick_size() && (type()&TICK_BOTH)) {
        fl_color(fl_inactive(fl_contrast(labelcolor(),color())));
        draw_ticks(sr, int(slider_size()+1)/2);
    }

    // if user directly set selected_color we use it:
    if ( selection_color() ) {
        Fl::set_box_color( selection_color() );
        fl_color(fl_contrast(labelcolor(), selection_color()));
    }

    return true;
}

void Timeline::draw_cacheline( CMedia* img, int64_t pos, int64_t size,
                               int64_t mn, int64_t mx, int64_t frame,
                               const mrv::Recti& r )
{

    int64_t j = frame;


    // if ( !img->has_video() && pos < j ) j = pos;

    int64_t max = frame + size;
    if ( mx < max ) max = mx;


    // If too many frames, playback suffers, so we exit here
    if ( max - j > uiMain->uiPrefs->uiPrefsMaxCachelineFrames->value() ) return;

    int rx = r.x() + int(slider_size()-1)/2;
    int ry = r.y() + r.h()/2;
    int ww = r.w();
    int hh = r.h() - 8;

    fl_push_clip( rx, ry, ww, hh );

    CMedia::Cache c = CMedia::kLeftCache;
    fl_color( FL_DARK_GREEN );
    fl_line_style( FL_SOLID, 1 );

    if ( ( img->stereo_output() != CMedia::kNoStereo &&
            img->stereo_output() != CMedia::kStereoLeft ) ||
            img->stereo_input() > CMedia::kSeparateLayersInput )
    {
        c = CMedia::kStereoCache;
        fl_color( FL_GREEN );
    }



#define NO_FRAME_VALUE std::numeric_limits<int>::min()

    int dx = NO_FRAME_VALUE;


    while ( j <= max )
    {
        dx = NO_FRAME_VALUE;
        int64_t t = j - pos + 1;
        for ( ; j < max; ++j, ++t )
        {
            if ( img->is_cache_filled( t ) >= c )
            {
                dx = rx + slider_position( double(j), ww );
                break;
            }
        }

        if ( dx == NO_FRAME_VALUE )
            break;

        t = j - pos + 1;
        for ( ; j <= max; ++j, ++t )
        {
            if ( img->is_cache_filled( t ) < c )
            {
                int dx2 = rx + slider_position( double(j), ww );
                int wh = dx2-dx;
                fl_rectf( dx, ry, wh, hh );
                dx = NO_FRAME_VALUE;
                break;
            }
        }
    }

    int64_t t = j - pos;  // not +1
    if ( dx != NO_FRAME_VALUE && img->is_cache_filled( t ) >= c )
    {
        int dx2 = rx + slider_position( double(j), ww );
        int wh = dx2-dx;
        fl_rectf( dx, ry, wh, hh );
    }

    fl_pop_clip();


}


void Timeline::draw_selection( const mrv::Recti& r )
{
    int rx = r.x() + int(slider_size()-1)/2;
    int  dx = slider_position( _display_min, r.w() );
    int end = slider_position( _display_max, r.w() );

    fl_color( FL_CYAN );
    fl_rectf( rx+dx, r.y(), end-dx, r.h()-8 );

}

    void showwin(mrv::Timeline* self)
    {
        self->show_thumb();
    }

    void Timeline::show_thumb()
    {
        int WX = window()->x();
        int WY = window()->y();
        int X = event_x - 64;
        int Y = y() - 80;

        if ( Y < 0 ) return;

        Fl_Box* b = NULL;
        if (! win ) {
            win = new Fl_Double_Window( X, Y, 128, 76 );
            win->parent( window() );
            win->border(0);
            win->begin();
            b = new Fl_Box( 0, 0, win->w(), win->h() );
            b->box( FL_FLAT_BOX );
            b->labelcolor( fl_contrast( b->labelcolor(), b->color() ) );
        }
        else {
            win->resize( X, Y, 128, 76 );
            b = (Fl_Box*)win->child(0);
        }

        mrv::media m;
        if ( _edl )
            m = media_at( frame );
        else
            m = browser()->current_image();
        if ( ! m ) {
            win->hide();
            return;
        }
        if ( !fg || strcmp( fg->image()->fileroot(), m->image()->fileroot() )
             != 0 )
        {
            image = CMedia::guess_image( m->image()->fileroot(), NULL, 0, true );
            if ( !image ) return;
            fg.reset( new mrv::gui::media( image ) );
        }
        else
        {
            image = fg->image();
        }
        int64_t global = frame;
        frame = global_to_local( frame );
        image->seek( frame );
        fg->create_thumbnail();
        char buf[64];
        Timecode::Display display;
        Timecode::format( buf, _display,
                          global, _tc,
                          image->fps(), true );
        b->copy_label( buf );
        b->image( fg->thumbnail() );
        b->redraw();
        win->end();
        win->cursor( FL_CURSOR_DEFAULT );
        win->show();
    }

int Timeline::handle( int e )
{
    if ( e == FL_ENTER ) {
        window()->cursor( FL_CURSOR_DEFAULT );
        return 1;
    }
    else if ( e == FL_MOVE || e == FL_DRAG || e == FL_PUSH )
    {
        window()->cursor( FL_CURSOR_DEFAULT );
        event_x = Fl::event_x();
        int X = x()+Fl::box_dx(box());
        int Y = y()+Fl::box_dy(box());
        int W = w()-Fl::box_dw(box());
        int H = h()-Fl::box_dh(box());

        // HERE, SET A VALUE TO CURRENT HOVERING PLACE,
        // *NOT* TO value().
        double val;
        if (minimum() == maximum())
            val = 0.5;
        else {
            val = (value()-minimum())/(maximum()-minimum());
            if (val > 1.0) val = 1.0;
            else if (val < 0.0) val = 0.0;
        }

        int ww = W;
        int mx = Fl::event_x()-X;
        int S;
        static int offcenter;

        S = int(slider_size()*ww+.5); if (S >= ww) return 0;
        int T = H / 2+1;
        if (type()==FL_HOR_NICE_SLIDER) T += 4;
        if (S < T) S = T;

        if ( e == FL_MOVE ) {
            int xx = int(val*(ww-S)+.5);
            offcenter = mx-xx;
            if (offcenter < 0) offcenter = 0;
            else if (offcenter > S) offcenter = S;
            else return 1;
        }

        int xx = mx-offcenter;
        double v = 0;
        if (xx < 0) {
            xx = 0;
            offcenter = mx; if (offcenter < 0) offcenter = 0;
        } else if (xx > (ww-S)) {
            xx = ww-S;
            offcenter = mx-xx; if (offcenter > S) offcenter = S;
        }
        v = round(xx*(maximum()-minimum())/(ww-S) + minimum());
        frame = clamp(v);

        if ( uiMain->uiPrefs->uiPrefsTimelineThumbnails->value() )
        {
            if ( Fl::has_timeout( (Fl_Timeout_Handler)showwin, this ) )
                Fl::remove_timeout( (Fl_Timeout_Handler)showwin, this );
            Fl::add_timeout( 0.01, (Fl_Timeout_Handler)showwin, this );
        }
    }
    else if ( e == FL_LEAVE )
    {
        Fl::remove_timeout( (Fl_Timeout_Handler)showwin, this );
        if (win) win->hide();
    }
    else if ( e == FL_KEYDOWN )
    {
        unsigned int rawkey = Fl::event_key();
        if ( kPlayBack.match( rawkey ) ||
             kPlayFwd.match( rawkey ) ||
             kPlayDirection.match( rawkey ) ||
             kPlayFwdTwiceSpeed.match( rawkey ) ||
             kPlayBackHalfSpeed.match( rawkey ) ||
             kShapeFrameStepFwd.match( rawkey ) ||
             kShapeFrameStepBack.match( rawkey ) ||
             kStop.match( rawkey ) )
        {
            return uiMain->uiView->handle( e );
        }
    }
    Fl_Boxtype bx = box();
    box( FL_FLAT_BOX );
    int ok = mrv::Slider::handle( e );
    box( bx );
    return ok;
    // if ( r != 0 ) return r;
    // return uiMain->uiView->handle( e );
}

/**
 * Main widget drawing routine
 *
 */
void Timeline::draw()
{
    // Flags flags = this->flags();
    // Flags f2 = flags & ~FOCUSED;
    // if (pushed()) f2 |= PUSHED;
    // flags &= ~HIGHLIGHT;

    int f2 = 0;

    // drawstyle(style(),flags);


    int X = x() + Fl::box_dx(box());
    int Y = y() + Fl::box_dy(box());
    int W = w() - Fl::box_dw(box());
    int H = h() - Fl::box_dh(box());

    mrv::Recti r( X, Y, W, H );


    draw_box();


    // Get number of frames
    double mn = minimum();
    double mx = maximum();

    if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    {
        mn = display_minimum();
        mx = display_maximum();
    }

    double v  = uiMain->uiView->frame(); //value(); // @todo: should be value()

    if ( !browser() ) return;


    // Draw each rectangle for each segment
    if ( _edl )
    {

        const mrv::Reel& reel = browser()->current_reel();
        if ( !reel ) return;

        mrv::MediaList::const_iterator i = reel->images.begin();
        mrv::MediaList::const_iterator e = reel->images.end();

        _fps = 24.0;

        int ww = r.w();

        // If minimum less than 0, start boxes later
        uint64_t size = 0;
        uint64_t frame = 1;
        int rx = r.x() + int(slider_size()-1)/2;

        CMedia* img = NULL;
        for ( ; i != e; frame += size, ++i )
        {
            int64_t pos = (*i)->position();
            img = (*i)->image();

            size = img->duration();


            // skip this block if outside visible timeline span
            if ( frame + size < mn || frame > mx ) continue;

            int  dx = slider_position( double(frame),      ww );
            int end = slider_position( double(frame+size), ww );

            mrv::Recti lr( rx+dx, r.y(), end-dx, r.h() );

            // Draw a block
            if ( v >= frame && v < frame + size )
            {
                fl_color( fl_darker( FL_YELLOW ) );
            }
            else
            {
                fl_color( fl_lighter( labelcolor() ) );
            }

            fl_rectf( lr.x(), lr.y(), lr.w(), lr.h() );
        }

        if ( img ) _fps = img->fps();

        if ( ( ! uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() ) &&
             ( _display_min > minimum() || _display_max < maximum() ) )
        {
            draw_selection(r);
        }

        frame = 1;
        unsigned idx = 0;
        mrv::media fg = browser()->current_image();

        for ( i = reel->images.begin(); i != e; frame += size, ++i )
        {
            CMedia* img = (*i)->image();

            size = img->duration();
            int64_t pos = (*i)->position() - img->in_frame();


            // skip this block if outside visible timeline span
            if ( frame + size < mn || frame > mx ) continue;

            if ( _draw_cache && (*i) == fg )
            {
                draw_cacheline( img, pos, size, int64_t(mn),
                                int64_t(mx),
                                frame, r );
            }


            if ( draw_annotation() && (*i) == fg )
            {
                int rx = r.x() + int(slider_size()-1)/2;
                int ry = r.y() + r.h()/2;
                int ww = r.w();
                int hh = r.h() - 8;
                GLShapeList::const_iterator si = img->shapes().begin();
                GLShapeList::const_iterator se = img->shapes().end();
                hh = r.h() / 2;
                for ( ; si != se; ++si )
                {
                    int64_t f = (*si)->frame;
                    fl_color( FL_RED );
                    fl_line_style( FL_SOLID, 2 );
                    int dx = rx + slider_position( double(f), ww );
                    fl_xyline( dx, ry-hh, dx, ry+hh );
                }
            }

            int dx = rx + slider_position( double(frame), ww );

            fl_color( FL_BLUE );
            fl_line_style( FL_SOLID, 3 );
            fl_line( dx, r.y(), dx, r.b()-1 ); // -1 to compensate line style
            fl_line_style( FL_SOLID );
        }
    }
    else
    {
        if ( _draw_cache )
        {
            mrv::media m = browser()->current_image();
            if ( m )
            {
                CMedia* img = m->image();
                // CMedia::Mutex& mtx = img->video_mutex();
                // SCOPED_LOCK( mtx );
                boost::int64_t first = img->first_frame();
                draw_cacheline( img, 1,
                                img->duration() + img->start_number(),
                                int64_t(mn), int64_t(mx),
                                first, r );

            }
        }

        mrv::media m = browser()->current_image();
        if ( m )
        {
            CMedia* img = m->image();
            if ( draw_annotation() )
            {
                int rx = r.x() + int(slider_size()-1)/2;
                int hh = r.h() / 2;
                int ry = r.y() + hh;
                int ww = r.w();
                GLShapeList::const_iterator si = img->shapes().begin();
                GLShapeList::const_iterator se = img->shapes().end();
                for ( ; si != se; ++si )
                {
                    int64_t f = (*si)->frame;
                    fl_color( FL_RED );
                    fl_line_style( FL_SOLID, 2 );
                    int dx = rx + slider_position( double(f), ww );
                    fl_xyline( dx, ry-hh, dx, ry+hh );
                }
            }
        }

        if ( ( ! uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() ) &&
             ( _display_min > minimum() || _display_max < maximum() ) )
        {
            draw_selection(r);
        }

    }

    // return Fl_Slider::draw();

    draw( r, f2, r.y()==0 );

    // Draw cursor
    fl_push_clip( X, Y, W, H );
    X += draw_coordinate( value(), w() - Fl::box_dw(box()) );
    W = 15  - Fl::box_dw(box());
    Fl_Color c = fl_rgb_color( 180, 180, 128 );
    draw_box( FL_PLASTIC_UP_BOX, X, Y, W, H, c );
    clear_damage();
    fl_pop_clip();
}

/**
 * Given an image, return its offset from frame 1 when in edl mode
 *
 * @param img image to search in edl list
 *
 * @return offset in timeline
 */
uint64_t Timeline::offset( const CMedia* img ) const
{
    if ( !img ) return 0;

    mrv::Reel reel = browser()->current_reel();
    if (!reel) return 0;

    return reel->offset( img );
}

/**
 * Given a frame, return its image index in browser when in edl mode
 *
 * @param f frame to search in edl list
 *
 * @return index of image in image browser list
 */
size_t Timeline::index( const int64_t f ) const
{
    const mrv::Reel& reel = browser()->current_reel();
    if (!reel) return 0;

    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    double mn = display_minimum();
    double mx = display_maximum();
    if ( mn > mx )
    {
        double t = mx;
        mx = mn;
        mn = t;
    }

    if ( f < boost::int64_t(mn) ) return 0;
    if ( f > boost::int64_t(mx) ) return unsigned(e - i);

    int64_t  t = 1;
    size_t r = 0;
    for ( ; i != e; ++i, ++r )
    {
        CMedia* img = (*i)->image();
        uint64_t size = img->duration();
        t += size;
        if ( t > f ) break;
    }
    if ( r >= reel->images.size() ) r = reel->images.size() - 1;
    return r;
}

/**
 * Given a frame, return the image in browser for that frame when in edl mode
 *
 * @param f frame to search in edl list
 *
 * @return image at that point in the timeline or NULL
 */
mrv::media Timeline::media_at( const int64_t f ) const
{
    mrv::Reel reel = browser()->current_reel();
    if (!reel) return mrv::media();

    return reel->media_at( f );
}

CMedia* Timeline::image_at( const int64_t f ) const
{
    mrv::media m = media_at( f );
    if ( !m ) return NULL;
    return m->image();
}
/**
 * Given an image, return its offset from frame 1 when in edl mode
 *
 * @param img image to search in edl list
 *
 * @return offset in timeline
 */
int64_t Timeline::global_to_local( const int64_t frame ) const
{
    mrv::Reel reel = browser()->current_reel();
    if (!reel) return 0;

    if ( !_edl ) return frame;

    return reel->global_to_local( frame );
}

void change_timeline_display( ViewerUI* uiMain )
{
    int i = uiMain->uiTimecodeSwitch->value();
    select_character( uiMain->uiTimecodeSwitch, true );

    mrv::Timecode::Display d = (mrv::Timecode::Display) i;
    uiMain->uiFrame->display( d );
    uiMain->uiStartFrame->display( d );
    uiMain->uiEndFrame->display( d );
    uiMain->uiTimeline->display( d );
}

int Timeline::draw_coordinate( double value, int w )
{
    double A = minimum();
    double B = maximum();

    if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    {
        A = display_minimum();
        B = display_maximum();
    }

    if (B == A) return 0;
    bool flip = B < A;
    if (flip) {A = B; B = minimum();}
    if (!horizontal()) flip = !flip;
    // if both are negative, make the range positive:
    if (B <= 0) {flip = !flip; double t = A; A = -B; B = -t; value = -value;}
    double fraction;
    if (!(slider_type() & kLOG)) {
        // linear slider
    fraction = (value-A)/(B-A+1);
  } else if (A > 0) {
    // logatithmic slider
    if (value <= A) fraction = 0;
    else fraction = (::log(value)-::log(A))/(::log(B)-::log(A));
  } else if (A == 0) {
    // squared slider
    if (value <= 0) fraction = 0;
    else fraction = sqrt(value/B);
  } else {
    // squared signed slider
    if (value < 0) fraction = (1-sqrt(value/A))*.5;
    else fraction = (1+sqrt(value/B))*.5;
  }
  if (flip) fraction = 1-fraction;
  w -= slider_size(); if (w <= 0) return 0;
  return int(fraction*w+.5);
}

int Timeline::slider_position( double value, int w )
{
    double A = minimum();
    double B = maximum();

    if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    {
        A = display_minimum();
        B = display_maximum();
    }

    if (B == A) return 0;
    bool flip = B < A;
    if (flip) {A = B; B = minimum();}
    if (!horizontal()) flip = !flip;
    // if both are negative, make the range positive:
    if (B <= 0) {flip = !flip; double t = A; A = -B; B = -t; value = -value;}
    double fraction;
    if (!(slider_type() & kLOG)) {
        // linear slider
    fraction = (value-A)/(B-A+1);
  } else if (A > 0) {
    // logatithmic slider
    if (value <= A) fraction = 0;
    else fraction = (::log(value)-::log(A))/(::log(B)-::log(A));
  } else if (A == 0) {
    // squared slider
    if (value <= 0) fraction = 0;
    else fraction = sqrt(value/B);
  } else {
    // squared signed slider
    if (value < 0) fraction = (1-sqrt(value/A))*.5;
    else fraction = (1+sqrt(value/B))*.5;
  }
  if (flip) fraction = 1-fraction;
  w -= slider_size(); if (w <= 0) return 0;
  if (fraction >= 1) return w;
  else if (fraction <= 0) return 0;
  else return int(fraction*w+.5);
}

} // namespace mrv
