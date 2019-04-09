/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

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

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <limits>

#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>

#include "mrViewer.h"
#include "mrvReelUI.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvMediaTrack.h"
#include "gui/mrvEDLGroup.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvElement.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
#include "mrvEDLWindowUI.h"

namespace {
const char* kModule = "edl";
}

namespace mrv {

static int kTrackHeight = 88;
static int kXOffset = 64;

EDLGroup::EDLGroup(int x, int y, int w, int h) :
    Fl_Group(x,y,w,h),
    _drag( NULL ),
    _dragX( 0 ),
    _dragY( 0 )
{
}

EDLGroup::~EDLGroup()
{
    for ( int i = 0; i < children(); ++i )
    {
        delete child(i);
        remove( i );
    }

    _audio_track.clear();
}

ImageBrowser* EDLGroup::browser() const
{
    return uiMain->uiReelWindow->uiBrowser;
}

ImageView* EDLGroup::view() const
{
    return uiMain->uiView;
}

// Add a media track and return its index
unsigned EDLGroup::add_media_track( int r )
{
    unsigned e = children();
    if (e >= 2) return 0;

    mrv::media_track* o = new mrv::media_track(x(), y() + 94 * e,
                                               w(), kTrackHeight);

    o->main( timeline()->main() );
    this->add( o );

    o->reel( r );

    return e;
}

bool  EDLGroup::shift_audio( unsigned reel_idx, std::string s,
                             boost::int64_t f )
{

    for ( int i = 0; i < 2; ++i )
    {
        mrv::media_track* t = (mrv::media_track*)child(i);
        if ( t->reel() == reel_idx )
        {
            int idx = t->index_for(s);
            if ( idx < 0 ) return false;
            mrv::media m = t->media( idx );
            m->image()->audio_offset( f );
            t->refresh();
            return true;
        }
    }
    return false;
}
bool  EDLGroup::shift_media_start( unsigned reel_idx, std::string s,
                                   boost::int64_t f )
{

    for ( int i = 0; i < 2; ++i )
    {
        mrv::media_track* t = (mrv::media_track*)child(i);
        if ( t->reel() == reel_idx )
        {
            int idx = t->index_for(s);
            if ( idx < 0 ) return false;
            mrv::media m = t->media( idx );
            m->image()->first_frame( f );
            t->refresh();
            return true;
        }
    }
    return false;
}

bool  EDLGroup::shift_media_end( unsigned reel_idx, std::string s,
                                 boost::int64_t f )
{

    for ( int i = 0; i < 2; ++i )
    {
        mrv::media_track* t = (mrv::media_track*)child(i);
        if ( t->reel() == reel_idx )
        {
            int idx = t->index_for(s);
            if ( idx < 0 ) return false;
            mrv::media m = t->media( idx );
            m->image()->last_frame( f );
            t->refresh();
            return true;
        }
    }
    return false;
}

// Add an audio only track and return its index
unsigned EDLGroup::add_audio_track()
{
    // _audio_track.push_back( mrv::audio_track_ptr() );
    return unsigned( _audio_track.size() - 1 );
}

// Return the number of media tracks
unsigned EDLGroup::number_of_media_tracks()
{
    return children();
}

// Return the number of audio only tracks
unsigned EDLGroup::number_of_audio_tracks()
{
    return (unsigned) _audio_track.size();
}


// Return an audio track at index i
audio_track_ptr& EDLGroup::audio_track( unsigned i )
{
    return _audio_track[i];
}

// Remove a media track at index i
void EDLGroup::remove_media_track( unsigned i )
{
    if ( children() == 1 )
    {
        mrv::media_track* track = (mrv::media_track*) child(i);
        mrv::Reel reel = browser()->reel_at( track->reel() );
        reel->edl = false;
    }
    remove( i );
}


void static_move( EDLGroup* e )
{
    int X = Fl::event_x();

    if ( X < 8 ) X = 8;

    int quarter = e->w() / 4;

    if ( X >= e->x() + e->w() - quarter ) {
        e->pan(-1);
        Fl::repeat_timeout( 0.1f, (Fl_Timeout_Handler) static_move, e );
    }
    else if ( X <= e->x() + quarter )
    {
        e->pan(1);
        Fl::repeat_timeout( 0.1f, (Fl_Timeout_Handler) static_move, e );
    }

}

// Remove an audio track at index i
void EDLGroup::remove_audio_track( unsigned i )
{
    _audio_track.erase( _audio_track.begin() + i );
}


void EDLGroup::pan( int diff )
{

    mrv::Timeline* t = timeline();

    double amt = double(diff) / (double) t->w();
    double tmax = t->maximum();
    double tmin = t->minimum();
    double avg = tmax - tmin + 1;
    amt *= avg;

    t->minimum( tmin - amt );
    if ( t->minimum() < 0 ) t->minimum( 0 );

    t->maximum( tmax - amt );
    t->redraw();
    redraw();

}

int EDLGroup::handle( int event )
{
    switch( event )
    {
    case FL_PUSH:
    {
        if ( Fl::event_button() == FL_MIDDLE_MOUSE )
        {
            _dragX = Fl::event_x();
            return 1;
        }

        if ( Fl::event_button() == FL_LEFT_MOUSE )
        {
            _dragX = Fl::event_x();
            _dragY = Fl::event_y();

            // LIMITS
            if ( _dragX < x() + 8 ) _dragX = x() + 8;
            if ( _dragY < y() + 33 ) _dragY = y() + 33;

            int idx = int( (_dragY - y() ) / kTrackHeight );
            if ( idx < 0 || idx >= children() ) {
                return 0;
            }
            _dragChild = idx;

            mrv::Timeline* t = timeline();
            if ( !t ) return 0;

            int ww = t->w();
            double len = (t->maximum() - t->minimum() + 1);
            double p = double( _dragX - x() ) / double(ww);
            p = t->minimum() + p * len;
            int64_t pt = int64_t( p );



            mrv::media_track* track = (mrv::media_track*) child(idx);
	    if ( !track ) return 0;
	    

            mrv::media m = track->media_at( pt );


            if ( m )
            {

		_drag = ImageBrowser::new_item( m );

                int j = track->index_for( m );
                if ( j < 0 ) {

                    return 0;
                }


                browser()->reel( track->reel() );
                DBG("Change  image " << j );
                // browser()->change_image( j );

                view()->stop();

                view()->seek( pt );

                DBG("Changed image " << j );
                browser()->redraw();
                return 1;
            }
	    else
	    {

		delete _drag; _drag = NULL;

		return 1;
	    }
        }

	return Fl_Group::handle( event );
	
        // for ( int i = 0; i < children(); ++i )
        // {
        //     Fl_Widget* c = this->child(i);
        //     // if (Fl::event_x() < c->x() - kXOffset) continue;
        //     // if (Fl::event_x() >= c->x()+c->w()) continue;
        //     // if (Fl::event_y() < c->y() - y() ) continue;
        //     // if (Fl::event_y() >= c->y() - y() +c->h()) continue;
        //     if ( c->handle( event ) ) return 1;
        // }
        // return 0;
        break;
    }
    case FL_ENTER:
        focus(this);
        window()->show();
        return 1;
        break;
    case FL_FOCUS:
        return 1;
        break;
    case FL_KEYBOARD:
    {
        int key = Fl::event_key();

        if ( key == FL_Delete )
        {
            browser()->remove_current();
            return 1;
        }

        if ( kPlayBack.match( key ) )
        {

            mrv::ImageView* v = view();
            mrv::media fg = v->foreground();
            if ( ! fg ) return 1;

            const CMedia* img = fg->image();
            double FPS = 24;
            if ( img ) FPS = img->play_fps();
            v->fps( FPS );

            if ( v->playback() != CMedia::kStopped )
                v->stop();
            else
                v->play_backwards();
            return 1;
        }
        else if ( kPlayFwd.match( key ) )
        {

            mrv::ImageView* v = view();
            mrv::media fg = v->foreground();
            if ( ! fg ) return 1;

            const CMedia* img = fg->image();
            double FPS = 24;
            if ( img ) FPS = img->play_fps();
            v->fps( FPS );

            if ( v->playback() != CMedia::kStopped )
                v->stop();
            else
                v->play_forwards();
            return 1;
        }
        else if ( kFrameStepFwd.match(key) )
        {
            view()->step_frame( 1 );
            return 1;
        }
        else if ( kFrameStepBack.match(key) )
        {
            view()->step_frame( -1 );
            return 1;
        }
        else if ( kPreviousVersionImage.match(key) )
        {
            browser()->previous_image_version();
            return 1;
        }
        else if ( kNextVersionImage.match(key) )
        {
            browser()->next_image_version();
            return 1;
        }
        else if ( kPreviousImage.match(key) )
        {
            browser()->previous_image();
            return 1;
        }
        else if ( kNextImage.match(key) )
        {
            browser()->next_image();
            return 1;
        }
        else if ( key == 'f' || key == 'a' )
        {
            unsigned i = 0;
            unsigned e = children();

            int64_t tmin = std::numeric_limits<int64_t>::max();
            int64_t tmax = std::numeric_limits<int64_t>::min();

            Fl_Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
            Fl_Choice* c2 = main()->uiEDLWindow->uiEDLChoiceTwo;

            int one = c1->value();
            int two = c2->value();

            if ( one == -1 && two == -1 )
            {
                tmin = 1;
                tmax = 100;
            }

            for ( ; i != e; ++i )
            {

                if ( i != one && i != two ) continue;

                mrv::media_track* o = (mrv::media_track*)child(i);
                mrv::Reel r = browser()->reel_at( i );
                if (!r) continue;

                mrv::MediaList::iterator j = r->images.begin();
                mrv::MediaList::iterator k = r->images.end();

                mrv::media fg = view()->foreground();

                if ( key == 'f' )
                {
                    for ( ; j != k; ++j )
                    {
                        const mrv::media& m = *j;
                        if (m == fg)
                        {
                            int64_t tmi = m->position();
                            int64_t tma = m->position() + m->duration();
                            if ( tmi < tmin ) tmin = tmi;
                            if ( tma > tmax ) tmax = tma;
                            break;
                        }
                    }

                    if ( j == k )
                    {
                        tmin = 1;
                        tmax = 100;
                    }

                }
                else
                {

                    for ( ; j != k; ++j )
                    {
                        const mrv::media& m = *j;
                        int64_t tmi = m->position();
                        int64_t tma = m->position() + m->duration();
                        if ( tmi < tmin ) tmin = tmi;
                        if ( tma > tmax ) tmax = tma;
                    }
                }
            }

            mrv::Timeline* t = timeline();
            t->minimum( double(tmin) );
            t->maximum( double(tmax) );
            t->redraw();
            redraw();
            return 1;
        }
    }
    break;
    case FL_MOUSEWHEEL:
        if ( Fl::event_dy() < 0.f )
        {
            zoom( 0.5 );
        }
        else
        {
            zoom( 2.0 );
        }
        return 1;
        break;
    case FL_RELEASE:
        if ( Fl::event_button() == FL_LEFT_MOUSE )
        {
	    window()->cursor( FL_CURSOR_DEFAULT );
	    
            _dragX = Fl::event_x();
            _dragY = Fl::event_y();

	    Fl::remove_timeout( (Fl_Timeout_Handler) static_move, this );
	    int idx = int( ( _dragY - y() ) / kTrackHeight );
            if ( idx < 0 || idx >= 2 || idx >= children() ) {
                delete _drag;
                _drag = NULL;
                redraw();
                return 1;
            }

	    if ( _dragChild < 0 || !_drag ) return 1;
	    


            mrv::media_track* t1 = (mrv::media_track*)child( _dragChild );
            mrv::media_track* t2 = (mrv::media_track*)child( idx );


            if ( !t2 || t2->reel() == -1 ) {
                delete _drag;
                _drag = NULL;
                redraw();
                return 1;
            }

            mrv::Timeline* t = timeline();
            if (!t) return 0;


            int ww = t->w();
            double len = (t->maximum() - t->minimum() + 1);
            double p = double( _dragX - x() ) / double(ww);
            p = t->minimum() + p * len;
            int64_t pt = int64_t( p );


            mrv::Reel r = browser()->current_reel();

            mrv::media m = _drag->media();


            if ( t1 == t2 )
            {

                if ( pt < m->position() )
                {

                    t1->remove( m );
                    t1->insert( pt, m );
                    t1->refresh();
                }
                else
                {

                    t1->insert( pt, m );
                    t1->remove( m );
                    t1->refresh();
                }


                browser()->reel( r->name.c_str() );
                browser()->redraw();
            }
            else
            {

                t2->insert( pt, m );
                t1->remove( m );
                t2->refresh();


                browser()->reel( r->name.c_str() );
                browser()->redraw();
            }


            browser()->set_edl();
            timeline()->value( (double)pt );
            view()->seek( pt );
            delete _drag;
            _drag = NULL;
            _dragChild = -1;
            redraw();
            return 1;
        }
        break;
    case FL_DRAG:
    {
        int diff = ( Fl::event_x() - _dragX );

        if ( Fl::event_button() == FL_LEFT_MOUSE )
        {
            int X = Fl::event_x();
            _dragY = Fl::event_y();

            // LIMITS
            if ( _dragY < 33 ) _dragY = 33;
            if ( X < 8 ) X = 8;

	    int quarter = w() / 4;

            if ( X >= x() + w() - quarter ) {
                pan(diff * 2);
                Fl::add_timeout( 0.1f, (Fl_Timeout_Handler) static_move, this );
            }
            else if ( X <= x() + quarter )
            {
                pan(diff * -2);
                Fl::add_timeout( 0.1f, (Fl_Timeout_Handler) static_move, this );
            }

            _dragX = X;


            redraw();
            return 1;
        }
        else if ( Fl::event_button() == FL_MIDDLE_MOUSE )
        {
            pan(diff * 2);
            _dragX = Fl::event_x();
            return 1;
        }
	return 0;
    }
    break;
    default:
        break;
    }

    return Fl_Group::handle( event );
}

void EDLGroup::zoom( double z )
{

    mrv::Timeline* t = timeline();

    double pct = (double) Fl::event_x() / (double)w();

    int64_t tmin = int64_t( t->minimum() );
    int64_t tmax = int64_t( t->maximum() );


    int64_t tdiff = tmax - tmin;
    int64_t tcur = tmin + int64_t( pct * double(tdiff) );


    tmax = int64_t( double(tmax) * z );
    tmin = int64_t( double(tmin) * z );

    int64_t tlen = tmax - tmin;

    tmax = tcur + int64_t( ( 1.0 - pct ) *  double(tlen));
    tmin = tcur - int64_t( double(tlen) * pct );


    if ( tmin < 0 ) tmin = 0;

    t->minimum( double(tmin) );
    t->maximum( double(tmax) );
    t->redraw();


    unsigned e = children();
    for ( unsigned i = 0; i < e; ++i )
    {
        mrv::media_track* c = (mrv::media_track*)child(i);
        c->zoom( z );
    }

}

void EDLGroup::cut( boost::int64_t frame )
{
    Fl_Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
    int c = c1->value();
    if ( c < 0 ) return;

    const mrv::Reel& r = browser()->reel_at(c);
    if (!r) return;

    
    CMedia* img = r->image_at( frame );
    if ( !img ) return;

    int idx = (int) r->index( frame );
    int64_t f = r->global_to_local( frame );

    if ( img->first_frame() == f || img->last_frame() == f )
        return;


    CMedia* right = CMedia::guess_image( img->fileroot(), NULL, 0, false, f,
                                         img->last_frame() );
    if (!right) return;

    mrv::media m( new mrv::gui::media( right ) );
    
    right->first_frame( f );
    right->last_frame( img->last_frame() );
    image_type_ptr canvasR;
    right->fetch( canvasR, f );
    right->cache( canvasR );
    right->decode_video( f );
    right->find_image( f );

    const std::string& file = img->audio_file();
    if (! file.empty() )
        right->audio_file( file.c_str() );
    right->audio_offset( img->audio_offset() );
    right->seek( f );


    f -= 1;
    img->last_frame( f );
    image_type_ptr canvasL;
    img->fetch( canvasL, f );
    img->cache( canvasL );
    img->decode_video( f );
    img->find_image( f );

    m->create_thumbnail();

    r->edl = true;

    browser()->reel( c );
    browser()->insert( idx+1, m );
    browser()->value( idx+1 );

    refresh();

    uiMain->uiTimeline->edl(true);
    right->ocio_input_color_space( img->ocio_input_color_space() );
    right->rendering_transform( img->rendering_transform() );

    redraw();
}

void EDLGroup::merge( boost::int64_t frame )
{
    Fl_Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
    int c = c1->value();
    if ( c < 0 ) return;

    mrv::Reel r = browser()->reel_at(c);
    if (!r) return;

    if ( r->images.size() < 2 ) return;

    CMedia* img = r->image_at( frame );
    int idx = (int) r->index( frame );
    int64_t f = r->global_to_local( frame );

    int idx2 = idx;
    if ( f < img->first_frame() + (int64_t) img->duration() / 2 )
    {
        idx -= 1;
    }

    idx2 = idx + 1;

    if ( idx < 0 || idx2 >= (int)r->images.size() ) return;

    mrv::media lm( r->images[idx] );
    if (!lm) return;
    mrv::media rm( r->images[idx2] );
    if (!rm ) return;

    CMedia* left  = lm->image();
    CMedia* right = rm->image();

    if ( strcmp( left->fileroot(), right->fileroot() ) != 0 ) return;

    if ( left->last_frame() + 1 != right->first_frame() ) return;

    left->last_frame( right->last_frame() );

    browser()->remove( unsigned(idx2) );
    browser()->change_image( int(idx) );
    refresh();
    redraw();
}

void EDLGroup::refresh()
{
    unsigned e = children();
    for ( unsigned i = 0; i < e; ++i )
    {
        DBG( "REFRESH MEDIA TRACK " << i );
        mrv::media_track* o = (mrv::media_track*) child(i);
        o->refresh();
        o->redraw();
    }
}

void EDLGroup::draw()
{

    fl_color( fl_rgb_color( 128, 128, 128 ) );
    fl_rectf( x(), y(), w(), h() );

    Fl_Group::draw();

    mrv::Timeline* t = timeline();
    if (!t) return;

    double frame = t->value();

    int p = t->x() + t->slider_position( frame, t->w() );
    p += int( t->slider_size()/2.0 );


    fl_color( FL_YELLOW );
    fl_push_clip( x(), y(), w(), y()+h() );
    fl_line( p, y(), p, y()+h() );
    fl_pop_clip();

    if ( _drag )
    {
	fl_push_clip( x(), y(), w(), h() );
	_drag->DrawAt( Fl::event_x(), Fl::event_y() );
	fl_pop_clip();
    }
}

} // namespace mrv
