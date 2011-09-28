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
    Reel_t( const char* n ) : name( n ) {}
    ~Reel_t() {}

    std::string    name;
    MediaList    images;
  };

  typedef boost::shared_ptr< Reel_t > Reel;


} // namespace mrv


#endif // mrvReel_h

