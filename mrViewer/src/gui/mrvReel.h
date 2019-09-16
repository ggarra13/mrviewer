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
/**
 * @file   mrvReel.h
 * @author
 * @date   Wed Oct 18 11:06:54 2006
 *
 * @brief  Class used to contain a "reel".  A reel is a sequence of
 *         movie files like an EDL
 *
 *
 */

#ifndef mrvReel_h
#define mrvReel_h

#include <iostream>
using namespace std;

#include "mrvMediaList.h"

namespace mrv
{

struct Reel_t
{
    Reel_t( const char* n ) : edl(false), name( n ) {}
    ~Reel_t() {}

    mrv::media media_at( const int64_t f ) const;
    inline CMedia* image_at( const int64_t f ) const
    {
	mrv::media m = media_at( f );
	if (!m) return NULL;
	return m->image();
    }

    size_t duration() const;

    size_t index( const CMedia* const img ) const;
    size_t index( const int64_t frame ) const;

    // Given a frame in the timeline, return the local frame for the image
    // at that point in the timeline.  If no image, return AV_NOPTS_VALUE.
    int64_t global_to_local( const int64_t frame ) const;

    int64_t offset( const CMedia* const img ) const;
    inline int64_t location( const CMedia* const img ) const
    {
	return offset( img ) + 1;
    }

    int64_t minimum() const;
    int64_t maximum() const;

    std::atomic<bool> edl;
    std::string       name;
    MediaList         images;
};

typedef boost::shared_ptr< Reel_t > Reel;


} // namespace mrv


#endif // mrvReel_h
