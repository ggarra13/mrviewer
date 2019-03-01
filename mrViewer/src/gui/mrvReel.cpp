/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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

#include "gui/mrvIO.h"
#include "mrvReel.h"

namespace {
const char* kModule = "reel_t";
}


namespace mrv {

int64_t Reel_t::minimum() const
{
    if ( images.size() == 0 ) return 0;
    mrv::media m = images.front();
    return m->position();
}

int64_t Reel_t::maximum() const
{
    if ( images.size() == 0 ) return 0;
    mrv::media m = images.back();
    return m->position() + m->duration() - 1;
}

size_t Reel_t::duration() const
{
    return maximum() - minimum() + 1;
}

size_t Reel_t::index( const CMedia* const img ) const
{
    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    if ( i == e ) return std::numeric_limits<size_t>::max();

    size_t r = 0;

    if ( img->is_stereo() && ! img->is_left_eye() )
    {
        for ( ; i != e; ++i, ++r )
        {
            if ( (*i)->image()->right_eye() == img )
            {
                return r;
            }
        }
    }
    else
    {
        for ( ; i != e; ++i, ++r )
        {
            if ( (*i)->image() == img )
            {
                return r;
            }
        }
    }
    return 0;
}

/**
 * Given a frame, return its image index in reel when in edl mode
 *
 * @param f frame to search in edl list
 *
 * @return index of image in reel list or std::
 */
size_t Reel_t::index( const int64_t f ) const
{
    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    if ( i == e ) return std::numeric_limits<size_t>::max();

    mrv::media fg = images.front();
    int64_t mn = fg->position();


    fg = images.back();
    int64_t mx = fg->position() + fg->duration() - 1;

    if ( f < mn || f > mx ) return std::numeric_limits<size_t>::max();

    int64_t  t = 1;
    size_t r = 0;
    for ( ; i != e; ++i, ++r )
    {
        const mrv::media& m = *i;
        CMedia* img = m->image();
        int64_t start = m->position();
        int64_t end = start + img->duration();
        if ( f >= start && f < end )
            break;
    }

    if ( r >= images.size() ) r = std::numeric_limits<size_t>::max();

    return r;

}


mrv::media Reel_t::media_at( const int64_t f ) const
{
    if ( images.empty() ) return mrv::media();

    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    mrv::media fg = images.front();
    if ( !fg ) return mrv::media();

    int64_t mn = fg->position();

    fg = images.back();
    int64_t mx = fg->position() + fg->duration();

    if ( f < mn || f >= mx ) return mrv::media();

    int64_t  t = 1;
    size_t r = 0;
    for ( ; i != e; ++i, ++r )
    {
        const mrv::media& m = *i;
        if ( !m ) continue;

        CMedia* img = m->image();
        int64_t start = m->position();
        int64_t end = start + img->duration();
        if ( f >= start && f < end ) break;
    }

    if ( r >= images.size() ) return mrv::media();

    return images[r];
}

int64_t Reel_t::global_to_local( const int64_t f ) const
{

    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    int64_t r = AV_NOPTS_VALUE;
    for ( ; i != e; ++i )
    {
        const mrv::media& m = *i;
        if ( !m ) continue;

        CMedia* img = m->image();
        assert( img != NULL );
        int64_t start = m->position();
        int64_t end   = start + m->duration();
        if ( f >= start && f < end )
        {
            r = f - start + img->first_frame();
            break;
        }
    }

    return r;
}

int64_t Reel_t::offset( const CMedia* const img ) const
{

    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();
    uint64_t t = 0;

    if ( img->is_stereo() && ! img->is_left_eye() )
    {
        for ( ; i != e && (*i)->image()->right_eye() != img; ++i )
        {
            CMedia* timg = (*i)->image()->right_eye();
            assert( timg != NULL );

            t += timg->duration();
        }
        return t;
    }

    for ( ; i != e && (*i)->image() != img; ++i )
    {
        CMedia* timg = (*i)->image();
        assert( timg != NULL );

        t += timg->duration();
    }
    if ( i == e ) LOG_ERROR("Invalid image " << img->name() << " for reel "
                                << name );
    return t;
}


}  // namespace mrv

