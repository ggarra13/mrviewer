/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

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

#include <inttypes.h>  // for PRId64

#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>

#include "core/mrvI8N.h"
#include "gui/mrvMediaTrack.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvReelList.h"
#include "gui/mrvElement.h"
#include "gui/mrvImageInformation.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvIO.h"
#include "mrViewer.h"
#include "mrvEDLWindowUI.h"

namespace {
const char* kModule = "track";
}

void open_cb( fltk::Widget* o, mrv::ImageBrowser* uiReelWindow );

void open_track_cb( fltk::Widget* o, mrv::media_track* track )
{
    track->browser()->reel( track->reel() );
    open_cb( o, track->browser() );
}

namespace mrv {

mrv::Element* media_track::_selected = NULL;
bool media_track::_audio_selected = false;

media_track::media_track(int x, int y, int w, int h) :
    fltk::Group( x, y, w, h ),
    _zoom( 1.0 )
{
}

media_track::~media_track()
{
}


double media_track::frame_size() const
{
    mrv::Timeline* t = main()->uiEDLWindow->uiTimeline;

    double x = t->w() / double(t->maximum() - t->minimum() + 1);
    return x;
}

// Add a media at a certain frame (or append to end by default)
void media_track::add( mrv::media m, boost::int64_t frame )
{
    if ( _reel_idx < 0 ) return;
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel || !m ) return;

    size_t e = reel->images.size();
    if ( frame == MRV_NOPTS_VALUE )
    {
        if ( e < 2 )
        {
            frame = 1;
        }
        else
        {
            mrv::media o = reel->images[e-2];
            frame = o->position();
            frame += o->duration();
        }
    }
    m->position( frame );

    if ( e >= 1 )
    {
        mrv::media o = reel->images[ e-1 ];
        timeline()->maximum( double( frame + o->duration() - 1 ) );
    }

    timeline()->redraw();
    parent()->redraw();
}

mrv::ImageBrowser* media_track::browser() const {
    return _main->uiReelWindow->uiBrowser;
}

int media_track::index_for( const mrv::media& m )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) {
        return -1;
    }

    unsigned e = (unsigned)reel->images.size();

    for (unsigned i = 0; i < e; ++i )
    {
        const mrv::media& m2 = reel->images[i];
        if ( m == m2 )
        {
            return i;
        }
    }

    return -1;
}

mrv::media media_track::media( const unsigned idx )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return mrv::media();

    size_t e = reel->images.size();
    if ( idx >= e ) return mrv::media();
    return reel->images[idx];
}


int media_track::index_for( const std::string s )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return -1;

    unsigned e = (unsigned) reel->images.size();

    for (unsigned i = 0; i < e; ++i )
    {
        mrv::media m = reel->images[i];
        if ( m && m->image()->fileroot() == s )
        {
            return i;
        }
    }

    return -1;
}

int media_track::index_at( const boost::int64_t frame )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return -1;

    return reel->index( frame );
}

mrv::media media_track::media_at( const boost::int64_t frame )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return mrv::media();

    int idx = index_at( frame );
    if ( idx >= 0 && idx < reel->images.size() )
    {
        return reel->images[idx];
    }

    return mrv::media();

}


// Insert a media in the track at a specific frame which will turn into an image
// index
void media_track::insert( const boost::int64_t frame, mrv::media m )
{
    int idx = index_at( frame );
    if ( idx < 0 ) idx = 0;
    if ( _reel_idx < 0 ) return;

    browser()->reel( _reel_idx );
    browser()->insert( idx, m );
    browser()->parent()->redraw();
    parent()->redraw();
    return;
}

// Remove a media from the track
bool media_track::remove( const int idx )
{
    if ( _reel_idx < 0 ) return false;
    browser()->reel( _reel_idx );
    browser()->remove( idx );
    browser()->parent()->redraw();
    delete _selected;
    _selected = NULL;
    redraw();
    return true;
}

// Remove a media from the track
bool media_track::remove( const mrv::media m )
{
    if ( _reel_idx < 0 ) return false;
    browser()->reel( _reel_idx );
    browser()->remove( m );
    browser()->parent()->redraw();
    delete _selected;
    _selected = NULL;
    redraw();
    return true;
}

// Move a media in track without changing its start or end frames
// Shift surrounding media to remain attached.
void media_track::shift_media( mrv::media m, boost::int64_t frame )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return;

    size_t e = reel->images.size();
    size_t idx = 0;

    for (size_t i = 0; i < e; ++i )
    {
        if ( m == reel->images[i] )
        {
            idx = i;
            reel->images[i]->position( frame );
            break;
        }
    }

    // Shift medias that come after
    for (size_t i = idx+1; i < e; ++i )
    {
        mrv::media& fg = reel->images[i-1];
        boost::int64_t end = fg->position() + fg->duration() - 1;
        reel->images[i]->position( end );
    }


    if ( idx == 0 ) {
        return;
    }

    // Shift medias that come before
    for (int i = int(idx)-1; i >= 0; --i )
    {
        boost::int64_t start = reel->images[i+1]->position();
        mrv::media& o = reel->images[i];
        boost::int64_t ee = o->position() + o->duration() - 1;
        boost::int64_t ss = o->position();

        // Shift indexes of position
        reel->images[i]->position( start - (ee - ss ) );
    }

    return;
}

void media_track::shift_audio( mrv::media m, boost::int64_t diff )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return;


    unsigned idx = 0;
    unsigned e = (unsigned)reel->images.size();
    for ( unsigned i = 0; i < e; ++i )
    {
        mrv::media fg = reel->images[i];
        if ( fg == m )
        {
            idx = i;
            CMedia* img = m->image();
            if ( !img ) continue;

            int64_t newoff = img->audio_offset() - diff;
            img->audio_offset( newoff );
            main()->uiView->foreground( fg );
            img->seek( img->frame() );

            char buf[1024];
            sprintf( buf, N_("ShiftAudio %d")
                     N_(" \"%s\" %") PRId64,
                     _reel_idx, img->fileroot(), img->audio_offset() );
            main()->uiView->send_network( buf );
            break;
        }
    }

}


void media_track::shift_media_start( mrv::media m, boost::int64_t diff )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return;


    unsigned idx = 0;
    unsigned e = (unsigned)reel->images.size();
    for ( unsigned i = 0; i < e; ++i )
    {
        mrv::media fg = reel->images[i];
        if ( fg == m )
        {
            idx = i;
            int64_t newpos = m->position() + diff;
            if ( newpos < (int64_t) ( m->position() + m->duration() ) )
            {
                CMedia* img = m->image();
                img->first_frame( img->first_frame() + diff );
                int64_t f = img->first_frame();
                if ( ! reel->edl )
                {
                    main()->uiStartFrame->value( f );
                }
                img->seek(f);
                main()->uiTimeline->value( newpos );
                main()->uiView->foreground( fg );
                main()->uiView->redraw();

                char buf[1024];
                sprintf( buf, N_("ShiftMediaStart %d")
                         N_(" \"%s\" %") PRId64,
                         _reel_idx, img->fileroot(), img->first_frame() );
                main()->uiView->send_network( buf );
            }
            break;
        }
    }

    // Shift medias that come after
    for (size_t i = idx+1; i < e; ++i )
    {
        mrv::media& o = reel->images[i-1];
        boost::int64_t end = o->position() + o->duration();
        mrv::media& fg = reel->images[i];
        fg->position( end );
    }


    if ( idx == 0 ) {
        return;
    }

    // Shift medias that come before
    for (int i = int(idx)-1; i >= 0; --i )
    {
        boost::int64_t start = reel->images[i+1]->position();
        mrv::media& o = reel->images[i];
        boost::int64_t ee = o->position() + o->duration() - 1;
        boost::int64_t ss = o->position();

        // Shift indexes of position
        o->position( start - (ee - ss ) );
    }


}

bool media_track::select_media( const boost::int64_t pos,
                                const int Y )
{
    bool ok = false;
    delete _selected;
    _selected = NULL;

    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return false;

    unsigned e = (unsigned) reel->images.size();

    for ( unsigned i = 0; i < e; ++i )
    {
        mrv::media fg = reel->images[i];
        if ( !fg ) continue;

        CMedia* img = fg->image();

        int64_t start = fg->position();
        int64_t duration = (int64_t)img->duration();

        if ( pos >= start && pos < start + duration)
        {
            if ( pos < start + duration / 2 )
            {
                _at_start = true;
            }
            else
            {
                _at_start = false;
            }

            ok = true;
            _audio_selected = false;
            if ( Y > h()-20 && Y < h() ) _audio_selected = true;
            _selected = ImageBrowser::new_item( fg );
            focus(this);
            if ( _reel_idx < 0 ) break;


            browser()->reel( _reel_idx );
            browser()->change_image( i );
            browser()->redraw();
            if ( reel->edl )
            {
                main()->uiTimeline->value( double(fg->position()) );
            }
            break;
        }
    }
    timeline()->redraw();
    parent()->redraw();
    return ok;
}

mrv::Timeline* media_track::timeline() const
{
    return main()->uiEDLWindow->uiTimeline;
}

void media_track::shift_media_end( mrv::media m, boost::int64_t diff )
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return;


    size_t e = reel->images.size();
    size_t i = 0;
    for ( ; i < e; ++i )
    {
        mrv::media& fg = reel->images[i];
        if ( fg == m )
        {
            int64_t pos = m->image()->last_frame() + diff;
            if ( pos > m->image()->first_frame() &&
                    pos <= m->image()->end_frame() )
            {
                CMedia* img = m->image();
                if ( reel->edl )
                {
                    main()->uiTimeline->value( m->position() + pos );
                }

                img->last_frame( pos );
                img->seek( pos );

                char buf[1024];
                sprintf( buf, N_( "ShiftMediaEnd %d" )
                         N_(" \"%s\" %" ) PRId64,
                         _reel_idx, img->fileroot(), img->last_frame() );
                main()->uiView->send_network( buf );
                main()->uiView->redraw();

                main()->uiImageInfo->uiInfoText->refresh();
                break;
            }
        }
    }

    // Shift medias that come after
    for ( ; i < e-1; ++i )
    {
        boost::int64_t start = reel->images[i]->position();
        boost::int64_t ee = reel->images[i]->duration();

        // Shift indexes of position
        reel->images[i+1]->position(start + ee );
    }
    timeline()->redraw();
    redraw();

}

void media_track::refresh()
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) {
        DBG( "EMPTY REEL AT INDEX " << _reel_idx );
        return;
    }

    DBG( reel->name << " #images=" << reel->images.size() );

    //
    // Adjust timeline position
    //
    MediaList::iterator i = reel->images.begin();
    MediaList::iterator j;
    MediaList::iterator end = reel->images.end();

    if ( i != end )
    {
        (*i)->position( 1 );

        for ( j = i, ++i; i != end; j = i, ++i )
        {
            int64_t frame = (*j)->position() + (*j)->duration();
            DBG( (*i)->image()->name() << " set to frame " << frame );
            (*i)->position( frame );
        }
    }

    timeline()->redraw();
    redraw();
}

void  media_track::zoom( double x )
{
    _zoom *= x;
    timeline()->redraw();
    redraw();
}

int media_track::handle( int event )
{
    switch( event )
    {
    case fltk::RELEASE:
        if ( _selected && fltk::event_key() == fltk::RightButton )
        {
            mrv::Timeline* t = main()->uiTimeline;
            if ( ! t->edl() )
            {
                mrv::media fg = _selected->element();
                int64_t start = fg->image()->first_frame();
                int64_t end   = fg->image()->last_frame();
                t->minimum( double(start) );
                // main()->uiStartFrame->value( start );
                t->maximum( double(end) );
                // main()->uiEndFrame->value( end );
            }

            main()->uiView->seek( _frame );
            if ( _playback != CMedia::kStopped )
                main()->uiView->play( _playback );
            main()->uiImageInfo->uiInfoText->refresh();
        }
        return 1;
        break;
    case fltk::PUSH:
    {
        int xx = _dragX = fltk::event_x();
        int yy = y() + fltk::event_y();

        if ( fltk::event_key() == fltk::RightButton )
        {
            cursor( fltk::CURSOR_ARROW );
            _playback = (CMedia::Playback) main()->uiView->playback();
            main()->uiView->stop();
            _frame = main()->uiView->frame();

            mrv::Timeline* t = timeline();

            int ww = t->w();

            double len = (t->maximum() - t->minimum() + 1);
            double p = double(xx+t->x()) / double(ww);
            p = t->minimum() + p * len + 0.5f;

            if ( select_media( int64_t(p), yy ) )
                return 1;
            else
            {
                if ( _reel_idx < 0 ) return 1;
                fltk::Menu menu(0,0,0,0);
                menu.add( _("File/Open/Movie or Sequence"),
                          kOpenImage.hotkey(),
                          (fltk::Callback*)open_track_cb, this );
                menu.popup( fltk::Rectangle( fltk::event_x(),
                                             fltk::event_y(), 80, 1) );
            }
        }
        else
        {
            return 0;
        }
    }
    case fltk::KEY:
    {
        int key = fltk::event_key();
        if ( key == fltk::DeleteKey ||
                key == fltk::BackSpaceKey )
        {
            if ( _selected )
                remove( _selected->element() );
            return 1;
        }
        break;
    }
    case fltk::ENTER:
        return 1;
    case fltk::DRAG:
    {
        if ( _selected )
        {
            cursor( fltk::CURSOR_WE );

            const mrv::Reel& reel = browser()->reel_at( _reel_idx );
            if ( !reel ) return 0;

            int diff = (fltk::event_x() - _dragX);

            mrv::MediaList::const_iterator i = reel->images.begin();
            mrv::MediaList::const_iterator e = reel->images.end();

            for ( ; i != e; ++i )
            {
                if ( *i == _selected->element() )
                {
                    if ( _audio_selected )
                    {
                        shift_audio( _selected->element(), diff );
                    }
                    else
                    {
                        if ( _at_start )
                            shift_media_start( _selected->element(), diff );
                        else
                            shift_media_end( _selected->element(), diff );
                        _selected->element()->create_thumbnail();
                    }


                    break;
                }
            }
            timeline()->redraw();
            redraw();
        }
        _dragX = fltk::event_x();
        return 1;
    }
    default:
        break;
    }

    return fltk::Group::handle( event );
}

int64_t media_track::minimum() const
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return MRV_NOPTS_VALUE;
    if ( reel->images.size() == 0 ) return MRV_NOPTS_VALUE;
    return reel->images[0]->position();
}

int64_t media_track::maximum() const
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return MRV_NOPTS_VALUE;

    size_t e = reel->images.size();
    if ( e == 0 ) return MRV_NOPTS_VALUE;

    e = e - 1;
    return reel->images[e]->position() + reel->images[e]->duration();
}

void media_track::draw()
{
    const mrv::Reel& reel = browser()->reel_at( _reel_idx );
    if ( !reel ) return;

    size_t e = reel->images.size();

    fltk::load_identity();
    fltk::setcolor( fltk::GRAY33 );

    fltk::push_clip( x(), y(), w(), h() );
    fltk::fillrect( x(), y(), w(), h() );

    fltk::load_identity();

    // {
    //    fltk::setcolor( fltk::WHITE );
    //    int ww, hh;
    //    const char* buf = reel->name.c_str();
    //    fltk::setfont( textfont(), 12 );
    //    fltk::measure( buf, ww, hh );
    //    fltk::drawtext( buf, 4.0f, float( y()+h()/2 ) );
    // }

    mrv::Timeline* t = timeline();
    assert( t != NULL );


    int ww = t->w();
    int rx = t->x() + (t->slider_size()-1)/2;

    for ( size_t i = 0; i < e; ++i )
    {
        mrv::media fg = reel->images[i];
        if (!fg) continue;

        int64_t pos = fg->position();
        const CMedia* img = fg->image();
        assert( img != NULL );

        int dx = t->slider_position( double( pos ), ww );
        int dw = t->slider_position( double( pos + fg->duration() ), //-1 needed
                                     ww );
        dw -= dx;


        fltk::Rectangle r(rx+dx, y(), dw, h()-20 );


        if ( browser()->current_image() == fg )
        {
            fltk::setcolor( fltk::DARK_YELLOW );
        }
        else
        {
            fltk::setcolor( fltk::DARK_GREEN );
        }

        fltk::fillrect( r );

        int stream = img->audio_stream();
        if ( stream >= 0 )
        {
            const CMedia::audio_info_t& info = img->audio_info( stream );

            double fps = img->fps();
            boost::int64_t first = boost::int64_t( info.start * fps - 0.5 );

            int64_t offset = img->audio_offset();
            boost::int64_t vlen = img->duration() - 1;
            boost::int64_t length = boost::int64_t( info.duration * fps + 0.5 );
            if ( length - offset > vlen ) length = vlen + offset;
            boost::int64_t last = first + length;

            if ( offset >= 0 )
            {
                first += pos;
            }
            else
            {
                first += pos - offset;
            }

            // std::cerr << img->name() << "  first: " << first
            //           << " last: " << pos+last << " length " << length
            //           << " vlen: " << vlen
            //           << " pos: " << pos << " offset " << offset << std::endl;

            int dx = t->slider_position( double( first ), ww );
            int dw = t->slider_position( double( pos+last-offset ), ww );
            dw -= dx;

            fltk::Rectangle ra(rx+dx, h()+y()-20, dw, 20 );
            fltk::fillrect( ra );

            if ( _selected && _selected->element() == fg )
                fltk::setcolor( fltk::WHITE );
            else
                fltk::setcolor( fltk::BLACK );
            fltk::strokerect( ra );


            if ( _selected && _selected->element() == fg )
                fltk::setcolor( fltk::BLACK );
            else
                fltk::setcolor( fltk::GRAY33 );

            int aw = 0, ah = 0;
            char buf[128];
            sprintf( buf, _("Audio: %" PRId64 ), offset );
            fltk::setfont( textfont(), textsize() );
            fltk::measure( buf, aw, ah );

            fltk::Rectangle off( rx + dx + dw/2 - aw/2,
                                 y() + h()-ah/2, aw, ah );
            ra.intersect( off );
            if ( !ra.empty() )
            {
                fltk::drawtext( buf, float( off.x()+2 ), float( off.y()+2 ) );

                if ( _selected && _selected->element() == fg )
                    fltk::setcolor( fltk::WHITE );
                else
                    fltk::setcolor( fltk::BLACK );

                fltk::drawtext( buf, float( off.x() ), float( off.y() ) );
            }
        }


        fltk::Image* thumb = fg->thumbnail();

        if ( thumb && dw > thumb->w() )
        {
            thumb->draw( r.x()+2, y()+2 );
        }

        if ( _selected && _selected->element() == fg )
            fltk::setcolor( fltk::WHITE );
        else
            fltk::setcolor( fltk::BLACK );
        fltk::strokerect( r );

        if ( _selected && _selected->element() == fg )
        {
            char buf[128];
            fltk::setcolor( fltk::BLUE );
            int Y = y();

            if ( _audio_selected ) Y += h() - 20;

            int yh = y() + h();
            if ( _at_start )
            {
                fltk::newpath();
                fltk::addvertex( r.x(), Y );
                fltk::addvertex( r.x(), yh );
                fltk::addvertex( r.x() + dw/2, yh );
                fltk::closepath();
                fltk::strokepath();
                if ( ! _audio_selected )
                {
                    sprintf( buf, "%" PRId64, img->first_frame() );
                    fltk::drawtext( buf, r.x() + dw/4, yh-5 );
                }
            }
            else
            {
                fltk::newpath();
                fltk::addvertex( r.x()+dw, Y );
                fltk::addvertex( r.x()+dw, yh );
                fltk::addvertex( r.x()+dw/2, yh );
                fltk::closepath();
                fltk::strokepath();
                if ( ! _audio_selected )
                {
                    sprintf( buf, "%" PRId64, img->last_frame() );
                    fltk::drawtext( buf, r.x() + dw/2 + dw/4, yh-5 );
                }
            }
        }


        if ( _selected && _selected->element() == fg )
            fltk::setcolor( fltk::BLACK );
        else
            fltk::setcolor( fltk::GRAY33 );

        int ww = 0, hh = 0;
        std::string name = img->name();
        const char* txt = name.c_str();
        fltk::setfont( textfont(), textsize() );
        fltk::measure( txt, ww, hh );

        fltk::Rectangle text( rx + dx + dw/2 - ww/2,
                              y() + (h()-20)/2, ww, hh );
        r.intersect( text );
        if ( r.empty() ) continue;



        fltk::drawtext( txt,
                        float( text.x() + 2 ),
                        float( text.y() + 2 ) );

        if ( _selected && _selected->element() == fg )
            fltk::setcolor( fltk::WHITE );
        else
            fltk::setcolor( fltk::BLACK );

        fltk::drawtext( txt,
                        float( text.x() ),
                        float( text.y() ) );
    }

    fltk::pop_clip();
}

}  // namespace mrv
