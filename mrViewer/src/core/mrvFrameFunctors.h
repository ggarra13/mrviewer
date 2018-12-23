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
/**
 * @file   mrvFrameFunctors.h
 * @author gga
 * @date   Wed Jul 18 08:29:19 2007
 *
 * @brief  Functors used to compare with video/audio stores
 *
 *
 */

#ifndef mrvFrameFunctors_h
#define mrvFrameFunctors_h


#ifndef mrvFrame_h
#  include "mrvFrame.h"
#endif


namespace mrv {


struct ClosestToFunctor
{
    const int64_t frame;

    ClosestToFunctor( const int64_t f ) : frame(f) {}

    bool operator()( const audio_type_ptr& a, const audio_type_ptr& b ) const
    {
        if ( !a || !b ) return false;
        return ( frame > a->frame() && frame < b->frame() );
    }

    bool operator()( const image_type_ptr& a, const image_type_ptr& b ) const
    {
        if ( !a || !b ) return false;
        return ( frame > a->frame() && frame < b->frame() );
    }

};

struct EqualFunctor
{
    const int64_t _frame;
    EqualFunctor( const int64_t frame ) : _frame( frame ) {}

    bool operator()( const audio_type_ptr& a ) const
    {
        if ( !a ) return false;
        return a->frame() == _frame;
    }

    bool operator()( const image_type_ptr& a ) const
    {
        if ( !a ) return false;
        return a->frame() == _frame;
    }
};

struct MoreThanFunctor
{
    bool operator()( const int64_t a, const audio_type_ptr& b ) const
    {
        if ( !b ) return false;
        return a > b->frame();
    }
    bool operator()( const audio_type_ptr& a, const int64_t b ) const
    {
        if ( !a ) return false;
        return a->frame() > b;
    }

    bool operator()( const int64_t a, const image_type_ptr& b ) const
    {
        if ( !b ) return false;
        return a > b->frame();
    }
    bool operator()( const image_type_ptr& a, const int64_t b ) const
    {
        if ( !a ) return false;
        return a->frame() > b;
    }
};

struct LessThanFunctor
{
    bool operator()( const int64_t a, const audio_type_ptr& b ) const
    {
        if ( !b ) return false;
        return a < b->frame();
    }
    bool operator()( const audio_type_ptr& a, const int64_t b ) const
    {
        if ( !a ) return false;
        return a->frame() < b;
    }

    bool operator()( const int64_t a, const image_type_ptr& b ) const
    {
        if ( !b ) return false;
        return a < b->frame();
    }
    bool operator()( const image_type_ptr& a, const int64_t b ) const
    {
        if ( !a ) return false;
        return a->frame() < b;
    }
};


struct LessPTSThanFunctor
{
    bool operator()( const int64_t a, const image_type_ptr& b ) const
    {
        if ( !b ) return false;
        return a < b->pts();
    }
    bool operator()( const image_type_ptr& a, const int64_t b ) const
    {
        if ( !a ) return false;
        return a->pts() < b;
    }
};

struct TooOldFunctor
{
    timeval _max_time;

#undef timercmp
# define timercmp(a, b, CMP)					\
    (((a).tv_sec == (b).tv_sec) ?				\
     ((a).tv_usec CMP (b).tv_usec) :				\
     ((a).tv_sec CMP (b).tv_sec))

    struct customMore
    {
        inline bool operator()( const timeval& a,
                                const timeval& b ) const
        {
            return timercmp( a, b, > );
        }
    };


    TooOldFunctor( timeval max_time ) :
        _max_time( max_time )
    {
    }

    bool operator()( const image_type_ptr& b ) const
    {
        if ( !b ) return false;
        return customMore()( b->ptime(), _max_time );
    }
};

struct NotInRangeFunctor
{
    const int64_t _start;
    const int64_t _end;

    NotInRangeFunctor( const int64_t start, const int64_t end ) :
        _start( start ), _end( end )
    {
        assert( end >= start );
    }

    bool operator()( const audio_type_ptr& b ) const
    {
        return ( b->frame() < _start || b->frame() > _end );
    }

    bool operator()( const image_type_ptr& b ) const
    {
        if ( !b ) return false;
        return ( b->frame() + b->repeat() < _start ||
                 b->frame() - b->repeat() > _end );
    }

};

} // namespace mrv


#endif  // mrvFrameFunctors_h
