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

#include "mrvReel.h"

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
   return m->position() + m->image()->duration() - 1;
}

size_t Reel_t::index( const CMedia* const img ) const
{
    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

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
   * @return index of image in reel list
   */
size_t Reel_t::index( const boost::int64_t f ) const
{
    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    int64_t mn = minimum();
    int64_t mx = maximum();

    if ( f < boost::int64_t(mn) ) return 0;
    if ( f > boost::int64_t(mx) ) return unsigned(e - i -1);

    int64_t  t = 1;
    size_t r = 0;
    for ( ; i != e; ++i, ++r )
      {
	CMedia* img = (*i)->image();
	uint64_t size = img->duration();
	t += size;
	if ( t > f ) break;
      }
    if ( r >= images.size() ) r = images.size() - 1;
    return r;

  }


mrv::media Reel_t::media_at( const boost::int64_t f ) const
{
    if ( images.size() == 0 ) return mrv::media();

    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    mrv::media m = images.front();
    double mn = m->position();

    m = images.back();
    double mx = m->position() + m->image()->duration() - 1;
    if ( mn > mx ) 
    {
       double t = mx;
       mx = mn; mn = t;
    }

    if ( f < mn ) return mrv::media();
    if ( f > mx ) return mrv::media();

    int64_t  t = 1;
    unsigned r = 0;
    for ( ; i != e; ++i, ++r )
      {
	CMedia* img = (*i)->image();
	t += img->duration();
 	if ( t > f ) break;
      }
    if ( r >= images.size() ) return mrv::media();

    return images[r];
}

boost::int64_t Reel_t::global_to_local( const boost::int64_t f ) const
{

    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    int64_t r = 0;
    uint64_t t = 0;
    for ( ; i != e; ++i )
      {
	CMedia* img = (*i)->image();
	assert( img != NULL );

	uint64_t size = img->duration();
	if ( boost::uint64_t(f) > t && t+size < boost::uint64_t(f) )
	   t += size;
	else if ( boost::uint64_t(f) >= t )
	{
	   r = f - int64_t(t) + img->first_frame() - 1;
	   return r;
	}
      }
    return r;
}

boost::int64_t Reel_t::offset( const CMedia* const img ) const
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
    return t;
  }


}  // namespace mrv

