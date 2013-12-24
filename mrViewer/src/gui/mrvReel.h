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
       Reel_t( const char* n ) : name( n ), edl(false) {}
       ~Reel_t() {}

       mrv::media media_at( const boost::int64_t f ) const;
       inline CMedia* image_at( const boost::int64_t f ) const
       {
	  mrv::media m = media_at( f );
	  if (!m) return NULL;
	  return m->image();
       }

       size_t index( const CMedia* const img ) const;
       size_t index( const int64_t frame ) const;
       boost::int64_t global_to_local( const boost::int64_t f ) const;
       boost::int64_t offset( const CMedia* const img ) const;
       inline boost::int64_t location( const CMedia* const img ) const
       {
	  return offset( img ) + 1;
       }

       int64_t minimum() const;
       int64_t maximum() const;

       bool         edl;
       std::string  name;
       MediaList    images;
  };

  typedef boost::shared_ptr< Reel_t > Reel;


} // namespace mrv


#endif // mrvReel_h

