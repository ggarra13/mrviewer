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
    return *a < b;
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
     return ( b->frame() + b->repeat() < _start || b->frame() - b->repeat() > _end );
  }
};

} // namespace mrv


#endif  // mrvFrameFunctors_h
